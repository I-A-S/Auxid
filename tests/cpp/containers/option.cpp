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
#include <auxid/containers/option.hpp>
#include <auxid/containers/string.hpp>

using namespace au;

AUT_BEGIN_BLOCK(containers, option)

auto test_some_and_none() -> bool
{
  Option<i32> opt1 = 42;
  AUT_CHECK(opt1.is_some());
  AUT_CHECK_NOT(opt1.is_none());
  AUT_CHECK_EQ(opt1.unwrap(), 42);

  Option<i32> opt2 = containers::nullopt;
  AUT_CHECK(opt2.is_none());
  AUT_CHECK_NOT(opt2.is_some());
  AUT_CHECK_EQ(opt2.value_or(10), 10);

  return true;
}

auto test_option_complex_type() -> bool
{
  Option<String> str_opt = String("Test");
  AUT_CHECK(str_opt.has_value());
  AUT_CHECK_EQ(str_opt.unwrap(), "Test");

  str_opt = containers::nullopt;
  AUT_CHECK(str_opt.is_none());

  return true;
}

AUT_BEGIN_TEST_LIST()
AUT_ADD_TEST(test_some_and_none);
AUT_ADD_TEST(test_option_complex_type);
AUT_END_TEST_LIST()

AUT_END_BLOCK()

AUT_REGISTER_ENTRY(containers, option);