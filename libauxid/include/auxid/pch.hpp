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

#include <new>
#include <utility>
#include <malloc.h>
#include <stdint.h>
#include <source_location>
#include <type_traits>

#if !defined(__clang__) && !defined(__GNUC__)
#  error "Auxid requires Clang or GCC for Statement Expressions ({...}). Use clang-cl on Windows."
#endif

#if __cplusplus < 202002L
#  error "Auxid requires C++20 or newer."
#endif

// =============================================================================
// Definitions
// =============================================================================

#undef pure_fn
#undef const_fn

#define pure_fn __attribute__((const)) [[nodiscard]]
#define const_fn __attribute__((pure)) [[nodiscard]]

#define AU_UNUSED(v) (void) (v)

namespace au
{
  // =============================================================================
  // Primitive Types
  // =============================================================================

  using u8 = uint8_t;
  using u16 = uint16_t;
  using u32 = uint32_t;
  using u64 = uint64_t;

  using i8 = int8_t;
  using i16 = int16_t;
  using i32 = int32_t;
  using i64 = int64_t;

  using f32 = float;
  using f64 = double;

  using usize = std::size_t;
  using isize = std::ptrdiff_t;
  using alignval = std::align_val_t;

  template<typename T> using Mut = T;

  template<typename T> [[nodiscard]] constexpr decltype(auto) mut(T &&arg) noexcept
  {
    return std::forward<T>(arg);
  }

  // =============================================================================
  // C++ STDLIB Polyfills
  // =============================================================================
  template<class T> constexpr T *addressof(T &arg) noexcept
  {
    return __builtin_addressof(arg);
  }
  template<class T> const T *addressof(const T &&) = delete;

  template<class T, class... Args> constexpr T *construct_at(T *p, Args &&...args)
  {
    return ::new (static_cast<void *>(p)) T(static_cast<Args &&>(args)...);
  }

  template<class T> constexpr void destroy_at(T *p)
  {
    if constexpr (std::is_array_v<T>)
    {
      for (auto &elem : *p)
        destroy_at(__builtin_addressof(elem));
    }
    else
    {
      if constexpr (!std::is_trivially_destructible_v<T>)
      {
        p->~T();
      }
    }
  }

  static void *aligned_alloc(size_t alignment, size_t size)
  {
    if ((alignment & (alignment - 1)) != 0 || alignment == 0)
      return nullptr;

    size_t total_size = size + alignment + sizeof(void *);
    void *raw = malloc(total_size);
    if (!raw)
      return nullptr;

    uintptr_t raw_addr = reinterpret_cast<uintptr_t>(raw);
    uintptr_t start_search = raw_addr + sizeof(void *);
    uintptr_t aligned_addr = (start_search + (alignment - 1)) & ~(alignment - 1);

    void **storage = reinterpret_cast<void **>(aligned_addr);
    storage[-1] = raw;

    return reinterpret_cast<void *>(aligned_addr);
  }

  static void aligned_free(void *aligned_ptr)
  {
    if (!aligned_ptr)
      return;
    void **storage = static_cast<void **>(aligned_ptr);
    free(storage[-1]);
  }

  // =============================================================================
  // Error Handling
  // =============================================================================

  template<typename E> struct Unexpected
  {
    E val;

    constexpr explicit Unexpected(E &&v) : val(std::move(v))
    {
    }

    constexpr explicit Unexpected(const E &v) : val(v)
    {
    }
  };

  template<typename E> [[nodiscard]] constexpr auto fail(E &&error)
  {
    return Unexpected<std::decay_t<E>>(std::forward<E>(error));
  }

  extern auto panic_handler(const char *msg, const char *file, u32 line) -> void;

  [[noreturn]] inline auto panic(const char *msg, std::source_location loc = std::source_location::current()) -> void
  {
    panic_handler(msg, loc.file_name(), loc.line());
    __builtin_trap();
  }

