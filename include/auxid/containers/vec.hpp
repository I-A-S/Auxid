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

#include <auxid/containers/span.hpp>
#include <auxid/memory/heap.hpp>
#include <auxid/pch.hpp>

#include <cstring>

namespace au::containers
{
  template<typename T, typename size_type = usize, typename AllocatorT = memory::HeapAllocator>
    requires memory::AllocatorType<AllocatorT>
  class VecT
  {
    T *m_data = nullptr;
    size_type m_size = 0;
    size_type m_capacity = 0;
    AUXID_NO_UNIQUE_ADDRESS AllocatorT m_allocator;

public:
    explicit VecT() = default;

    explicit VecT(size_type initial_capacity)
    {
      reserve(initial_capacity);
    }

    VecT(std::initializer_list<T> init)
    {
      reserve(static_cast<size_type>(init.size()));
      if constexpr (std::is_trivially_copyable_v<T>)
      {
        if (init.size() > 0)
        {
          std::memcpy(m_data, init.begin(), init.size() * sizeof(T));
        }
        m_size = static_cast<size_type>(init.size());
      }
      else
      {
        for (const auto &item : init)
        {
          push_back(item);
        }
      }
    }

    VecT(VecT &&other) noexcept
        : m_data(other.m_data), m_size(other.m_size), m_capacity(other.m_capacity),
          m_allocator(std::move(other.m_allocator))
    {
      other.m_data = nullptr;
      other.m_size = 0;
      other.m_capacity = 0;
    }

    VecT &operator=(VecT &&other) noexcept
    {
      if (this != &other)
      {
        clear();
        if (m_data)
          m_allocator.free(m_data, m_capacity * sizeof(T), alignof(T));

        m_allocator = std::move(other.m_allocator);
        m_data = other.m_data;
        m_size = other.m_size;
        m_capacity = other.m_capacity;

        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
      }
      return *this;
    }

    VecT(const VecT &other)
    {
      reserve(other.m_size);

      if constexpr (std::is_trivially_copyable_v<T>)
      {
        if (other.m_size > 0)
          std::memcpy(m_data, other.m_data, other.m_size * sizeof(T));
        m_size = other.m_size;
      }
      else
      {
        for (size_type i = 0; i < other.m_size; ++i)
        {
          construct_at(&m_data[i], other.m_data[i]);
        }
        m_size = other.m_size;
      }
    }

    VecT &operator=(const VecT &other)
    {
      if (this != &other)
      {
        clear();
        reserve(other.m_size);

        if constexpr (std::is_trivially_copyable_v<T>)
        {
          if (other.m_size > 0)
            std::memcpy(m_data, other.m_data, other.m_size * sizeof(T));
          m_size = other.m_size;
        }
        else
        {
          for (size_type i = 0; i < other.m_size; ++i)
          {
            construct_at(&m_data[i], other.m_data[i]);
          }
          m_size = other.m_size;
        }
      }
      return *this;
    }

    ~VecT()
    {
      clear();
      if (m_data)
        m_allocator.free(m_data, m_capacity * sizeof(T), alignof(T));
    }

public:
    [[nodiscard]] VecT clone() const
    {
      return VecT(*this);
    }

    template<typename... Args> T &emplace_back(Args &&...args)
    {
      if (m_size >= m_capacity)
      {
        grow();
      }
      construct_at(&m_data[m_size], std::forward<Args>(args)...);
      return m_data[m_size++];
    }

    void push(const T &val)
    {
      emplace_back(val);
    }

    void push(T &&val)
    {
      emplace_back(std::move(val));
    }

    void push_back(const T &val)
    {
      emplace_back(val);
    }

    void push_back(T &&val)
    {
      emplace_back(std::move(val));
    }

    void pop()
    {
      if (m_size > 0)
      {
        m_size--;
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
          au::destroy_at(&m_data[m_size]);
        }
      }
    }

    void pop_back()
    {
      pop();
    }

    void clear()
    {
      if constexpr (!std::is_trivially_destructible_v<T>)
      {
        for (size_type i = 0; i < m_size; ++i)
        {
          au::destroy_at(&m_data[i]);
        }
      }
      m_size = 0;
    }

