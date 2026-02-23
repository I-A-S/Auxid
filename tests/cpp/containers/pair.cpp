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
#include <auxid/containers/pair.hpp>
#include <auxid/containers/string.hpp>

using namespace au;

AUT_BEGIN_BLOCK(containers, pair)

auto test_pair_construction() -> bool
{
  Pair<i32, String> p(42, "Answer");

  AUT_CHECK_EQ(p.first, 42);
  AUT_CHECK_EQ(p.second, "Answer");

  return true;
}

auto test_pair_move_semantics() -> bool
{
  Pair<i32, String> p1(100, "Moving");
  Pair<i32, String> p2(std::move(p1));

  AUT_CHECK_EQ(p2.first, 100);
  AUT_CHECK_EQ(p2.second, "Moving");

  return true;
}

AUT_BEGIN_TEST_LIST()
AUT_ADD_TEST(test_pair_construction);
AUT_ADD_TEST(test_pair_move_semantics);
AUT_END_TEST_LIST()

AUT_END_BLOCK()

AUT_REGISTER_ENTRY(containers, pair);
