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

module;

#include <print>
#include <string_view>
#include <vector>

module auxid.test;

namespace au::test
{
  auto TestRegistry::get_entries() -> Vec<TestEntry> &
  {
    static Vec<TestEntry> entries;
    return entries;
  }

  auto TestRegistry::run_all() -> i32
  {
    DefaultRunner r;
    Vec<TestEntry> &entries = get_entries();
    impl::print_discovered(entries.size());

    for (usize i = 0; i < entries.size(); ++i)
    {
      entries[i](r);
    }

    return r.fail_count() == 0 ? 0 : 1;
  }

  auto register_test_block(TestRegistry::TestEntry entry) -> void
  {
    TestRegistry::get_entries().push_back(entry);
  }
} // namespace au::test

namespace au::test::impl
{
  auto print_block_header(std::string_view name) -> void
  {
    std::println("{}Testing [{}]...{}", console::MAGENTA, name, console::RESET);
  }

  auto print_test_progress(std::string_view name) -> void
  {
    std::print("{}  Testing {}...\n{}", console::YELLOW, name, console::RESET);
  }

  auto print_blank_line() -> void
  {
    std::println();
  }

  auto print_discovered(usize count) -> void
  {
    std::print("{}[AUTest] Discovered {} Test Blocks\n\n{}", console::CYAN, count, console::RESET);
  }

  auto print_summary(usize fail_count, usize test_count, usize block_count) -> void
  {
    std::println("{}\n-----------------------------------\n\t      SUMMARY\n-----------------------------------",
                 console::GREEN);

    if (fail_count == 0)
    {
      std::println("\n\tALL TESTS PASSED!\n");
    }
    else
    {
      const f64 success_rate =
          test_count == 0 ? 0.0 : (100.0 * static_cast<f64>(test_count - fail_count) / static_cast<f64>(test_count));
      std::println("{}{} OF {} TESTS FAILED\n{}Success Rate: {:.2f}%", console::RED, fail_count, test_count,
                   console::YELLOW, success_rate);
    }

    std::println("{}Ran {} test(s) across {} block(s)\n{}-----------------------------------{}", console::MAGENTA,
                 test_count, block_count, console::GREEN, console::RESET);
  }
} // namespace au::test::impl
