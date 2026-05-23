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

import auxid;
import auxid.test;

using namespace au;

namespace
{
  struct HashMapBlock final : test::Block
  {
    [[nodiscard]] auto get_name() const -> const char * override { return "containers::hash_map"; }

    auto declare_tests() -> void override
    {
      add_test("insert_and_find",    [this] { return insert_and_find(); });
      add_test("erase",              [this] { return erase_(); });
      add_test("operator_brackets",  [this] { return operator_brackets(); });
      add_test("transparent_lookup", [this] { return transparent_lookup(); });
    }

    auto insert_and_find() -> bool
    {
      HashMap<i32, String> map;
      if (!check(map.insert(1, "One"), "insert(1, \"One\")")) return false;
      if (!check(map.insert(2, "Two"), "insert(2, \"Two\")")) return false;

      if (!check(map.contains(1), "map.contains(1)"))                            return false;
      if (!check_eq(*map.find(1), "One", "*map.find(1) == \"One\""))             return false;

      if (!check_not(map.insert(1, "Duplicate"), "duplicate insert returns false")) return false;
      return check_eq(map.size(), 2u, "map.size() == 2");
    }

    auto erase_() -> bool
    {
      HashMap<i32, i32> map;
      map.insert(10, 100);
      map.insert(20, 200);

      if (!check(map.erase(10), "erase(10)"))                       return false;
      if (!check_not(map.contains(10), "!contains(10) after erase")) return false;
      if (!check_eq(map.size(), 1u, "size == 1 after erase"))        return false;

      return check_not(map.erase(999), "erase(999) returns false");
    }

    auto operator_brackets() -> bool
    {
      HashMap<String, i32> map;
      map["Score"] = 150;
      if (!check_eq(map["Score"], 150, "map[\"Score\"] == 150")) return false;

      map["Score"] = 250;
      return check_eq(map["Score"], 250, "map[\"Score\"] == 250 after re-assign");
    }

    auto transparent_lookup() -> bool
    {
      HashMap<String, i32> map;
      map.insert(String("alpha"), 1);
      map.insert(String("beta"), 2);
      map.insert(String("gamma"), 3);

      if (!check(map.contains(StringView("alpha")), "contains(StringView)"))   return false;
      if (!check(map.contains("beta"), "contains(const char*)"))               return false;
      if (!check_not(map.contains(StringView("missing")), "!contains(missing)")) return false;

      i32 *p = map.find(StringView("gamma"));
      if (!check(p != nullptr, "find(StringView) hit")) return false;
      if (!check_eq(*p, 3, "*find(StringView) == 3"))   return false;

      if (!check(map.erase("beta"), "erase(const char*)"))                  return false;
      if (!check_not(map.contains("beta"), "!contains(\"beta\") post-erase")) return false;
      return check_eq(map.size(), 2u, "size == 2 post-erase");
    }
  };

  const test::AutoRegister<HashMapBlock> _registered;
} // namespace
