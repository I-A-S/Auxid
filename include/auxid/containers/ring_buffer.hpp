// Auxid: The Orthodox C++ Platform.
// Copyright (C) 2026 IAS (ias@iasoft.dev)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <auxid/memory/box.hpp>
#include <auxid/containers/ring_buffer_view.hpp>

#if defined(_WIN32)
#  include <windows.h>
#else
#  include <sys/mman.h>
#  include <unistd.h>
#  include <fcntl.h>
#endif

namespace au::containers
{
  struct MirroredMemory
  {
    u8 *data_ptr{nullptr};
    u32 aligned_capacity{0};
  };

  class MirroredAllocator
  {
public:
    static auto allocate(u32 requested_capacity) -> Result<MirroredMemory>
    {
      MirroredMemory result;

#if defined(_WIN32)
      SYSTEM_INFO sys_info;
      GetSystemInfo(&sys_info);
      const u32 align = sys_info.dwAllocationGranularity;
      u32 pow2_capacity = 1;
      while (pow2_capacity < requested_capacity)
        pow2_capacity *= 2;
      result.aligned_capacity = pow2_capacity < align ? align : pow2_capacity;

      HANDLE map_handle =
          CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, result.aligned_capacity, NULL);
      if (!map_handle)
        return fail("Failed to create file mapping");

      while (true)
      {
        void *address = VirtualAlloc(NULL, result.aligned_capacity * 2, MEM_RESERVE, PAGE_NOACCESS);
        if (!address)
        {
          CloseHandle(map_handle);
          return fail("Failed to reserve virtual address space");
        }

        VirtualFree(address, 0, MEM_RELEASE);

        void *view1 = MapViewOfFileEx(map_handle, FILE_MAP_ALL_ACCESS, 0, 0, result.aligned_capacity, address);
        void *view2 = MapViewOfFileEx(map_handle, FILE_MAP_ALL_ACCESS, 0, 0, result.aligned_capacity,
                                      static_cast<u8 *>(address) + result.aligned_capacity);

        if (view1 && view2)
        {
          result.data_ptr = static_cast<u8 *>(view1);
          break;
        }

        if (view1)
          UnmapViewOfFile(view1);
        if (view2)
          UnmapViewOfFile(view2);
      }

      CloseHandle(map_handle);

#else
      const long page_size = sysconf(_SC_PAGESIZE);
      u32 pow2_capacity = 1;
      while (pow2_capacity < requested_capacity)
        pow2_capacity *= 2;
      result.aligned_capacity = pow2_capacity < page_size ? page_size : pow2_capacity;

      int fd = memfd_create("mirrored_ring_buffer", 0);
      if (fd == -1)
        return fail("Failed to create memfd");

      if (ftruncate(fd, result.aligned_capacity) == -1)
      {
        close(fd);
        return fail("Failed to set memfd size");
      }

      void *address = mmap(NULL, result.aligned_capacity * 2, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (address == MAP_FAILED)
      {
        close(fd);
        return fail("Failed to reserve virtual address space");
      }

      void *view1 = mmap(address, result.aligned_capacity, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0);
      void *view2 = mmap(static_cast<u8 *>(address) + result.aligned_capacity, result.aligned_capacity,
                         PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0);

      if (view1 == MAP_FAILED || view2 == MAP_FAILED)
      {
        close(fd);
        return fail("Failed to map physical memory to virtual addresses");
      }

      result.data_ptr = static_cast<u8 *>(view1);

      close(fd);
#endif

      return result;
    }

    static auto deallocate(MirroredMemory memory) -> void
    {
      if (!memory.data_ptr)
        return;

#if defined(_WIN32)
      UnmapViewOfFile(memory.data_ptr);
      UnmapViewOfFile(memory.data_ptr + memory.aligned_capacity);
#else
      munmap(memory.data_ptr, memory.aligned_capacity * 2);
#endif
    }
  };
} // namespace au::containers

namespace au::containers
{
  class DynamicRingBuffer
  {
public:
    DynamicRingBuffer(const DynamicRingBuffer &) = delete;
    DynamicRingBuffer &operator=(const DynamicRingBuffer &) = delete;

