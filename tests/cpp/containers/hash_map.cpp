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
#include <auxid/containers/hash_map.hpp>
#include <auxid/containers/string.hpp>

using namespace au;

AUT_BEGIN_BLOCK(containers, hash_map)

auto test_insert_and_find() -> bool
{
  HashMap<i32, String> map;

  AUT_CHECK(map.insert(1, "One"));
  AUT_CHECK(map.insert(2, "Two"));

  AUT_CHECK(map.contains(1));
  AUT_CHECK_EQ(*map.find(1), "One");

  AUT_CHECK_NOT(map.insert(1, "Duplicate"));
  AUT_CHECK_EQ(map.size(), 2);

  return true;
}

auto test_erase() -> bool
{
  HashMap<i32, i32> map;
  map.insert(10, 100);
  map.insert(20, 200);

  AUT_CHECK(map.erase(10));
  AUT_CHECK_NOT(map.contains(10));
  AUT_CHECK_EQ(map.size(), 1);

  AUT_CHECK_NOT(map.erase(999));

  return true;
}

auto test_operator_brackets() -> bool
{
  HashMap<String, i32> map;
  map["Score"] = 150;
  AUT_CHECK_EQ(map["Score"], 150);

  map["Score"] = 250;
  AUT_CHECK_EQ(map["Score"], 250);

  return true;
}

AUT_BEGIN_TEST_LIST()
AUT_ADD_TEST(test_insert_and_find);
AUT_ADD_TEST(test_erase);
AUT_ADD_TEST(test_operator_brackets);
AUT_END_TEST_LIST()

AUT_END_BLOCK()

AUT_REGISTER_ENTRY(containers, hash_map);