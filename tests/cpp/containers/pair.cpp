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

#include <utility>

import auxid;
import auxid.test;

using namespace au;

namespace
{
  struct PairBlock final : test::Block
  {
    [[nodiscard]] auto get_name() const -> const char * override
    {
      return "containers::pair";
    }

    auto declare_tests() -> void override
    {
      add_test("construction", [this] { return construction(); });
      add_test("move_semantics", [this] { return move_semantics(); });
    }

    auto construction() -> bool
    {
      Pair<i32, String> p(42, "Answer");
      return check_eq(p.first, 42, "p.first == 42") && check_eq(p.second, "Answer", "p.second == \"Answer\"");
    }

    auto move_semantics() -> bool
    {
      Pair<i32, String> p1(100, "Moving");
      Pair<i32, String> p2(std::move(p1));
      return check_eq(p2.first, 100, "p2.first == 100") && check_eq(p2.second, "Moving", "p2.second == \"Moving\"");
    }
  };

  const test::AutoRegister<PairBlock> _registered;
} // namespace
