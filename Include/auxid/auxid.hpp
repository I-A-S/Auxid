// Auxid: Explicit Safety and Syntax for C++20.
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

#include <concepts>
#include <cstdint>
#include <new>
#include <source_location>
#include <type_traits>
#include <utility>

#include <format>
#include <string>

#if !defined(__clang__) && !defined(__GNUC__)
#error                                                                         \
    "Auxid requires Clang or GCC for Statement Expressions ({...}). Use clang-cl on Windows."
#endif

#if __cplusplus < 202002L
#error "Auxid requires C++20 or newer."
#endif

// =============================================================================
// Definitions
// =============================================================================

#undef pure_fn
#undef const_fn

#define pure_fn __attribute__((const)) [[nodiscard]]
#define const_fn __attribute__((pure)) [[nodiscard]]
#define AU_UNUSED(v) (void)(v)

namespace auxid {

// =============================================================================
// Primitive Types
// =============================================================================

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;

using usize = std::size_t;
using isize = std::ptrdiff_t;

template <typename T> using Mut = T;
template <typename T> using Ref = const T &;
template <typename T> using MutRef = T &;
template <typename T> using ForwardRef = T &&;

template <typename T>
[[nodiscard]] constexpr decltype(auto) mut(T &&arg) noexcept {
  return std::forward<T>(arg);
}

// =============================================================================
// Error Handling
// =============================================================================

template <typename E> struct Unexpected {
  E val;
  constexpr explicit Unexpected(E &&v) : val(std::move(v)) {}
  constexpr explicit Unexpected(const E &v) : val(v) {}
};

template <typename E> [[nodiscard]] constexpr auto fail(E &&error) {
  return Unexpected<std::decay_t<E>>(std::forward<E>(error));
}

struct ErrorMsg {
  const char *msg;
  constexpr ErrorMsg(const char *m) : msg(m) {}
};

extern void panic_handler(const char *msg, const char *file, u32 line);

[[noreturn]] inline void
panic(const char *msg,
      std::source_location loc = std::source_location::current()) {
  panic_handler(msg, loc.file_name(), loc.line());
  __builtin_trap();
}

template <typename... Args>
[[noreturn]] inline void panic(std::format_string<Args...> fmt,
                               Args &&...args) {
  std::string s = std::format(fmt, std::forward<Args>(args)...);
  panic(s.c_str());
}

// =============================================================================
// Result
// =============================================================================

struct Unit {};

template <typename T, typename E = ErrorMsg> class [[nodiscard]] Result {
  union {
    T m_val;
    E m_err;
  };
  bool m_is_ok;

public:
  constexpr Result(const T &val) : m_val(val), m_is_ok(true) {}
  constexpr Result(T &&val) : m_val(std::move(val)), m_is_ok(true) {}

  template <typename ErrT>
  constexpr Result(Unexpected<ErrT> &&failure)
      : m_err(std::move(failure.val)), m_is_ok(false) {}

  constexpr Result(Result &&other) noexcept : m_is_ok(other.m_is_ok) {
    if (m_is_ok)
      std::construct_at(&m_val, std::move(other.m_val));
    else
      std::construct_at(&m_err, std::move(other.m_err));
  }

  constexpr ~Result() {
    if (m_is_ok) {
      if constexpr (!std::is_trivially_destructible_v<T>)
        std::destroy_at(&m_val);
    } else {
      if constexpr (!std::is_trivially_destructible_v<E>)
        std::destroy_at(&m_err);
    }
  }

  constexpr T &
  unwrap(std::source_location loc = std::source_location::current()) & {
    if (!m_is_ok)
      auxid::panic("Called unwrap() on an Error Result", loc);
    return m_val;
  }

  constexpr const T &
  unwrap(std::source_location loc = std::source_location::current()) const & {
    if (!m_is_ok)
      auxid::panic("Called unwrap() on an Error Result", loc);
    return m_val;
  }

  constexpr T
  unwrap(std::source_location loc = std::source_location::current()) && {
    if (!m_is_ok)
      auxid::panic("Called unwrap() on an Error Result", loc);
    return std::move(m_val);
  }

  constexpr E &
  unwrap_err(std::source_location loc = std::source_location::current()) & {
    if (m_is_ok)
      auxid::panic("Called unwrap_err() on an Ok Result", loc);
    return m_err;
  }

  [[nodiscard]] constexpr bool is_ok() const { return m_is_ok; }
  [[nodiscard]] constexpr bool is_err() const { return !m_is_ok; }

  constexpr T &operator*() & { return unwrap(); }
  constexpr const T &operator*() const & { return unwrap(); }
  constexpr T *operator->() { return &unwrap(); }
};

template <typename E> class [[nodiscard]] Result<void, E> {
  union {
    E m_err;
  };
  bool m_is_ok;

public:
  constexpr Result() : m_is_ok(true) {}

  template <typename ErrT>
  constexpr Result(Unexpected<ErrT> &&failure)
      : m_err(std::move(failure.val)), m_is_ok(false) {}

  constexpr Result(Result &&other) noexcept : m_is_ok(other.m_is_ok) {
    if (!m_is_ok)
      std::construct_at(&m_err, std::move(other.m_err));
  }

  constexpr ~Result() {
    if (!m_is_ok) {
      if constexpr (!std::is_trivially_destructible_v<E>)
        std::destroy_at(&m_err);
    }
  }

  constexpr void
  unwrap(std::source_location loc = std::source_location::current()) const {
    if (!m_is_ok)
      auxid::panic("Called unwrap() on an Error Result", loc);
  }

  [[nodiscard]] constexpr bool is_ok() const { return m_is_ok; }
  [[nodiscard]] constexpr bool is_err() const { return !m_is_ok; }

  constexpr E &
  unwrap_err(std::source_location loc = std::source_location::current()) & {
    if (m_is_ok)
      auxid::panic("Called unwrap_err() on an Ok Result", loc);
    return m_err;
  }
};

} // namespace auxid

// =============================================================================
// Macros
// =============================================================================

#define AU_TRY_PURE(expr)                                                      \
  {                                                                            \
    auto _au_res = (expr);                                                     \
    if (_au_res.is_err()) {                                                    \
      return auxid::fail(std::move(_au_res.unwrap_err()));                     \
    }                                                                          \
  }

#define AU_TRY(expr)                                                           \
  __extension__({                                                              \
    auto _au_res = (expr);                                                     \
    if (_au_res.is_err()) {                                                    \
      return auxid::fail(std::move(_au_res.unwrap_err()));                     \
    }                                                                          \
    std::move(_au_res.unwrap());                                               \
  })

#define AU_TRY_DISCARD(expr)                                                   \
  {                                                                            \
    auto _au_res = (expr);                                                     \
    if (_au_res.is_err()) {                                                    \
      return auxid::fail(std::move(_au_res.unwrap_err()));                     \
    }                                                                          \
  }
