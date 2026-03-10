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

#include <auxid/memory/heap.hpp>

namespace au::memory
{
  class RefCounted
  {
public:
    constexpr RefCounted() noexcept = default;

    constexpr RefCounted(const RefCounted &) noexcept : m_ref_count(0)
    {
    }

    constexpr RefCounted &operator=(const RefCounted &) noexcept
    {
      return *this;
    }

    inline void arc_retain() const noexcept
    {
      __atomic_add_fetch(&m_ref_count, 1, __ATOMIC_RELAXED);
    }

    [[nodiscard]] inline bool arc_release() const noexcept
    {
      return __atomic_sub_fetch(&m_ref_count, 1, __ATOMIC_ACQ_REL) == 0;
    }

    [[nodiscard]] inline u32 arc_count() const noexcept
    {
      return __atomic_load_n(&m_ref_count, __ATOMIC_RELAXED);
    }

private:
    mutable u32 m_ref_count{0};
  };

  template<typename T, AllocatorType Allocator = HeapAllocator> class Arc
  {
public:
    constexpr Arc() noexcept : m_ptr(nullptr), m_alloc(Allocator{})
    {
    }

    constexpr explicit Arc(T *ptr, Allocator alloc = Allocator{}) noexcept : m_ptr(ptr), m_alloc(alloc)
    {
      if (m_ptr)
        m_ptr->arc_retain();
    }

    constexpr ~Arc()
    {
      reset();
    }

    constexpr Arc(const Arc &other) noexcept : m_ptr(other.m_ptr), m_alloc(other.m_alloc)
    {
      if (m_ptr)
        m_ptr->arc_retain();
    }

    constexpr Arc &operator=(const Arc &other) noexcept
    {
      if (this != &other)
      {
        reset();
        m_ptr = other.m_ptr;
        m_alloc = other.m_alloc;
        if (m_ptr)
          m_ptr->arc_retain();
      }
      return *this;
    }

    constexpr Arc(Arc &&other) noexcept : m_ptr(other.m_ptr), m_alloc(other.m_alloc)
    {
      other.m_ptr = nullptr;
    }

    constexpr Arc &operator=(Arc &&other) noexcept
    {
      if (this != &other)
      {
        reset();
        m_ptr = other.m_ptr;
        m_alloc = other.m_alloc;
        other.m_ptr = nullptr;
      }
      return *this;
    }

    [[nodiscard]] constexpr T *get() const noexcept
    {
      return m_ptr;
    }

    [[nodiscard]] constexpr T *operator->() const noexcept
    {
      return m_ptr;
    }

    [[nodiscard]] constexpr T &operator*() const noexcept
    {
      return *m_ptr;
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
      return m_ptr != nullptr;
    }

    constexpr void reset() noexcept
    {
      if (m_ptr)
      {
        if (m_ptr->arc_release())
        {
          m_ptr->~T();
          m_alloc.free(m_ptr, sizeof(T), alignof(T));
        }
        m_ptr = nullptr;
      }
    }

    [[nodiscard]] constexpr auto operator<=>(const Arc &rhs) const noexcept
    {
      return m_ptr <=> rhs.m_ptr;
    }

    [[nodiscard]] constexpr bool operator==(const Arc &rhs) const noexcept
    {
      return m_ptr == rhs.m_ptr;
    }

    [[nodiscard]] constexpr auto operator<=>(decltype(nullptr)) const noexcept
    {
      return m_ptr <=> nullptr;
    }

    [[nodiscard]] constexpr bool operator==(decltype(nullptr)) const noexcept
    {
      return m_ptr == nullptr;
    }

private:
    T *m_ptr;
    [[no_unique_address]] Allocator m_alloc;
  };

  template<typename T, AllocatorType Allocator = HeapAllocator, typename... Args>
  [[nodiscard]] Arc<T, Allocator> make_arc(Allocator alloc, Args &&...args)
  {
    void *mem = alloc.alloc(sizeof(T), alignof(T));
    if (!mem)
      panic("make_arc allocation failed");

    T *ptr = new (mem) T(static_cast<Args &&>(args)...);
    return Arc<T, Allocator>(ptr, alloc);
  }

  template<typename T, typename... Args> [[nodiscard]] Arc<T, HeapAllocator> make_arc(Args &&...args)
  {
    return make_arc<T, HeapAllocator>(HeapAllocator{}, static_cast<Args &&>(args)...);
  }

  template<typename T, AllocatorType Allocator = HeapAllocator, typename... Args>
  [[nodiscard]] Arc<T, Allocator> make_arc_protected(Allocator alloc, Args &&...args)
  {
    struct Enabler : public T
    {
      constexpr explicit Enabler(Args &&...fwd_args) : T(static_cast<Args &&>(fwd_args)...)
      {
      }
    };

    void *mem = alloc.alloc(sizeof(Enabler), alignof(Enabler));
    if (!mem)
      panic("make_arc_protected allocation failed");

    T *ptr = new (mem) Enabler(static_cast<Args &&>(args)...);
    return Arc<T, Allocator>(ptr, alloc);
  }

  template<typename T, typename... Args> [[nodiscard]] Arc<T, HeapAllocator> make_arc_protected(Args &&...args)
  {
    return make_arc_protected<T, HeapAllocator>(HeapAllocator{}, static_cast<Args &&>(args)...);
  }
} // namespace au::memory