    DynamicRingBuffer(DynamicRingBuffer &&other) noexcept
        : m_control_block(std::move(other.m_control_block)), m_memory(other.m_memory),
          m_view(m_control_block.get(), m_memory.data_ptr, m_memory.aligned_capacity)
    {
      other.m_memory.data_ptr = nullptr;
    }

    DynamicRingBuffer &operator=(DynamicRingBuffer &&other) noexcept
    {
      if (this != &other)
      {
        if (m_memory.data_ptr)
          MirroredAllocator::deallocate(m_memory);

        m_control_block = std::move(other.m_control_block);
        m_memory = other.m_memory;
        m_view = DynamicRingBufferView(m_control_block.get(), m_memory.data_ptr, m_memory.aligned_capacity);

        other.m_memory.data_ptr = nullptr;
      }
      return *this;
    }

    ~DynamicRingBuffer()
    {
      if (m_memory.data_ptr)
      {
        MirroredAllocator::deallocate(m_memory);
        m_memory.data_ptr = nullptr;
      }
    }

    static auto create(const u32 requested_capacity) -> Result<DynamicRingBuffer>
    {
      MirroredMemory mem = AU_TRY(MirroredAllocator::allocate(requested_capacity));

      auto cb = memory::make_box<ControlBlock>();

      cb->consumer.capacity = mem.aligned_capacity;
      cb->producer.write_offset.store(0, std::memory_order_release);
      cb->consumer.read_offset.store(0, std::memory_order_release);

      return DynamicRingBuffer(std::move(cb), mem);
    }

    auto push(const u16 packet_id, Span<const u8> data) -> Result<void>
    {
      return m_view.push(packet_id, data);
    }

    auto pop(PacketHeader &out_header, Span<u8> out_buffer) -> Result<usize>
    {
      return m_view.pop(out_header, out_buffer);
    }

    auto get_view() -> DynamicRingBufferView &
    {
      return m_view;
    }

private:
    DynamicRingBuffer(memory::Box<ControlBlock> cb, MirroredMemory mem)
        : m_control_block(std::move(cb)), m_memory(mem),
          m_view(m_control_block.get(), m_memory.data_ptr, m_memory.aligned_capacity)
    {
    }

    memory::Box<ControlBlock> m_control_block;
    MirroredMemory m_memory;
    DynamicRingBufferView m_view;
  };

  class FixedRingBuffer
  {
public:
    FixedRingBuffer(const FixedRingBuffer &) = delete;
    FixedRingBuffer &operator=(const FixedRingBuffer &) = delete;

    FixedRingBuffer(FixedRingBuffer &&other) noexcept = default;
    FixedRingBuffer &operator=(FixedRingBuffer &&other) noexcept = default;

    ~FixedRingBuffer()
    {
      if (m_memory.data_ptr)
      {
        MirroredAllocator::deallocate(m_memory);
        m_memory.data_ptr = nullptr;
      }
    }

    static auto create(const u32 requested_capacity, const u32 packet_size) -> Result<FixedRingBuffer>
    {
      if (packet_size == 0)
        return fail("Packet size cannot be 0");

      MirroredMemory mem = AU_TRY(MirroredAllocator::allocate(requested_capacity));
      auto cb = memory::make_box<ControlBlock>();

      cb->consumer.capacity = mem.aligned_capacity;
      cb->producer.write_offset.store(0, std::memory_order_release);
      cb->consumer.read_offset.store(0, std::memory_order_release);

      return FixedRingBuffer(std::move(cb), mem, packet_size);
    }

    auto push(Span<const u8> data) -> Result<void>
    {
      return m_view.push(data);
    }

    auto pop(Span<u8> out_buffer) -> Result<bool>
    {
      return m_view.pop(out_buffer);
    }

    auto get_view() -> FixedRingBufferView &
    {
      return m_view;
    }

private:
    FixedRingBuffer(memory::Box<ControlBlock> cb, MirroredMemory mem, u32 packet_size)
        : m_control_block(std::move(cb)), m_memory(mem),
          m_view(cb.get(), m_memory.data_ptr, m_memory.aligned_capacity, packet_size)
    {
    }

    memory::Box<ControlBlock> m_control_block;
    MirroredMemory m_memory;
    FixedRingBufferView m_view;
  };
} // namespace au::containers