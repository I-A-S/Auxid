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

#include <auxid/result.hpp>
#include <auxid/containers/vec.hpp>

#include <atomic>

namespace au::containers
{
  struct ControlBlock
  {
    struct alignas(64)
    {
      Mut<std::atomic<u32>> write_offset{0};
    } producer;

    struct alignas(64)
    {
      Mut<std::atomic<u32>> read_offset{0};
      Mut<u32> capacity{0};
    } consumer;
  };

  static_assert(offsetof(ControlBlock, consumer) == 64, "False sharing detected in ControlBlock");

  struct PacketHeader
  {
    u16 id{0};
    u16 payload_size{0};
  };

  class DynamicRingBufferView
  {
public:
    auto pop(PacketHeader &out_header, Span<u8> out_buffer) -> Result<usize>;
    auto push(const u16 packet_id, Span<const u8> data) -> Result<void>;

protected:
    // `data` must point to a *mirrored memory region* of size (capacity * 2) + sizeof(ControlBlock)
    DynamicRingBufferView(ControlBlock *cb, u8 *data, u32 cap) : m_control_block(cb), m_data_ptr(data), m_capacity(cap)
    {
    }

    Mut<u8 *> m_data_ptr{};
    Mut<u32> m_capacity{};
    Mut<ControlBlock *> m_control_block{};

    auto write_mirrored(const u32 offset, const void *data, const u32 size) -> void
    {
      std::memcpy(m_data_ptr + (offset & (m_capacity - 1)), data, size);
    }

    auto read_mirrored(const u32 offset, void *out_data, const u32 size) -> void
    {
      std::memcpy(out_data, m_data_ptr + (offset & (m_capacity - 1)), size);
    }

    friend class DynamicRingBuffer;
  };

  inline auto DynamicRingBufferView::push(const u16 packet_id, Span<const u8> data) -> Result<void>
  {
    const u32 total_size = sizeof(PacketHeader) + static_cast<u32>(data.size());
    if (total_size > m_capacity)
      return fail("Packet larger than buffer capacity");

    const u32 write = m_control_block->producer.write_offset.load(std::memory_order_relaxed);

    u32 read = m_control_block->consumer.read_offset.load(std::memory_order_acquire);
    u32 used_space = write - read;
    u32 free_space = m_capacity - used_space;

    if (free_space < total_size)
      return fail("Buffer full. Cannot push packet.");

    const PacketHeader header{packet_id, static_cast<u16>(data.size())};
    write_mirrored(write, &header, sizeof(PacketHeader));

    if (!data.empty())
    {
      write_mirrored(write + sizeof(PacketHeader), data.data(), static_cast<u32>(data.size()));
    }

    m_control_block->producer.write_offset.store(write + total_size, std::memory_order_release);
    return {};
  }

  inline auto DynamicRingBufferView::pop(PacketHeader &out_header, Span<u8> out_buffer) -> Result<usize>
  {
    u32 read = m_control_block->consumer.read_offset.load(std::memory_order_relaxed);
    const u32 write = m_control_block->producer.write_offset.load(std::memory_order_acquire);

    if (read == write)
      return 0;

    read_mirrored(read, &out_header, sizeof(PacketHeader));

    if (out_header.payload_size > out_buffer.size())
      return fail("Buffer too small");

    if (out_header.payload_size > 0)
    {
      read_mirrored(read + sizeof(PacketHeader), out_buffer.data(), out_header.payload_size);
    }

    const u32 new_read = read + sizeof(PacketHeader) + out_header.payload_size;
    m_control_block->consumer.read_offset.compare_exchange_strong(read, new_read, std::memory_order_release,
                                                                  std::memory_order_relaxed);

    return static_cast<usize>(sizeof(PacketHeader) + out_header.payload_size);
  }
} // namespace au::containers

namespace au::containers
{
  class FixedRingBufferView : public DynamicRingBufferView
  {
private:
    u32 m_packet_size;

public:
    auto pop(Span<u8> out_buffer) -> Result<bool>;
    auto push(Span<const u8> data) -> Result<void>;

protected:
    // `data` must point to a *mirrored memory region* of size (capacity * 2) + sizeof(ControlBlock)
    FixedRingBufferView(ControlBlock *cb, u8 *data, u32 cap, u32 packet_size)
        : DynamicRingBufferView(cb, data, cap), m_packet_size(packet_size)
    {
    }

    friend class FixedRingBuffer;
  };

  inline auto FixedRingBufferView::push(Span<const u8> data) -> Result<void>
  {
    if (data.size() != m_packet_size)
      return fail("Invalid fixed packet size");

    const u32 write = m_control_block->producer.write_offset.load(std::memory_order_relaxed);

    u32 read = m_control_block->consumer.read_offset.load(std::memory_order_acquire);
    u32 free_space = m_capacity - (write - read);

    if (free_space < m_packet_size)
      return fail("Buffer full. Cannot push packet.");

    write_mirrored(write, data.data(), m_packet_size);
    m_control_block->producer.write_offset.store(write + m_packet_size, std::memory_order_release);
    return {};
  }
} // namespace au::containers