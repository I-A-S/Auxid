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

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>

#if defined(_MSC_VER) && !defined(__clang__)
#  include <intrin.h>
#endif

#if defined(__clang__) || defined(__GNUC__)
#  define AUXID_ATTR_CONST __attribute__((const))
#  define AUXID_ATTR_PURE __attribute__((pure))
#else
#  define AUXID_ATTR_CONST
#  define AUXID_ATTR_PURE
#endif

namespace au
{
  namespace compiler
  {
    template<class T> constexpr T *addressof(T &arg) noexcept
    {
      return std::addressof(arg);
    }

    template<class T> const T *addressof(const T &&) = delete;

    [[noreturn]] inline void trap() noexcept
    {
#if defined(_MSC_VER) && !defined(__clang__)
      __debugbreak();
      std::abort();
#elif defined(__has_builtin)
#  if __has_builtin(__builtin_trap)
      __builtin_trap();
#  else
      std::abort();
#  endif
#else
      std::abort();
#endif
    }

    inline int memcmp(const void *lhs, const void *rhs, std::size_t n) noexcept
    {
      return std::memcmp(lhs, rhs, n);
    }

    inline std::size_t strlen(const char *s) noexcept
    {
      return std::strlen(s);
    }

    inline const void *memchr(const void *p, int c, std::size_t n) noexcept
    {
      return std::memchr(p, c, n);
    }
  } // namespace compiler
} // namespace au
