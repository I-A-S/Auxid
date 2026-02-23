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
#include <auxid/result.hpp>
#include <auxid/containers/string.hpp>

using namespace au;

AUT_BEGIN_BLOCK(core, result)

auto test_ok() -> bool
{
  Result<i32> res = 42;
  AUT_CHECK(res.is_ok());
  AUT_CHECK_NOT(res.is_err());
  AUT_CHECK_EQ(res.unwrap(), 42);

  return true;
}

auto test_err() -> bool
{
  Result<i32> res = fail("Memory allocation failed");
  AUT_CHECK(res.is_err());
  AUT_CHECK_NOT(res.is_ok());
  AUT_CHECK_EQ(res.unwrap_err(), "Memory allocation failed");

  return true;
}

auto test_void_result() -> bool
{
  ResultT<void, String> res;
  AUT_CHECK(res.is_ok());

  ResultT<void, String> err = fail("Void operation failed");
  AUT_CHECK(err.is_err());
  AUT_CHECK_EQ(err.unwrap_err(), "Void operation failed");

  return true;
}

AUT_BEGIN_TEST_LIST()
AUT_ADD_TEST(test_ok);
AUT_ADD_TEST(test_err);
AUT_ADD_TEST(test_void_result);
AUT_END_TEST_LIST()

AUT_END_BLOCK()

AUT_REGISTER_ENTRY(core, result);