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
#include <auxid/containers/string.hpp>

using namespace au;

AUT_BEGIN_BLOCK(containers, string)

auto test_sso() -> bool
{
  String s("Orthodox");
  AUT_CHECK_EQ(s.size(), 8);
  AUT_CHECK_EQ(s, "Orthodox");
  return true;
}

auto test_heap_allocation() -> bool
{
  String s("This string is deliberately long to bypass the SSO capacity.");
  AUT_CHECK(s.size() > 23);
  AUT_CHECK_EQ(s.substr(0, 4), "This");
  return true;
}

auto test_append_and_concat() -> bool
{
  String s("Data");
  s += " Oriented";
  AUT_CHECK_EQ(s, "Data Oriented");

  String combined = s + String(" Design");
  AUT_CHECK_EQ(combined, "Data Oriented Design");
  return true;
}

auto test_push_pop() -> bool
{
  String s("C+");
  s.push_back('+');
  AUT_CHECK_EQ(s, "C++");
  s.pop_back();
  AUT_CHECK_EQ(s, "C+");
  return true;
}

AUT_BEGIN_TEST_LIST()
AUT_ADD_TEST(test_sso);
AUT_ADD_TEST(test_heap_allocation);
AUT_ADD_TEST(test_append_and_concat);
AUT_ADD_TEST(test_push_pop);
AUT_END_TEST_LIST()

AUT_END_BLOCK()

AUT_REGISTER_ENTRY(containers, string);