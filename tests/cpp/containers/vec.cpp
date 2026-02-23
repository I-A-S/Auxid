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
#include <auxid/containers/vec.hpp>
#include <auxid/containers/string.hpp>

using namespace au;

AUT_BEGIN_BLOCK(containers, vec)

auto test_push_and_pop() -> bool
{
  Vec<i32> v;
  v.push_back(10);
  v.push_back(20);

  AUT_CHECK_EQ(v.size(), 2);
  AUT_CHECK_EQ(v[0], 10);
  AUT_CHECK_EQ(v[1], 20);

  v.pop_back();
  AUT_CHECK_EQ(v.size(), 1);
  AUT_CHECK_EQ(v.back(), 10);

  return true;
}

auto test_reserve_and_capacity() -> bool
{
  Vec<i32> v;
  v.reserve(100);

  AUT_CHECK(v.capacity() >= 100);
  AUT_CHECK_EQ(v.size(), 0);
  AUT_CHECK(v.empty());

  return true;
}

auto test_initializer_list() -> bool
{
  Vec<String> v = {"Orthodox", "C++", "Auxid"};

  AUT_CHECK_EQ(v.size(), 3);
  AUT_CHECK_EQ(v[0], "Orthodox");
  AUT_CHECK_EQ(v[1], "C++");
  AUT_CHECK_EQ(v[2], "Auxid");

  return true;
}

auto test_clear() -> bool
{
  Vec<i32> v = {1, 2, 3, 4, 5};
  v.clear();

  AUT_CHECK(v.empty());
  AUT_CHECK_EQ(v.size(), 0);
  AUT_CHECK(v.capacity() >= 5);

  return true;
}

AUT_BEGIN_TEST_LIST()
AUT_ADD_TEST(test_push_and_pop);
AUT_ADD_TEST(test_reserve_and_capacity);
AUT_ADD_TEST(test_initializer_list);
AUT_ADD_TEST(test_clear);
AUT_END_TEST_LIST()

AUT_END_BLOCK()

AUT_REGISTER_ENTRY(containers, vec);
