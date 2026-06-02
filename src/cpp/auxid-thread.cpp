// Auxid: The Rigid C++ Platform.
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

module;

#include <auxid/macros.hpp>

#include <cstdio>
#include <cstdlib>
#include <format>
#include <print>

#if !defined(AUXID_USE_SYSTEM_MALLOC)
#  include <rpmalloc/rpmalloc.h>
#endif

module auxid.thread;

import auxid.memory;

namespace au::auxid
{
  struct ThreadData
  {
    i32 init_counter{};
    memory::Box<Logger> logger;
  };

#if !defined(AUXID_USE_SYSTEM_MALLOC)
  struct RpmallocLifetime
  {
    RpmallocLifetime() noexcept
    {
      rpmalloc_initialize(nullptr);
    }

    ~RpmallocLifetime()
    {
      rpmalloc_finalize();
    }

    RpmallocLifetime(const RpmallocLifetime &) = delete;
    RpmallocLifetime &operator=(const RpmallocLifetime &) = delete;
  };
#endif

  struct State
  {
#if !defined(AUXID_USE_SYSTEM_MALLOC)
    RpmallocLifetime rpmalloc_lifetime{};
#endif
    Mutex logger_mutex{};
    Thread::ThreadID main_thread_id{};
    HashMap<Thread::ThreadID, ThreadData> thread_data{};
  };

  static auto get_state() -> State &
  {
    static State s_state{};
    return s_state;
  }

  AUXID_API auto initialize_main_thread() -> void
  {
    auto &state = get_state();

    const auto thread_id = Thread::get_calling_thread_id();
    state.thread_data[thread_id].init_counter++;
    if (state.thread_data[thread_id].init_counter > 1)
      return;

    state.main_thread_id = thread_id;
    state.thread_data[thread_id].logger = memory::make_box<Logger>(state.logger_mutex);
  }

  AUXID_API auto terminate_main_thread() -> void
  {
    auto &state = get_state();

    const auto thread_id = Thread::get_calling_thread_id();
    state.thread_data[thread_id].init_counter--;
    if (state.thread_data[thread_id].init_counter > 0)
      return;

    state.thread_data[thread_id].logger.reset();
  }

  AUXID_API auto initialize_worker_thread() -> void
  {
    auto &state = get_state();

    const auto thread_id = Thread::get_calling_thread_id();
    state.thread_data[thread_id].init_counter++;
    if (state.thread_data[thread_id].init_counter > 1)
      return;

    state.thread_data[thread_id].logger = memory::make_box<Logger>(state.logger_mutex);

#if !defined(AUXID_USE_SYSTEM_MALLOC)
    rpmalloc_thread_initialize();
#endif
  }

  AUXID_API auto terminate_worker_thread() -> void
  {
    auto &state = get_state();

    const auto thread_id = Thread::get_calling_thread_id();
    state.thread_data[thread_id].init_counter--;
    if (state.thread_data[thread_id].init_counter > 0)
      return;

    state.thread_data[thread_id].logger.reset();

#if !defined(AUXID_USE_SYSTEM_MALLOC)
    rpmalloc_thread_finalize();
#endif
  }

  AUXID_API auto is_main_thread() -> bool
  {
    return get_state().main_thread_id == Thread::get_calling_thread_id();
  }

  AUXID_API auto is_thread_initialized() -> bool
  {
    return get_state().thread_data[Thread::get_calling_thread_id()].init_counter > 0;
  }

  AUXID_API auto get_thread_logger() -> Logger &
  {
    return *get_state().thread_data[Thread::get_calling_thread_id()].logger;
  }
} // namespace au::auxid

namespace au
{
#define CC_RESET "\033[0m"
#define CC_RED "\033[31m"
#define CC_GREEN "\033[32m"
#define CC_YELLOW "\033[33m"
#define CC_BLUE "\033[34m"
#define CC_MAGENTA "\033[35m"
#define CC_CYAN "\033[36m"

  AUXID_API Logger::Logger(Mutex &logger_mutex) : m_logger_mutex_ref(logger_mutex)
  {
  }

  AUXID_API auto Logger::log_impl(ELevel level, StringView fmt, std::format_args args) -> void
  {
    const auto msg = String::vformat(fmt, args);
    m_logger_mutex_ref.lock();
    m_handler(msg.c_str(), level);
    m_logger_mutex_ref.unlock();
  }

  AUXID_API auto Logger::default_handler(const char *msg, ELevel level) -> void
  {
    switch (level)
    {
    case LEVEL_TRACE:
      std::println(stdout, CC_RESET "[TRCE]: {}" CC_RESET, msg);
      break;
    case LEVEL_DEBUG:
      std::println(stdout, CC_CYAN "[DBUG]: {}" CC_RESET, msg);
      break;
    case LEVEL_INFO:
      std::println(stdout, CC_GREEN "[INFO]: {}" CC_RESET, msg);
      break;
    case LEVEL_WARN:
      std::println(stdout, CC_YELLOW "[WARN]: {}" CC_RESET, msg);
      break;
    case LEVEL_ERROR:
      std::println(stdout, CC_RED "[EROR]: {}" CC_RESET, msg);
      break;
    }
  }
} // namespace au
