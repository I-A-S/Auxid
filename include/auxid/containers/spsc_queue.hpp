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

#include <auxid/pch.hpp>

#include <atomic>
#include <utility>

namespace au::containers
{
  template<typename T, usize Capacity>
    requires((Capacity != 0) && ((Capacity & (Capacity - 1)) == 0))
  class SpscQueue
  {
public:
    SpscQueue() = default;

    ~SpscQueue()
    {
      T dummy;
      while (pop(dummy))
      {
      }
    }

    SpscQueue(const SpscQueue &) = delete;
    SpscQueue &operator=(const SpscQueue &) = delete;

    [[nodiscard]] bool push(T value)
    {
      const usize write_idx = m_write_pos.load(std::memory_order_relaxed);

      const usize read_idx = m_read_pos.load(std::memory_order_acquire);

      if (write_idx - read_idx == Capacity)
        return false;

      T *slot = reinterpret_cast<T *>(&m_slots[(write_idx & K_MASK) * sizeof(T)]);

      new (slot) T(std::move(value));

      m_write_pos.store(write_idx + 1, std::memory_order_release);
      return true;
    }

    [[nodiscard]] bool pop(T &out_value)
    {
      const usize read_idx = m_read_pos.load(std::memory_order_relaxed);

      const usize write_idx = m_write_pos.load(std::memory_order_acquire);

      if (read_idx == write_idx)
        return false;

      T *slot = reinterpret_cast<T *>(&m_slots[(read_idx & K_MASK) * sizeof(T)]);

      out_value = std::move(*slot);

      slot->~T();

      m_read_pos.store(read_idx + 1, std::memory_order_release);
      return true;
    }

private:
    static constexpr usize K_MASK = Capacity - 1;

    alignas(64) std::atomic<usize> m_write_pos{0};
    alignas(64) std::atomic<usize> m_read_pos{0};

    alignas(T) u8 m_slots[Capacity * sizeof(T)];
  };
} // namespace au::containers