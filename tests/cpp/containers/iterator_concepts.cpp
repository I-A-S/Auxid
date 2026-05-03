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
#include <auxid/containers/hash_set.hpp>
#include <auxid/containers/span.hpp>

#include <algorithm>
#include <iterator>
#include <ranges>

using namespace au;

static_assert(std::contiguous_iterator<Vec<i32>::iterator>);
static_assert(std::contiguous_iterator<Vec<i32>::const_iterator>);
static_assert(std::ranges::contiguous_range<Vec<i32>>);

static_assert(std::contiguous_iterator<String::iterator>);
static_assert(std::ranges::contiguous_range<String>);

static_assert(std::contiguous_iterator<StringView::iterator>);
static_assert(std::ranges::contiguous_range<StringView>);
static_assert(std::ranges::borrowed_range<StringView>);

static_assert(std::contiguous_iterator<Span<const i32>::iterator>);
static_assert(std::ranges::contiguous_range<Span<const i32>>);

static_assert(std::contiguous_iterator<HashMap<i32, i32>::iterator>);
static_assert(std::ranges::contiguous_range<HashMap<i32, i32>>);

static_assert(std::contiguous_iterator<HashSet<i32>::iterator>);
static_assert(std::ranges::contiguous_range<HashSet<i32>>);

AUT_BEGIN_BLOCK(containers, iterator_concepts)

auto test_std_sort_on_vec() -> bool
{
  Vec<i32> v;
  v.push_back(3);
  v.push_back(1);
  v.push_back(2);
  std::ranges::sort(v);
  AUT_CHECK_EQ(v[0], 1);
  AUT_CHECK_EQ(v[1], 2);
  AUT_CHECK_EQ(v[2], 3);
  return true;
}

auto test_std_sort_on_string() -> bool
{
  String s = "cba";
  std::ranges::sort(s);
  AUT_CHECK_EQ(s, StringView("abc"));
  return true;
}

auto test_stringview_range_for() -> bool
{
  StringView sv = "xy";
  usize sum = 0;
  for (char c : sv)
  {
    sum += static_cast<usize>(c);
  }
  AUT_CHECK_EQ(sum, static_cast<usize>('x') + static_cast<usize>('y'));
  return true;
}

auto test_hash_map_iteration() -> bool
{
  HashMap<i32, i32> m;
  m.insert(1, 10);
  m.insert(2, 20);
  usize count = 0;
  for (auto &p : m)
  {
    (void) p;
    ++count;
  }
  AUT_CHECK_EQ(count, m.size());
  return true;
}

AUT_BEGIN_TEST_LIST()
AUT_ADD_TEST(test_std_sort_on_vec);
AUT_ADD_TEST(test_std_sort_on_string);
AUT_ADD_TEST(test_stringview_range_for);
AUT_ADD_TEST(test_hash_map_iteration);
AUT_END_TEST_LIST()

AUT_END_BLOCK()

AUT_REGISTER_ENTRY(containers, iterator_concepts);
