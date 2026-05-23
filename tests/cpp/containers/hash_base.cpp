// Auxid: The Orthodox C++ Platform.
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

#include <concepts>
#include <tuple>
#include <type_traits>

import auxid;
import auxid.test;

using namespace au;
using namespace au::containers;

template<typename T>
concept Hashable = requires(const T &v) {
  { Hash<T>{}(v) } -> std::convertible_to<u64>;
};

struct NoPadding
{
  u32 a;
  u32 b;
};

struct PaddedStruct
{
  u8 a;
  u32 b;
};

static_assert(Hashable<u32>, "u32 must be hashable via the integer mixer");
static_assert(Hashable<u64>, "u64 must be hashable via the integer mixer");
static_assert(Hashable<bool>, "bool must be hashable via the integer mixer");
static_assert(Hashable<char>, "char must be hashable via the integer mixer");
static_assert(Hashable<i32 *>, "Pointers must be hashable via the integer mixer");
static_assert(Hashable<String>, "String must be hashable via wyhash");
static_assert(Hashable<StringView>, "StringView must be hashable via wyhash");
static_assert(Hashable<NoPadding>, "Padding-free structs must hash via the generic wyhash fallback");

static_assert(!Hashable<PaddedStruct>,
              "Structs with padding MUST NOT satisfy the default Hash<T>; "
              "users must specialize Hash explicitly to avoid hashing "
              "uninitialised padding bytes.");

namespace
{
  struct HashBaseBlock final : test::Block
  {
    [[nodiscard]] auto get_name() const -> const char * override { return "containers::hash_base"; }

    auto declare_tests() -> void override
    {
      add_test("int_mixer_disperses_sequential_inputs", [this] { return int_mixer_disperses_sequential_inputs(); });
      add_test("int_mixer_avalanche",                   [this] { return int_mixer_avalanche(); });
      add_test("string_hash_matches_stringview_hash",   [this] { return string_hash_matches_stringview_hash(); });
      add_test("string_hash_changes_with_content",      [this] { return string_hash_changes_with_content(); });
      add_test("string_hash_handles_empty",             [this] { return string_hash_handles_empty(); });
      add_test("string_hash_long_input",                [this] { return string_hash_long_input(); });
      add_test("struct_hash_works_for_padding_free",    [this] { return struct_hash_works_for_padding_free(); });
      add_test("hash_combine_disperses",                [this] { return hash_combine_disperses(); });
      add_test("pair_hash_distinguishes_field_order",   [this] { return pair_hash_distinguishes_field_order(); });
      add_test("pair_hash_string_value",                [this] { return pair_hash_string_value(); });
      add_test("tuple_hash_basic",                      [this] { return tuple_hash_basic(); });
      add_test("tuple_hash_empty_is_zero",              [this] { return tuple_hash_empty_is_zero(); });
      add_test("span_bytes_hash_matches_hash_bytes",    [this] { return span_bytes_hash_matches_hash_bytes(); });
      add_test("hash_combine_range_basic",              [this] { return hash_combine_range_basic(); });
    }

    auto int_mixer_disperses_sequential_inputs() -> bool
    {
      Hash<u32> h;
      u64 a = h(0), b = h(1), c = h(2);
      return check_neq(a, b, "h(0) != h(1)")
          && check_neq(b, c, "h(1) != h(2)")
          && check_neq(a, c, "h(0) != h(2)");
    }

    auto int_mixer_avalanche() -> bool
    {
      Hash<u64> h;
      u64 x = 0x0123456789ABCDEFULL;
      u64 h1 = h(x);
      u64 h2 = h(x ^ 1ULL);
      u32 differing = __builtin_popcountll(h1 ^ h2);
      return check(differing >= 8, "popcount(h1 ^ h2) >= 8");
    }

    auto string_hash_matches_stringview_hash() -> bool
    {
      Hash<String> hs;
      Hash<StringView> hv;

      String owned = "the quick brown fox";
      StringView view = "the quick brown fox";

      return check_eq(hs(StringView(owned)), hv(view), "Hash<String>(view) == Hash<StringView>(view)");
    }

