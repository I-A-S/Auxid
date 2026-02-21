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

#include <ranges>
#include <iterator>
#include <algorithm>
#include <type_traits>

namespace au::containers
{
  template<typename T> class Span
  {
public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = usize;
    using difference_type = isize;
    using pointer = T *;
    using const_pointer = const T *;
    using reference = T &;
    using const_reference = const T &;
    using iterator = T *;
    using reverse_iterator = std::reverse_iterator<iterator>;

public:
    constexpr Span() noexcept : m_ptr(nullptr), m_len(0)
    {
    }

    constexpr Span(pointer ptr, size_type count) noexcept : m_ptr(ptr), m_len(count)
    {
    }

    constexpr Span(pointer first, pointer last) noexcept : m_ptr(first), m_len(static_cast<size_type>(last - first))
    {
    }

    template<size_type N> constexpr Span(element_type (&arr)[N]) noexcept : m_ptr(arr), m_len(N)
    {
    }

    constexpr Span(std::initializer_list<value_type> il) noexcept
      requires(std::is_const_v<element_type>)
        : m_ptr(il.begin()), m_len(static_cast<size_type>(il.size()))
    {
    }

    template<typename Range>
      requires(!std::is_same_v<std::remove_cvref_t<Range>, Span> && std::ranges::contiguous_range<Range> &&
               std::ranges::sized_range<Range> && std::ranges::borrowed_range<Range> &&
               std::is_convertible_v<std::ranges::range_reference_t<Range>, reference>)
    constexpr Span(Range &&r) noexcept
        : m_ptr(std::ranges::data(r)), m_len(static_cast<size_type>(std::ranges::size(r)))
    {
    }

    constexpr Span(const Span &other) noexcept = default;
    constexpr Span &operator=(const Span &other) noexcept = default;

    template<typename U>
      requires(std::is_convertible_v<U (*)[], element_type (*)[]>)
    constexpr Span(const Span<U> &other) noexcept : m_ptr(other.data()), m_len(other.size())
    {
    }

public:
    [[nodiscard]] constexpr pointer data() const noexcept
    {
      return m_ptr;
    }

    [[nodiscard]] constexpr size_type size() const noexcept
    {
      return m_len;
    }

    [[nodiscard]] constexpr size_type size_bytes() const noexcept
    {
      return m_len * sizeof(T);
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
      return m_len == 0;
    }

    [[nodiscard]] constexpr reference operator[](size_type idx) const
    {
#if !defined(NDEBUG)
      if (idx >= m_len)
        panic("Span index out of bounds");
#endif
      return m_ptr[idx];
    }

    [[nodiscard]] constexpr reference front() const
    {
#if !defined(NDEBUG)
      if (empty())
        panic("Called front() on empty Span");
#endif
      return m_ptr[0];
    }

    [[nodiscard]] constexpr reference back() const
    {
#if !defined(NDEBUG)
      if (empty())
        panic("Called back() on empty Span");
#endif
      return m_ptr[m_len - 1];
    }

public:
    [[nodiscard]] constexpr iterator begin() const noexcept
    {
      return m_ptr;
    }

    [[nodiscard]] constexpr iterator end() const noexcept
    {
      return m_ptr + m_len;
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() const noexcept
    {
      return reverse_iterator(end());
    }

    [[nodiscard]] constexpr reverse_iterator rend() const noexcept
    {
      return reverse_iterator(begin());
    }

public:
    [[nodiscard]] constexpr Span<T> first(size_type count) const
    {
#if !defined(NDEBUG)
      if (count > m_len)
        panic("Span::first() count > size");
#endif
      return Span<T>(m_ptr, count);
    }

    [[nodiscard]] constexpr Span<T> last(size_type count) const
    {
#if !defined(NDEBUG)
      if (count > m_len)
        panic("Span::last() count > size");
#endif
      return Span<T>(m_ptr + (m_len - count), count);
    }

    [[nodiscard]] constexpr Span<T> subspan(size_type offset, size_type count = static_cast<size_type>(-1)) const
    {
#if !defined(NDEBUG)
      if (offset > m_len)
        panic("Span::subspan() offset > size");
#endif

      size_type remain = m_len - offset;
      if (count == static_cast<size_type>(-1))
        count = remain;

#if !defined(NDEBUG)
      if (count > remain)
        panic("Span::subspan() count > remaining");
#endif

      return Span<T>(m_ptr + offset, count);
    }

public:
    template<size_type Count> [[nodiscard]] constexpr Span<T> first() const
    {
#if !defined(NDEBUG)
      if (Count > m_len)
        panic("Span::first<N>() count > size");
#endif
      return Span<T>(m_ptr, Count);
    }

    template<size_type Count> [[nodiscard]] constexpr Span<T> last() const
    {
#if !defined(NDEBUG)
      if (Count > m_len)
        panic("Span::last<N>() count > size");
#endif
      return Span<T>(m_ptr + (m_len - Count), Count);
    }

    template<size_type Offset, size_type Count = static_cast<size_type>(-1)>
    [[nodiscard]] constexpr Span<T> subspan() const
    {
#if !defined(NDEBUG)
      if (Offset > m_len)
        panic("Span::subspan<Offset, Count>() offset > size");
#endif

      if constexpr (Count == static_cast<size_type>(-1))
      {
        return Span<T>(m_ptr + Offset, m_len - Offset);
      }
      else
      {
#if !defined(NDEBUG)
        if (Count > m_len - Offset)
          panic("Span::subspan<Offset, Count>() count > remaining");
#endif
        return Span<T>(m_ptr + Offset, Count);
      }
    }

    template<typename U>
    [[nodiscard]] constexpr bool operator==(const Span<U> &rhs) const noexcept
      requires(requires { std::ranges::equal(*this, rhs); })
    {
      return std::ranges::equal(*this, rhs);
    }

public:
    [[nodiscard]] auto as_bytes() const noexcept
    {
      return Span<const u8>(reinterpret_cast<const u8 *>(m_ptr), m_len * sizeof(T));
    }

    [[nodiscard]] auto as_writable_bytes() const noexcept
      requires(!std::is_const_v<T>)
    {
      return Span<u8>(reinterpret_cast<u8 *>(m_ptr), m_len * sizeof(T));
    }

private:
    pointer m_ptr;
    size_type m_len;
  };

  template<typename T, usize N> Span(T (&)[N]) -> Span<T>;
  template<typename T> Span(std::initializer_list<T>) -> Span<const T>;
  template<std::ranges::contiguous_range Range>
  Span(Range &&) -> Span<std::remove_reference_t<std::ranges::range_reference_t<Range>>>;

} // namespace au::containers

namespace au
{
  template<typename T> using Span = containers::Span<T>;
}

template<typename T> inline constexpr bool std::ranges::enable_borrowed_range<au::containers::Span<T>> = true;
