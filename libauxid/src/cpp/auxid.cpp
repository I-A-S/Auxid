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

#include <auxid/auxid.hpp>

#include <auxid/thread/thread.hpp>
#include <auxid/containers/hash_map.hpp>

#if !defined(AUXID_USE_SYSTEM_MALLOC)
#  include <auxid/vendor/rpmalloc/rpmalloc.h>
#endif

namespace au::auxid
{
  struct State
  {
    Mut<Thread::ThreadID> main_thread_id{};
    Mut<HashMap<Thread::ThreadID, i32>> thread_init_counter{};
  };

  auto get_state() -> State &
  {
    static Mut<State> s_state{};
    return s_state;
  }

  auto initialize_main_thread() -> void
  {
    auto &state = get_state();

    const auto thread_id = Thread::get_calling_thread_id();
    state.thread_init_counter[thread_id]++;
    if (state.thread_init_counter[thread_id] > 1)
      return;

    state.main_thread_id = thread_id;

#if !defined(AUXID_USE_SYSTEM_MALLOC)
    rpmalloc_initialize(nullptr);
#endif
  }

  auto terminate_main_thread() -> void
  {
    auto &state = get_state();

    const auto thread_id = Thread::get_calling_thread_id();
    state.thread_init_counter[thread_id]--;
    if (state.thread_init_counter[thread_id] > 0)
      return;

#if !defined(AUXID_USE_SYSTEM_MALLOC)
    rpmalloc_finalize();
#endif
  }

  auto initialize_worker_thread() -> void
  {
    auto &state = get_state();

    const auto thread_id = Thread::get_calling_thread_id();
    state.thread_init_counter[thread_id]++;
    if (state.thread_init_counter[thread_id] > 1)
      return;

#if !defined(AUXID_USE_SYSTEM_MALLOC)
    rpmalloc_thread_initialize();
#endif
  }

  auto terminate_worker_thread() -> void
  {
    auto &state = get_state();

    const auto thread_id = Thread::get_calling_thread_id();
    state.thread_init_counter[thread_id]--;
    if (state.thread_init_counter[thread_id] > 0)
      return;

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
    return get_state().thread_init_counter[Thread::get_calling_thread_id()] > 0;
  }
} // namespace au::auxid

namespace au
{
#if !defined(AUXID_DISABLE_DEFAULT_PANIC_HANDLER)
  auto panic_handler(const char *msg, const char *file, u32 line) -> void
  {
    // Default Panic Handler (Simply prints to stdout and hangs)
    printf("[PANIC]: (%s:%u): %s\n", file, line, msg);
    while (true)
    {
    }
  }
#endif
} // namespace au