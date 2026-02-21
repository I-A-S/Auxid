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

#include <auxid/containers/pair.hpp>
#include <auxid/containers/string.hpp>

namespace au::containers
{
  static constexpr u32 INDEX_INVALID = UINT32_MAX;

  template<typename T> struct Hash
  {
    u64 operator()(const T &val) const noexcept
    {
      const u8 *ptr = reinterpret_cast<const u8 *>(&val);
      u64 hash = 14695981039346656037ULL;
      for (usize i = 0; i < sizeof(T); ++i)
      {
        hash ^= ptr[i];
        hash *= 1099511628211ULL;
      }
      return hash;
    }
  };

  template<> struct Hash<i32>
  {
    u64 operator()(i32 x) const
    {
      auto t = static_cast<u64>(x) * 11400714819323198485ULL;
      t ^= (t >> 32);
      return t;
    }
  };

  template<> struct Hash<u32>
  {
    u64 operator()(u32 x) const
    {
      auto t = static_cast<u64>(x) * 11400714819323198485ULL;
      t ^= (t >> 32);
      return t;
    }
  };

  template<> struct Hash<u64>
  {
    u64 operator()(u64 x) const
    {
      auto t = static_cast<u64>(x) * 11400714819323198485ULL;
      t ^= (t >> 32);
      return t;
    }
  };

  template<> struct Hash<String>
  {
    u64 operator()(const String &s) const
    {
      return hash_string_view(s);
    }
  };

  template<> struct Hash<StringView>
  {
    u64 operator()(StringView s) const
    {
      return hash_string_view(s);
    }
  };

  template<> struct Hash<const char *>
  {
    u64 operator()(const char *s) const
    {
      return hash_string_view(StringView(s));
    }
  };

  template<typename T> struct EqualTo
  {
    constexpr bool operator()(const T &lhs, const T &rhs) const
    {
      return lhs == rhs;
    }
  };
} // namespace au::containers