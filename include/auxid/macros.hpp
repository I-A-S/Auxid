// Auxid: The Orthodox C++ Platform.
//
// Copyright (C) 2026 I-A-S (ias@iasoft.dev)
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

#if !defined(__clang__) && !defined(_MSC_VER)
#  error "Auxid requires Clang/Clang-CL or native MSVC."
#endif

#if defined(_MSC_VER)
#  if !defined(_MSVC_LANG) || _MSVC_LANG < 202302L
#    error "Auxid requires C++23 or newer."
#  endif
#else
#  if __cplusplus < 202302L
#    error "Auxid requires C++23 or newer."
#  endif
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
#  define AU_ARCH_X64 1
#elif defined(__aarch64__) || defined(_M_ARM64)
#  define AU_ARCH_ARM64 1
#elif defined(__wasm__) || defined(__wasm32__) || defined(__wasm64__)
#  define AU_ARCH_WASM 1
#else
#  if defined(_MSC_VER) && !defined(__clang__)
#    pragma message("Auxid: Unknown Architecture.")
#  else
#    warning "Auxid: Unknown Architecture."
#  endif
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  define AU_PLATFORM_WINDOWS 1
#elif __APPLE__
#  include <TargetConditionals.h>
#  define AU_PLATFORM_APPLE 1
#  define AU_PLATFORM_UNIX 1
#elif __linux__
#  define AU_PLATFORM_LINUX 1
#  define AU_PLATFORM_UNIX 1
#elif __wasm__
#  define AU_PLATFORM_WASM 1
#else
#  if defined(_MSC_VER) && !defined(__clang__)
#    pragma message("Auxid: Unknown Platform.")
#  else
#    warning "Auxid: Unknown Platform."
#  endif
#endif

#if !defined(AU_PLATFORM_WINDOWS)
#  define AU_PLATFORM_WINDOWS 0
#endif

#if defined(__clang__) || defined(__GNUC__)
#  define AUXID_ATTR_CONST __attribute__((const))
#  define AUXID_ATTR_PURE  __attribute__((pure))
#else
#  define AUXID_ATTR_CONST
#  define AUXID_ATTR_PURE
#endif

#undef pure_fn
#undef const_fn

// IANOTE: Intentionally swapped, this naming convention makes more sense.
#define pure_fn  AUXID_ATTR_CONST [[nodiscard]]
#define const_fn AUXID_ATTR_PURE  [[nodiscard]]

#define AU_LIKELY(v)   (v) [[likely]]
#define AU_UNLIKELY(v) (v) [[unlikely]]

#define AU_UNUSED(v) (void) (v)

#if defined(_MSC_VER)
#  define AUXID_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#  define AUXID_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#define AU_CONCAT_IMPL(x, y) x##y
#define AU_CONCAT(x, y)      AU_CONCAT_IMPL(x, y)
#define AU_UNIQUE_NAME(prefix) AU_CONCAT(prefix, __COUNTER__)

#define AU_ENSURE_CLASS_HAS_CONCEPT(cls, cpt)                                                                          \
  static_assert(cpt<cls>, "Class '" #cls "' must satisfy concept '" #cpt "'.")

#define AU_TRY_VAR_IMPL(name, expr, res_name)                                                                          \
  auto res_name = (expr);                                                                                              \
  if (res_name.is_err())                                                                                               \
  {                                                                                                                    \
    return au::fail(std::move(res_name.unwrap_err()));                                                                 \
  }                                                                                                                    \
  auto name = std::move(res_name.unwrap())

#define AU_TRY_VAR(name, expr) AU_TRY_VAR_IMPL(name, expr, AU_UNIQUE_NAME(_au_try_res_))

#define AU_TRY_IMPL(name, expr, res_name)                                                                          \
  auto res_name = (expr);                                                                                              \
  if (res_name.is_err())                                                                                               \
  {                                                                                                                    \
    return au::fail(std::move(res_name.unwrap_err()));                                                                 \
  }                                                                                                                    \
  name = std::move(res_name.unwrap())

#define AU_TRY(name, expr) AU_TRY_IMPL(name, expr, AU_UNIQUE_NAME(_au_try_res_))

#define AU_TRY_DISCARD_IMPL(expr, res_name)                                                                            \
  {                                                                                                                    \
    auto res_name = (expr);                                                                                            \
    if (res_name.is_err())                                                                                             \
    {                                                                                                                  \
      return au::fail(std::move(res_name.unwrap_err()));                                                               \
    }                                                                                                                  \
  }

#define AU_TRY_DISCARD(expr) AU_TRY_DISCARD_IMPL(expr, AU_UNIQUE_NAME(_au_try_discard_res_))
