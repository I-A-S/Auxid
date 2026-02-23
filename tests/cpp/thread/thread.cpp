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

#include <auxid/utils/test.hpp>
#include <auxid/thread/thread.hpp>
#include <auxid/thread/mutex.hpp>

using namespace au;

AUT_BEGIN_BLOCK(core, thread)

auto test_thread_execution() -> bool
{
  Mutex mtx;
  i32 shared_counter = 0;

  auto thread_res = Thread::create([&mtx, &shared_counter]() {
    LockGuard<Mutex> lock(mtx);
    shared_counter = 42;
  });

  AUT_CHECK(thread_res.is_ok());

  Thread t = std::move(thread_res.unwrap());
  AUT_CHECK(t.joinable());

  t.join();
  AUT_CHECK_NOT(t.joinable());

  LockGuard<Mutex> lock(mtx);
  AUT_CHECK_EQ(shared_counter, 42);

  return true;
}

AUT_BEGIN_TEST_LIST()
AUT_ADD_TEST(test_thread_execution);
AUT_END_TEST_LIST()

AUT_END_BLOCK()

AUT_REGISTER_ENTRY(core, thread);