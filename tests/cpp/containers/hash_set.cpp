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
#include <auxid/containers/hash_set.hpp>
#include <auxid/containers/string.hpp>

using namespace au;

AUT_BEGIN_BLOCK(containers, hash_set)

auto test_insert_and_contains() -> bool
{
  HashSet<String> set;

  AUT_CHECK(set.insert("Core"));
  AUT_CHECK(set.insert("Renderer"));

  AUT_CHECK(set.contains("Core"));
  AUT_CHECK(set.contains("Renderer"));
  AUT_CHECK_NOT(set.contains("Physics"));

  AUT_CHECK_NOT(set.insert("Core"));
  AUT_CHECK_EQ(set.size(), 2);

  return true;
}

auto test_erase_and_clear() -> bool
{
  HashSet<i32> set;
  set.insert(10);
  set.insert(20);
  set.insert(30);

  AUT_CHECK(set.erase(20));
  AUT_CHECK_NOT(set.contains(20));
  AUT_CHECK_EQ(set.size(), 2);

  AUT_CHECK_NOT(set.erase(999));

  set.clear();
  AUT_CHECK(set.empty());
  AUT_CHECK_EQ(set.size(), 0);

  return true;
}

AUT_BEGIN_TEST_LIST()
AUT_ADD_TEST(test_insert_and_contains);
AUT_ADD_TEST(test_erase_and_clear);
AUT_END_TEST_LIST()

AUT_END_BLOCK()

AUT_REGISTER_ENTRY(containers, hash_set);
