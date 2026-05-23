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

#include <expected>
#include <utility>

import auxid;
import auxid.test;

using namespace au;

namespace
{
  struct ResultBlock final : test::Block
  {
    [[nodiscard]] auto get_name() const -> const char * override { return "core::result"; }

    auto declare_tests() -> void override
    {
      add_test("ok",                 [this] { return ok(); });
      add_test("err",                [this] { return err(); });
      add_test("void_result",        [this] { return void_result(); });
      add_test("std_expected_ok",    [this] { return std_expected_ok(); });
      add_test("std_expected_err",   [this] { return std_expected_err(); });
      add_test("std_expected_void",  [this] { return std_expected_void(); });
    }

    auto ok() -> bool
    {
      Result<i32> res = 42;
      return check(res.is_ok(), "res.is_ok()")
          && check_not(res.is_err(), "!res.is_err()")
          && check_eq(res.unwrap(), 42, "res.unwrap() == 42");
    }

    auto err() -> bool
    {
      Result<i32> res = fail("Memory allocation failed");
      return check(res.is_err(), "res.is_err()")
          && check_not(res.is_ok(), "!res.is_ok()")
          && check_eq(res.unwrap_err(), "Memory allocation failed", "unwrap_err matches");
    }

    auto void_result() -> bool
    {
      ResultT<void, String> res;
      if (!check(res.is_ok(), "default ResultT<void> is_ok"))
        return false;

      ResultT<void, String> errr = fail("Void operation failed");
      return check(errr.is_err(), "fail-constructed is_err")
          && check_eq(errr.unwrap_err(), "Void operation failed", "unwrap_err matches");
    }

    auto std_expected_ok() -> bool
    {
      std::expected<i32, String> from_std{42};
      Result<i32> ours = from_std;
      if (!check(ours.is_ok(), "ours.is_ok()"))                  return false;
      if (!check_eq(ours.unwrap(), 42, "ours.unwrap() == 42"))   return false;

      std::expected<i32, String> back_to_std = ours;
      return check(back_to_std.has_value(), "back_to_std.has_value()")
          && check_eq(*back_to_std, 42, "*back_to_std == 42");
    }

    auto std_expected_err() -> bool
    {
      std::expected<i32, String> err_from_std{std::unexpect, String("boom")};
      Result<i32> ours = err_from_std;
      if (!check(ours.is_err(), "ours.is_err()"))                                return false;
      if (!check_eq(ours.unwrap_err(), "boom", "ours.unwrap_err() == \"boom\"")) return false;

      std::expected<i32, String> back_to_std = ours;
      return check_not(back_to_std.has_value(), "!back_to_std.has_value()")
          && check_eq(back_to_std.error(), "boom", "back_to_std.error() == \"boom\"");
    }

    auto std_expected_void() -> bool
    {
      std::expected<void, String> ok_std;
      ResultT<void, String> ours_ok = ok_std;
      if (!check(ours_ok.is_ok(), "ours_ok.is_ok()")) return false;

      std::expected<void, String> err_std{std::unexpect, String("void-fail")};
      ResultT<void, String> ours_err = std::move(err_std);
      if (!check(ours_err.is_err(), "ours_err.is_err()"))                              return false;
      if (!check_eq(ours_err.unwrap_err(), "void-fail", "unwrap_err matches"))         return false;

      std::expected<void, String> back = ours_err;
      return check_not(back.has_value(), "!back.has_value()")
          && check_eq(back.error(), "void-fail", "back.error() matches");
    }
  };

  const test::AutoRegister<ResultBlock> _registered;
} // namespace
