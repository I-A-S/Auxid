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
  struct StringBlock final : test::Block
  {
    [[nodiscard]] auto get_name() const -> const char * override { return "containers::string"; }

    auto declare_tests() -> void override
    {
      add_test("sso",                [this] { return sso(); });
      add_test("heap_allocation",    [this] { return heap_allocation(); });
      add_test("append_and_concat",  [this] { return append_and_concat(); });
      add_test("push_pop",           [this] { return push_pop(); });
    }

    auto sso() -> bool
    {
      String s("Orthodox");
      return check_eq(s.size(), 8u, "s.size() == 8")
          && check_eq(s, "Orthodox", "s == \"Orthodox\"");
    }

    auto heap_allocation() -> bool
    {
      String s("This string is deliberately long to bypass the SSO capacity.");
      return check(s.size() > 23u, "s.size() > 23 (heap path)")
          && check_eq(s.substr(0, 4), "This", "s.substr(0,4) == \"This\"");
    }

    auto append_and_concat() -> bool
    {
      String s("Data");
      s += " Oriented";
      if (!check_eq(s, "Data Oriented", "after += \" Oriented\""))
        return false;

      String combined = s + String(" Design");
      return check_eq(combined, "Data Oriented Design", "concat result");
    }

    auto push_pop() -> bool
    {
      String s("C+");
      s.push_back('+');
      if (!check_eq(s, "C++", "after push_back('+')"))
        return false;
      s.pop_back();
      return check_eq(s, "C+", "after pop_back");
    }
  };

  const test::AutoRegister<StringBlock> _registered;
} // namespace
