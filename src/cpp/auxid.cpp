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

module;

#include <cstdlib>
#include <iostream>
#include <print>

#if !defined(AUXID_USE_SYSTEM_MALLOC)
#  include <auxid/vendor/rpmalloc/rpmalloc.h>
#endif

module auxid.core;

import auxid.thread;
import auxid.containers;

namespace au::auxid
{
  struct ThreadData
  {
    i32 init_counter{};
    Logger *logger;
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
    Mut<Thread::ThreadID> main_thread_id{};
    Mut<HashMap<Thread::ThreadID, ThreadData>> thread_data{};
  };

  static auto get_state() -> State &
  {
    static Mut<State> s_state{};
    return s_state;
  }

  auto initialize_main_thread() -> void
  {
    auto &state = get_state();

    const auto thread_id = Thread::get_calling_thread_id();
    state.thread_data[thread_id].init_counter++;
    if (state.thread_data[thread_id].init_counter > 1)
      return;

    state.main_thread_id = thread_id;
    state.thread_data[thread_id].logger = new Logger(state.logger_mutex);
  }

  auto terminate_main_thread() -> void
  {
    auto &state = get_state();

    const auto thread_id = Thread::get_calling_thread_id();
    state.thread_data[thread_id].init_counter--;
    if (state.thread_data[thread_id].init_counter > 0)
      return;

    delete state.thread_data[thread_id].logger;
    state.thread_data[thread_id].logger = nullptr;
  }

  auto initialize_worker_thread() -> void
  {
    auto &state = get_state();

    const auto thread_id = Thread::get_calling_thread_id();
    state.thread_data[thread_id].init_counter++;
    if (state.thread_data[thread_id].init_counter > 1)
      return;

    state.thread_data[thread_id].logger = new Logger(state.logger_mutex);

#if !defined(AUXID_USE_SYSTEM_MALLOC)
    rpmalloc_thread_initialize();
#endif
  }

  auto terminate_worker_thread() -> void
  {
    auto &state = get_state();

    const auto thread_id = Thread::get_calling_thread_id();
    state.thread_data[thread_id].init_counter--;
    if (state.thread_data[thread_id].init_counter > 0)
      return;

    delete state.thread_data[thread_id].logger;
    state.thread_data[thread_id].logger = nullptr;

#if !defined(AUXID_USE_SYSTEM_MALLOC)
    rpmalloc_thread_finalize();
#endif
  }

  auto is_main_thread() -> bool
  {
    return get_state().main_thread_id == Thread::get_calling_thread_id();
  }

  auto is_thread_initialized() -> bool
  {
    return get_state().thread_data[Thread::get_calling_thread_id()].init_counter > 0;
  }

  auto get_thread_logger() -> Logger &
  {
    return *get_state().thread_data[Thread::get_calling_thread_id()].logger;
  }
} // namespace au::auxid

namespace au
{
#if !defined(AUXID_DISABLE_DEFAULT_PANIC_HANDLER)
  auto panic_handler(const char *msg, const char *file, u32 line) -> void
  {
    std::println(std::cerr, "[PANIC]: ({}:{}): {}", file, line, msg);
    std::abort();
  }
#endif
} // namespace au
