// Auxid: The Rigid C++ Platform.
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
  struct HashSetBlock final : test::Block
  {
    [[nodiscard]] auto get_name() const -> const char * override
    {
      return "containers::hash_set";
    }

    auto declare_tests() -> void override
    {
      add_test("insert_and_contains", [this] { return insert_and_contains(); });
      add_test("erase_and_clear", [this] { return erase_and_clear(); });
    }

    auto insert_and_contains() -> bool
    {
      HashSet<String> set;

      if (!check(set.insert("Core"), "insert(\"Core\")"))
        return false;
      if (!check(set.insert("Renderer"), "insert(\"Renderer\")"))
        return false;

      if (!check(set.contains("Core"), "contains(\"Core\")"))
        return false;
      if (!check(set.contains("Renderer"), "contains(\"Renderer\")"))
        return false;
      if (!check_not(set.contains("Physics"), "!contains(\"Physics\")"))
        return false;

      if (!check_not(set.insert("Core"), "duplicate insert returns false"))
        return false;
      return check_eq(set.size(), 2u, "set.size() == 2");
    }

    auto erase_and_clear() -> bool
    {
      HashSet<i32> set;
      set.insert(10);
      set.insert(20);
      set.insert(30);

      if (!check(set.erase(20), "erase(20)"))
        return false;
      if (!check_not(set.contains(20), "!contains(20) after erase"))
        return false;
      if (!check_eq(set.size(), 2u, "size == 2 after erase"))
        return false;
      if (!check_not(set.erase(999), "erase(999) returns false"))
        return false;

      set.clear();
      return check(set.empty(), "set.empty() after clear") && check_eq(set.size(), 0u, "size == 0 after clear");
    }
  };

  const test::AutoRegister<HashSetBlock> _registered;
} // namespace
