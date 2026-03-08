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

#include <auxid/result.hpp>
#include <auxid/thread/mutex.hpp>

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
  // Logger API
  //
  // Each thread gets its own logger instance. Call `auxid::get_thread_logger()` to
  // access the calling threads logger instance.
  // =============================================================================
  class Logger
  {
public:
    enum ELevel
    {
      LEVEL_TRACE,
      LEVEL_DEBUG,
      LEVEL_INFO,
      LEVEL_WARN,
      LEVEL_ERROR
    };

    typedef void (*LogHandler_FuncT)(const char *msg, ELevel level);

public:
    auto trace(const char *fmt, ...) -> void;
    auto debug(const char *fmt, ...) -> void;
    auto info(const char *fmt, ...) -> void;
    auto warn(const char *fmt, ...) -> void;
    auto error(const char *fmt, ...) -> void;

public:
    Logger(Mutex &logger_mutex);

    // You may set a custom log handler
    // You can safely access shared resources (such as stdout)
    // inside the handler, synchronization is handled automatically
    // for you by auxid.
    auto set_log_handler(LogHandler_FuncT handler) -> void
    {
      m_handler = handler;
    }

private:
    static auto default_handler(const char *msg, ELevel level) -> void;

    Mutex &m_logger_mutex_ref;
    LogHandler_FuncT m_handler{default_handler};
  };

  // =============================================================================
  // Auxid API
  // =============================================================================
  namespace auxid

  {
    // ========================================================
    // Thread Lifetime Management
    //
    // Instead of calling these functions directly, consider
    // utilizing `MainThreadGuard` and `WorkerThreadGuard`,
    // unless you have an explicit reason not to.
    // ========================================================

    auto initialize_main_thread() -> void;
    auto terminate_main_thread() -> void;
    // Must only be called on all *manually* spawned threads.
    // If you're using Auxid Threads (au::ThreadT, au::NThread, au::JThread),
    // this is handled automatically for you.
    auto initialize_worker_thread() -> void;
    auto terminate_worker_thread() -> void;

    struct MainThreadGuard
    {
      MainThreadGuard()
      {
        initialize_main_thread();
      }

      ~MainThreadGuard()
      {
        terminate_main_thread();
      }
    };

    struct WorkerThreadGuard
    {
      WorkerThreadGuard()
      {
        initialize_worker_thread();
      }

      ~WorkerThreadGuard()
      {
        terminate_worker_thread();
      }
    };

    // ========================================================

    auto is_main_thread() -> bool;
    auto is_thread_initialized() -> bool;

    auto get_thread_logger() -> Logger &;
  } // namespace auxid
} // namespace au