    void reserve(size_type new_cap)
    {
      if (new_cap <= m_capacity)
        return;

      if constexpr (std::is_trivially_copyable_v<T>)
      {
        if (m_data)
        {
          void *ptr = m_allocator.realloc(m_data, m_capacity * sizeof(T), new_cap * sizeof(T), alignof(T));
          if (ptr)
          {
            m_data = static_cast<T *>(ptr);
            m_capacity = new_cap;
            return;
          }
        }
      }

      T *new_data = static_cast<T *>(m_allocator.alloc(new_cap * sizeof(T), alignof(T)));

      if (m_data)
      {
        if constexpr (std::is_trivially_copyable_v<T>)
        {
          std::memcpy(new_data, m_data, m_size * sizeof(T));
        }
        else
        {
          for (size_type i = 0; i < m_size; ++i)
          {
            construct_at(&new_data[i], std::move(m_data[i]));
            au::destroy_at(&m_data[i]);
          }
        }
        m_allocator.free(m_data, m_capacity * sizeof(T), alignof(T));
      }

      m_data = new_data;
      m_capacity = new_cap;
    }

    void resize(size_type new_size)
    {
      if (new_size > m_size)
      {
        if (new_size > m_capacity)
          reserve(new_size);
        for (size_type i = m_size; i < new_size; ++i)
        {
          construct_at(&m_data[i]);
        }
      }
      else if (new_size < m_size)
      {
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
          for (size_type i = new_size; i < m_size; ++i)
          {
            au::destroy_at(&m_data[i]);
          }
        }
      }
      m_size = new_size;
    }

    void resize(size_type new_size, const T &fill_val)
    {
      if (new_size > m_size)
      {
        if (new_size > m_capacity)
          reserve(new_size);
        for (size_type i = m_size; i < new_size; ++i)
        {
          construct_at(&m_data[i], fill_val);
        }
      }
      else if (new_size < m_size)
      {
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
          for (size_type i = new_size; i < m_size; ++i)
          {
            au::destroy_at(&m_data[i]);
          }
        }
      }
      m_size = new_size;
    }

public:
    [[nodiscard]] size_type size() const
    {
      return m_size;
    }

    [[nodiscard]] size_type capacity() const
    {
      return m_capacity;
    }

    [[nodiscard]] bool empty() const
    {
      return m_size == 0;
    }

    [[nodiscard]] T *data()
    {
      return m_data;
    }

    [[nodiscard]] const T *data() const
    {
      return m_data;
    }

    T &operator[](size_type idx)
    {
#if !defined(NDEBUG)
      if (idx >= m_size)
        panic("VecT index out of bounds");
#endif
      return m_data[idx];
    }

    const T &operator[](size_type idx) const
    {
#if !defined(NDEBUG)
      if (idx >= m_size)
        panic("VecT index out of bounds");
#endif
      return m_data[idx];
    }

    T *begin()
    {
      return m_data;
    }

    T *end()
    {
      return m_data + m_size;
    }

    const T *begin() const
    {
      return m_data;
    }

    const T *end() const
    {
      return m_data + m_size;
    }

    T &back()
    {
      return m_data[m_size - 1];
    }

    const T &back() const
    {
      return m_data[m_size - 1];
    }

    operator Span<T>()
    {
      return Span<T>(m_data, m_size);
    }

    operator Span<const T>() const
    {
      return Span<const T>(m_data, m_size);
    }

    [[nodiscard]] Span<T> as_span()
    {
      return Span<T>(m_data, m_size);
    }

    [[nodiscard]] Span<const T> as_span() const
    {
      return Span<const T>(m_data, m_size);
    }

private:
    void grow()
    {
      const size_type new_cap = (m_capacity == 0) ? 8 : m_capacity + (m_capacity / 2) + 1;
      reserve(new_cap);
    }
  };
} // namespace au::containers

namespace au
{
  template<typename T> using Vec = containers::VecT<T, usize>;
  template<typename T> using TinyVec = containers::VecT<T, u16>;
  template<typename T> using CompactVec = containers::VecT<T, u32>;
} // namespace au