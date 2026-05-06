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
  template<typename T, typename AllocatorType> struct BoxAllocatorDeleter
  {
    AUXID_NO_UNIQUE_ADDRESS AllocatorType m_alloc;

    constexpr BoxAllocatorDeleter() noexcept : m_alloc(AllocatorType{})
    {
    }

    constexpr explicit BoxAllocatorDeleter(AllocatorType alloc) noexcept : m_alloc(static_cast<AllocatorType &&>(alloc))
    {
    }

    constexpr void operator()(T *ptr) noexcept
    {
      if (ptr)
      {
        ptr->~T();
        m_alloc.free(ptr, sizeof(T), alignof(T));
      }
    }
  };

  template<typename T, typename Deleter = BoxAllocatorDeleter<T, HeapAllocator>> class Box
  {
public:
    constexpr Box() noexcept : m_ptr(nullptr), m_deleter()
    {
    }

    constexpr explicit Box(T *ptr, Deleter deleter = Deleter{}) noexcept
        : m_ptr(ptr), m_deleter(static_cast<Deleter &&>(deleter))
    {
    }

    constexpr ~Box()
    {
      reset();
    }

    constexpr Box(Box &&other) noexcept : m_ptr(other.m_ptr), m_deleter(static_cast<Deleter &&>(other.m_deleter))
    {
      other.m_ptr = nullptr;
    }

    constexpr Box &operator=(Box &&other) noexcept
    {
      if (this != &other)
      {
        reset();
        m_ptr = other.m_ptr;
        m_deleter = static_cast<Deleter &&>(other.m_deleter);
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

    constexpr void reset(T *ptr = nullptr) noexcept
    {
      if (m_ptr != ptr)
      {
        if (m_ptr)
        {
          m_deleter(m_ptr);
        }
        m_ptr = ptr;
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
    AUXID_NO_UNIQUE_ADDRESS Deleter m_deleter;
  };

  template<typename T, AllocatorType Allocator = HeapAllocator, typename... Args>
  [[nodiscard]] Box<T, BoxAllocatorDeleter<T, Allocator>> make_box(Allocator alloc, Args &&...args)
  {
    void *mem = alloc.alloc(sizeof(T), alignof(T));
    if (!mem)
      panic("make_box allocation failed");

    T *ptr = new (mem) T(static_cast<Args &&>(args)...);

    return Box<T, BoxAllocatorDeleter<T, Allocator>>(ptr, BoxAllocatorDeleter<T, Allocator>(alloc));
  }

  template<typename T, typename... Args>
  [[nodiscard]] Box<T, BoxAllocatorDeleter<T, HeapAllocator>> make_box(Args &&...args)
  {
    return make_box<T, HeapAllocator>(HeapAllocator{}, static_cast<Args &&>(args)...);
  }

  template<typename T, AllocatorType Allocator = HeapAllocator, typename... Args>
  [[nodiscard]] Box<T, BoxAllocatorDeleter<T, Allocator>> make_box_protected(Allocator alloc, Args &&...args)
  {
    struct Enabler : public T
    {
      constexpr explicit Enabler(Args &&...fwd_args) : T(static_cast<Args &&>(fwd_args)...)
      {
      }
    };

    void *mem = alloc.alloc(sizeof(T), alignof(T));
    if (!mem)
      panic("make_box_protected allocation failed");

    T *ptr = new (mem) Enabler(static_cast<Args &&>(args)...);
    return Box<T, BoxAllocatorDeleter<T, Allocator>>(ptr, BoxAllocatorDeleter<T, Allocator>(alloc));
  }

  template<typename T, typename... Args>
  [[nodiscard]] Box<T, BoxAllocatorDeleter<T, HeapAllocator>> make_box_protected(Args &&...args)
  {
    return make_box_protected<T, HeapAllocator>(HeapAllocator{}, static_cast<Args &&>(args)...);
  }

#define AU_DECLARE_CUSTOM_DELETER(type, deleter_function)                                                              \
  template<typename AllocatorType> struct Deleter_##type                                                               \
  {                                                                                                                    \
    AUXID_NO_UNIQUE_ADDRESS AllocatorType m_alloc;                                                                     \
                                                                                                                       \
    constexpr Deleter_##type() noexcept : m_alloc(AllocatorType{})                                                     \
    {                                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    constexpr explicit Deleter_##type(AllocatorType alloc) noexcept : m_alloc(static_cast<AllocatorType &&>(alloc))    \
    {                                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    constexpr void operator()(type *ptr) noexcept                                                                      \
    {                                                                                                                  \
      deleter_function(ptr);                                                                                           \
    }                                                                                                                  \
  };
} // namespace au::memory
