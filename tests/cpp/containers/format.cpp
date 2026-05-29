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
  struct FormatBlock final : test::Block
  {
    [[nodiscard]] auto get_name() const -> const char * override
    {
      return "containers::format";
    }

    auto declare_tests() -> void override
    {
      add_test("std_format_string", [this] { return std_format_string(); });
      add_test("logger_format_string", [this] { return logger_format_string(); });
    }

    auto std_format_string() -> bool
    {
      const String s("hello");
      const StringView sv("world");
      return check_eq(String::format("{}", s), "hello", "String::format with String") &&
             check_eq(String::format("{}", sv), "world", "String::format with StringView");
    }

    auto logger_format_string() -> bool
    {
      const String s("format regression");
      auxid::get_thread_logger().error("{}", s);
      return true;
    }
  };

  const test::AutoRegister<FormatBlock> _registered;
} // namespace
