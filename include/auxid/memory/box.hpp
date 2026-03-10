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
  template<typename T, AllocatorType Allocator = HeapAllocator> class Box
  {
public:
    constexpr explicit Box(T *ptr, Allocator alloc = Allocator{}) noexcept : m_ptr(ptr), m_alloc(alloc)
    {
    }

    constexpr ~Box()
    {
      reset();
    }

    constexpr Box(Box &&other) noexcept : m_ptr(other.m_ptr), m_alloc(other.m_alloc)
    {
      other.m_ptr = nullptr;
    }

    constexpr Box &operator=(Box &&other) noexcept
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

    Box(const Box &) = delete;
    Box &operator=(const Box &) = delete;

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
        m_ptr->~T();
        m_alloc.free(m_ptr, sizeof(T), alignof(T));
        m_ptr = nullptr;
      }
    }

    [[nodiscard]] constexpr T *leak() noexcept
    {
      T *ptr = m_ptr;
      m_ptr = nullptr;
      return ptr;
    }

    [[nodiscard]] constexpr auto operator<=>(const Box &rhs) const noexcept
    {
      return m_ptr <=> rhs.m_ptr;
    }

    [[nodiscard]] constexpr bool operator==(const Box &rhs) const noexcept
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
  [[nodiscard]] Box<T, Allocator> make_box(Allocator alloc, Args &&...args)
  {
    void *mem = alloc.alloc(sizeof(T), alignof(T));
    if (!mem)
      panic("make_box allocation failed");

    T *ptr = new (mem) T(static_cast<Args &&>(args)...);
    return Box<T, Allocator>(ptr, alloc);
  }

  template<typename T, typename... Args> [[nodiscard]] Box<T, HeapAllocator> make_box(Args &&...args)
  {
    return make_box<T, HeapAllocator>(HeapAllocator{}, static_cast<Args &&>(args)...);
  }

  template<typename T, AllocatorType Allocator = HeapAllocator, typename... Args>
  [[nodiscard]] Box<T, Allocator> make_box_protected(Allocator alloc, Args &&...args)
  {
    struct Enabler : public T
    {
      constexpr explicit Enabler(Args &&...fwd_args) : T(static_cast<Args &&>(fwd_args)...)
      {
      }
    };

    void *mem = alloc.alloc(sizeof(Enabler), alignof(Enabler));
    if (!mem)
      panic("make_box_protected allocation failed");

    T *ptr = new (mem) Enabler(static_cast<Args &&>(args)...);
    return Box<T, Allocator>(ptr, alloc);
  }

  template<typename T, typename... Args> [[nodiscard]] Box<T, HeapAllocator> make_box_protected(Args &&...args)
  {
    return make_box_protected<T, HeapAllocator>(HeapAllocator{}, static_cast<Args &&>(args)...);
  }
} // namespace au::memory
