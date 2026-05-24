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

#include <utility>

import auxid;
import auxid.test;

using namespace au;

namespace
{
  struct ThreadBlock final : test::Block
  {
    [[nodiscard]] auto get_name() const -> const char * override
    {
      return "core::thread";
    }

    auto declare_tests() -> void override
    {
      add_test("thread_execution", [this] { return thread_execution(); });
    }

    auto thread_execution() -> bool
    {
      Mutex mtx;
      i32 shared_counter = 0;

      auto thread_res = Thread::create([&mtx, &shared_counter]() {
        LockGuard<Mutex> lock(mtx);
        shared_counter = 42;
      });
      if (!check(thread_res.is_ok(), "Thread::create returns Ok"))
        return false;

      Thread t = std::move(thread_res.unwrap());
      if (!check(t.joinable(), "thread is joinable"))
        return false;

      t.join();
      if (!check_not(t.joinable(), "thread is not joinable after join"))
        return false;

      LockGuard<Mutex> lock(mtx);
      return check_eq(shared_counter, 42, "shared_counter == 42");
    }
  };

  const test::AutoRegister<ThreadBlock> _registered;
} // namespace