  template<typename T, typename E> class [[nodiscard]] ResultT
  {
    union {
      T m_val;
      E m_err;
    };

    bool m_is_ok;

public:
    constexpr ResultT(const T &val) : m_val(val), m_is_ok(true)
    {
    }

    constexpr ResultT(T &&val) : m_val(std::move(val)), m_is_ok(true)
    {
    }

    template<typename ErrT>
    constexpr ResultT(Unexpected<ErrT> &&failure) : m_err(std::move(failure.val)), m_is_ok(false)
    {
    }

    constexpr ResultT(ResultT &&other) noexcept : m_is_ok(other.m_is_ok)
    {
      if (m_is_ok)
        construct_at(&m_val, std::move(other.m_val));
      else
        construct_at(&m_err, std::move(other.m_err));
    }

    constexpr ~ResultT()
    {
      if (m_is_ok)
      {
        if constexpr (!std::is_trivially_destructible_v<T>)
          destroy_at(&m_val);
      }
      else
      {
        if constexpr (!std::is_trivially_destructible_v<E>)
          destroy_at(&m_err);
      }
    }

    constexpr T &unwrap(std::source_location loc = std::source_location::current()) &
    {
      if (!m_is_ok)
        au::panic("Called unwrap() on an Error Result", loc);
      return m_val;
    }

    constexpr const T &unwrap(std::source_location loc = std::source_location::current()) const &
    {
      if (!m_is_ok)
        au::panic("Called unwrap() on an Error Result", loc);
      return m_val;
    }

    constexpr T unwrap(std::source_location loc = std::source_location::current()) &&
    {
      if (!m_is_ok)
        au::panic("Called unwrap() on an Error Result", loc);
      return std::move(m_val);
    }

    constexpr const E &unwrap_err(std::source_location loc = std::source_location::current()) const &
    {
      if (m_is_ok)
        au::panic("Called unwrap_err() on an Ok Result", loc);
      return m_err;
    }

    constexpr const E &err(std::source_location loc = std::source_location::current()) const &
    {
      return unwrap_err(loc);
    }

    constexpr const E &error(std::source_location loc = std::source_location::current()) const &
    {
      return unwrap_err(loc);
    }

    [[nodiscard]] constexpr bool is_ok() const
    {
      return m_is_ok;
    }

    [[nodiscard]] constexpr bool is_err() const
    {
      return !m_is_ok;
    }

    [[nodiscard]] constexpr bool has_value() const
    {
      return m_is_ok;
    }

    constexpr T &operator*() &
    {
      return unwrap();
    }

    constexpr const T &operator*() const &
    {
      return unwrap();
    }

    constexpr T *operator->()
    {
      return &unwrap();
    }

    constexpr const T *operator->() const
    {
      return &unwrap();
    }

    constexpr operator bool() const
    {
      return is_ok();
    }
  };

  template<typename E> class [[nodiscard]] ResultT<void, E>
  {
    union {
      E m_err;
    };

    bool m_is_ok;

public:
    constexpr ResultT() : m_is_ok(true)
    {
    }

    template<typename ErrT>
    constexpr ResultT(Unexpected<ErrT> &&failure) : m_err(std::move(failure.val)), m_is_ok(false)
    {
    }

    constexpr ResultT(ResultT &&other) noexcept : m_is_ok(other.m_is_ok)
    {
      if (!m_is_ok)
        construct_at(&m_err, std::move(other.m_err));
    }

    constexpr ~ResultT()
    {
      if (!m_is_ok)
      {
        if constexpr (!std::is_trivially_destructible_v<E>)
          destroy_at(&m_err);
      }
    }

    constexpr void unwrap(std::source_location loc = std::source_location::current()) const
    {
      if (!m_is_ok)
        au::panic("Called unwrap() on an Error Result", loc);
    }

    [[nodiscard]] constexpr bool is_ok() const
    {
      return m_is_ok;
    }

    [[nodiscard]] constexpr bool is_err() const
    {
      return !m_is_ok;
    }

    [[nodiscard]] constexpr bool has_value() const
    {
      return m_is_ok;
    }

    constexpr const E &unwrap_err(std::source_location loc = std::source_location::current()) const &
    {
      if (m_is_ok)
        au::panic("Called unwrap_err() on an Ok Result", loc);
      return m_err;
    }

    constexpr const E &err(std::source_location loc = std::source_location::current()) const &
    {
      return unwrap_err(loc);
    }

    constexpr const E &error(std::source_location loc = std::source_location::current()) const &
    {
      return unwrap_err(loc);
    }

    constexpr operator bool() const
    {
      return is_ok();
    }
  };

} // namespace au

// =============================================================================
// Macros
// =============================================================================

#define AU_LIKELY(v) (v) [[likely]]
#define AU_UNLIKELY(v) (v) [[unlikely]]

#define AU_TRY_PURE(expr)                                                                                              \
  {                                                                                                                    \
    auto _au_res = (expr);                                                                                             \
    if (_au_res.is_err())                                                                                              \
    {                                                                                                                  \
      return au::fail(std::move(_au_res.unwrap_err()));                                                                \
    }                                                                                                                  \
  }

#define AU_TRY(expr)                                                                                                   \
  __extension__({                                                                                                      \
    auto _au_res = (expr);                                                                                             \
    if (_au_res.is_err())                                                                                              \
    {                                                                                                                  \
      return au::fail(std::move(_au_res.unwrap_err()));                                                                \
    }                                                                                                                  \
    std::move(_au_res.unwrap());                                                                                       \
  })

#define AU_TRY_DISCARD(expr)                                                                                           \
  {                                                                                                                    \
    auto _au_res = (expr);                                                                                             \
    if (_au_res.is_err())                                                                                              \
    {                                                                                                                  \
      return au::fail(std::move(_au_res.unwrap_err()));                                                                \
    }                                                                                                                  \
  }
