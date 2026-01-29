// Auxid: Rust like safety and syntax for C++
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

#if __cplusplus >= 202302L && __has_include(<expected>)
#include <expected>

#if defined(__cpp_lib_expected)
namespace auxid {
namespace internal {
template <typename T, typename E> using Expected = std::expected<T, E>;

template <typename E> auto make_unexpected(E &&e) {
  return std::unexpected(std::forward<E>(e));
}
} // namespace internal
} // namespace auxid

#define AUXID_USE_STD_EXPECTED
#endif
#endif

#ifndef AUXID_USE_STD_EXPECTED
#include "tl/expected.hpp"

namespace auxid {
namespace internal {
template <typename T, typename E> using Expected = tl::expected<T, E>;

template <typename E> auto make_unexpected(E &&e) {
  return tl::make_unexpected(std::forward<E>(e));
}
} // namespace internal
} // namespace auxid
#endif

#include <array>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <iostream>
#include <memory>
#include <optional>
#include <source_location>
#include <span>
#include <string>
#include <vector>

#if !defined(__clang__) && !defined(__GNUC__)
#error                                                                         \
    "Auxid requires Clang or GCC for Statement Expressions ({...}). Use clang-cl on Windows."
#endif

#if __cplusplus < 202002L
#error "Auxid requires C++20 or newer."
#endif

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

// =============================================================================
// Template Types
// =============================================================================

template <typename T> using Mut = T;

template <typename T> using Ref = const T &;
template <typename T> using MutRef = T &;
template <typename T> using ForwardRef = T &&;

template <typename T>
[[nodiscard]] constexpr decltype(auto) mut(T &&arg) noexcept {
  return std::forward<T>(arg);
}

// =============================================================================
// Memory & Ownership
// =============================================================================

template <typename T> using Box = std::unique_ptr<T>;
template <typename T> using Arc = std::shared_ptr<T>;
template <typename T> using Weak = std::weak_ptr<T>;

template <typename T, typename... Args>
[[nodiscard]] inline auto make_box(ForwardRef<Args>... args) -> Box<T> {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
inline Box<T> make_box_protected(ForwardRef<Args>... args) {
  struct make_box_enabler : public T {
    make_box_enabler(ForwardRef<Args>... args)
        : T(std::forward<Args>(args)...) {}
  };

  return std::make_unique<make_box_enabler>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
[[nodiscard]] inline auto make_arc(ForwardRef<Args>... args) -> Arc<T> {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
inline Arc<T> make_arc_protected(ForwardRef<Args>... args) {
  struct make_arc_enabler : public T {
    make_arc_enabler(ForwardRef<Args>... args)
        : T(std::forward<Args>(args)...) {}
  };

  return std::make_shared<make_arc_enabler>(std::forward<Args>(args)...);
}

// =============================================================================
// Error Handling
// =============================================================================

template <typename T, typename E = std::string>
using Result = auxid::internal::Expected<T, E>;

template <typename E> [[nodiscard]] inline auto fail(ForwardRef<E> error) {
  return auxid::internal::make_unexpected(std::forward<E>(error));
}

template <typename... Args>
[[nodiscard]] inline auto fail(Ref<std::format_string<Args...>> fmt,
                               ForwardRef<Args>... args) {
  return auxid::internal::make_unexpected(
      std::format(fmt, std::forward<Args>(args)...));
}

// =============================================================================
// Utilities
// =============================================================================

namespace env {
#if defined(NDEBUG)
constexpr bool IS_DEBUG = false;
constexpr bool IS_RELEASE = true;
#else
constexpr bool IS_DEBUG = true;
constexpr bool IS_RELEASE = false;
#endif
} // namespace env

[[noreturn]] inline void
panic(Ref<std::string> msg,
      Ref<std::source_location> loc = std::source_location::current()) {
  std::cerr << "\n[panic] " << msg << "\n           At: " << loc.file_name()
            << ":" << loc.line() << "\n";
  std::abort();
}

inline void
ensure(bool condition, Ref<std::string> msg,
       Ref<std::source_location> loc = std::source_location::current()) {
  if (env::IS_DEBUG && !condition) {
    std::cerr << "\n[assert] " << msg << "\n            At: " << loc.file_name()
              << ":" << loc.line() << "\n";
    std::abort();
  }
}

using String = std::string;
using StringView = std::string_view;

template <typename T> using Option = std::optional<T>;
template <typename T> using Vec = std::vector<T>;
template <typename T> using Span = std::span<T>;
template <typename T1, typename T2> using Pair = std::pair<T1, T2>;
template <typename T, usize Count> using Array = std::array<T, Count>;

} // namespace auxid

#define AU_TRY_PURE(expr)                                                      \
  {                                                                            \
    auto _au_res = (expr);                                                     \
    if (!_au_res) {                                                            \
      return auxid::internal::make_unexpected(std::move(_au_res.error()));     \
    }                                                                          \
  }

#define AU_TRY(expr)                                                           \
  __extension__({                                                              \
    auto _au_res = (expr);                                                     \
    if (!_au_res) {                                                            \
      return auxid::internal::make_unexpected(std::move(_au_res.error()));     \
    }                                                                          \
    std::move(*_au_res);                                                       \
  })

#define AU_TRY_DISCARD(expr)                                                   \
  {                                                                            \
    auto _au_res = (expr);                                                     \
    if (!_au_res) {                                                            \
      return auxid::internal::make_unexpected(std::move(_au_res.error()));     \
    }                                                                          \
    AU_UNUSED(*_au_res);                                                       \
  }

#if !defined(AUXID_DONT_ALIAS_TO_AU)
namespace au = auxid;
#endif
