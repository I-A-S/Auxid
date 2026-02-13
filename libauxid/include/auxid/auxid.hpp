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

#include <auxid/containers/hash_map.hpp>
#include <auxid/containers/hash_set.hpp>
#include <auxid/containers/option.hpp>
#include <auxid/containers/pair.hpp>
#include <auxid/containers/string.hpp>
#include <auxid/containers/vec.hpp>

namespace au
{
  using StringView = StringView;
  using String = containers::String;
  template<typename T> using Vec = containers::VecT<T, usize>;
  template<typename T> using TinyVec = containers::VecT<T, u16>;
  template<typename T> using CompactVec = containers::VecT<T, u32>;
  template<typename T> using Span = containers::Span<T>;
  template<typename T1, typename T2> using Pair = containers::Pair<T1, T2>;
  template<typename K, typename V> using HashMap = containers::HashMap<K, V>;
  template<typename T> using Option = containers::Option<T>;
  template<typename T> using HashSet = containers::HashSet<T>;

  template<typename T> using Result = ResultT<T, String>;

  template<typename... Args> [[noreturn]] inline void panic(const char *fmt, Args &&...args)
  {
    panic(String::format(fmt, std::forward(args)...));
  }

} // namespace au

namespace au
{
  // =============================================================================
  // Build Environment & Constants
  // =============================================================================
  namespace env
  {
#if defined(NDEBUG)
    constexpr bool IS_DEBUG = false;
    constexpr bool IS_RELEASE = true;
#else
    constexpr bool IS_DEBUG = true;
    constexpr bool IS_RELEASE = false;
#endif

#if IA_PLATFORM_WINDOWS
    constexpr const bool IS_WINDOWS = true;
    constexpr const bool IS_UNIX = false;
#else
    constexpr const bool IS_WINDOWS = false;
    constexpr const bool IS_UNIX = true;
#endif

    constexpr const usize MAX_PATH_LEN = 4096;

  } // namespace env

  template<typename... Args> [[nodiscard]] inline auto fail(const char *fmt, Args &&...args)
  {
    return fail(String::format(fmt, std::forward<Args>(args)...));
  }

  // =============================================================================
  // Versioning
  // =============================================================================
  struct Version
  {
    u32 major = 0;
    u32 minor = 0;
    u32 patch = 0;

    [[nodiscard]] constexpr auto to_u64() const -> u64
    {
      return (static_cast<u64>(major) << 40) | (static_cast<u64>(minor) << 16) | (static_cast<u64>(patch));
    }
  };

  // =============================================================================
  // Console Colors
  // =============================================================================
  namespace console
  {
    constexpr const char *RESET = "\033[0m";
    constexpr const char *RED = "\033[31m";
    constexpr const char *GREEN = "\033[32m";
    constexpr const char *YELLOW = "\033[33m";
    constexpr const char *BLUE = "\033[34m";
    constexpr const char *MAGENTA = "\033[35m";
    constexpr const char *CYAN = "\033[36m";
  } // namespace console

  // =============================================================================
  // Auxid API
  // =============================================================================
  namespace auxid
  {
    auto initialize_main_thread() -> void;
    auto terminate_main_thread() -> void;

    auto initialize_worker_thread() -> void;
    auto terminate_worker_thread() -> void;

    auto is_main_thread() -> bool;
    auto is_thread_initialized() -> bool;
  } // namespace auxid
} // namespace au