    auto string_hash_changes_with_content() -> bool
    {
      Hash<StringView> h;
      return check_neq(h(StringView("foo")), h(StringView("bar")),  "h(\"foo\") != h(\"bar\")")
          && check_neq(h(StringView("foo")), h(StringView("food")), "h(\"foo\") != h(\"food\")")
          && check_neq(h(StringView("foo")), h(StringView("oof")),  "h(\"foo\") != h(\"oof\")");
    }

    auto string_hash_handles_empty() -> bool
    {
      Hash<StringView> h;
      return check_neq(h(StringView("")), h(StringView("a")), "h(\"\") != h(\"a\")");
    }

    auto string_hash_long_input() -> bool
    {
      String s;
      for (int i = 0; i < 256; ++i)
        s.push_back(static_cast<char>('a' + (i % 26)));
      Hash<StringView> h;
      u64 h1 = h(StringView(s));
      String s2 = s;
      s2.push_back('!');
      u64 h2 = h(StringView(s2));
      return check_neq(h1, h2, "h(s) != h(s + '!')");
    }

    auto struct_hash_works_for_padding_free() -> bool
    {
      Hash<NoPadding> h;
      NoPadding x{1, 2}, y{1, 2}, z{2, 1};
      return check_eq(h(x), h(y), "h({1,2}) == h({1,2})")
          && check_neq(h(x), h(z), "h({1,2}) != h({2,1})");
    }

    auto hash_combine_disperses() -> bool
    {
      u64 seed1 = hash_combine(42, 7);
      u64 seed2 = hash_combine(7, 42);
      return check_neq(seed1, seed2, "hash_combine is non-commutative");
    }

    auto pair_hash_distinguishes_field_order() -> bool
    {
      Hash<au::Pair<u32, u32>> h;
      au::Pair<u32, u32> a{1, 2};
      au::Pair<u32, u32> b{2, 1};
      au::Pair<u32, u32> c{1, 2};
      return check_eq(h(a), h(c), "h({1,2}) == h({1,2})")
          && check_neq(h(a), h(b), "h({1,2}) != h({2,1})");
    }

    auto pair_hash_string_value() -> bool
    {
      Hash<au::Pair<String, u64>> h;
      au::Pair<String, u64> a{String("hello"), 42};
      au::Pair<String, u64> b{String("hello"), 42};
      au::Pair<String, u64> c{String("hello"), 43};
      return check_eq(h(a), h(b), "h({\"hello\",42}) equality")
          && check_neq(h(a), h(c), "h(...,42) != h(...,43)");
    }

    auto tuple_hash_basic() -> bool
    {
      Hash<std::tuple<u32, u32, u32>> h;
      std::tuple<u32, u32, u32> a{1, 2, 3};
      std::tuple<u32, u32, u32> b{3, 2, 1};
      std::tuple<u32, u32, u32> c{1, 2, 3};
      return check_eq(h(a), h(c), "tuple hash equality")
          && check_neq(h(a), h(b), "tuple hash field order matters");
    }

    auto tuple_hash_empty_is_zero() -> bool
    {
      Hash<std::tuple<>> h;
      return check_eq(h(std::tuple<>{}), static_cast<u64>(0), "h(empty tuple) == 0");
    }

    auto span_bytes_hash_matches_hash_bytes() -> bool
    {
      const u8 buf[5] = {1, 2, 3, 4, 5};
      Hash<au::Span<const u8>> h;
      u64 via_span = h(au::Span<const u8>(buf, 5));
      u64 via_raw  = hash_bytes(buf, 5);
      return check_eq(via_span, via_raw, "Hash<Span<const u8>> matches hash_bytes()");
    }

    auto hash_combine_range_basic() -> bool
    {
      u32 forward[3] = {10, 20, 30};
      u32 reverse[3] = {30, 20, 10};
      u64 a = hash_combine_range(0, forward, forward + 3);
      u64 b = hash_combine_range(0, reverse, reverse + 3);
      u64 c = hash_combine_range(0, forward, forward + 3);
      return check_eq(a, c, "hash_combine_range is deterministic")
          && check_neq(a, b, "hash_combine_range is order-sensitive");
    }
  };

  const test::AutoRegister<HashBaseBlock> _registered;
} // namespace
