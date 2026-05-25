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

#include <cmath>
#include <concepts>
#include <cstdlib>
#include <functional>
#include <print>
#include <string_view>
#include <type_traits>

export module auxid.test;

import auxid.core;
import auxid.containers;

export namespace au::console
{
  inline constexpr std::string_view RESET = "\033[0m";
  inline constexpr std::string_view RED = "\033[31m";
  inline constexpr std::string_view GREEN = "\033[32m";
  inline constexpr std::string_view YELLOW = "\033[33m";
  inline constexpr std::string_view BLUE = "\033[34m";
  inline constexpr std::string_view MAGENTA = "\033[35m";
  inline constexpr std::string_view CYAN = "\033[36m";
} // namespace au::console

export namespace au::test
{
  template<typename T>
  concept HasToStringMethod = requires(const T &t) {
    { t.to_string() } -> std::convertible_to<String>;
  };

  template<typename T> auto to_string(const T &value) -> String
  {
    using Decayed = std::decay_t<T>;

    if constexpr (std::is_pointer_v<Decayed>)
    {
      if constexpr (std::is_same_v<Decayed, const char *> || std::is_same_v<Decayed, char *>)
      {
        if (value == nullptr)
          return String("\"nullptr\"");
        return String("\"") + String(value) + "\"";
      }
      else
      {
        if (value == nullptr)
          return String("nullptr");
        return String::format("ptr({})", static_cast<const void *>(value));
      }
    }
    else if constexpr (std::is_arithmetic_v<T>)
    {
      return String::format("{}", value);
    }
    else if constexpr (std::is_convertible_v<T, String>)
    {
      return String("\"") + String(value) + "\"";
    }
    else if constexpr (HasToStringMethod<T>)
    {
      return value.to_string();
    }
    else
    {
      return String("{Object}");
    }
  }

  using TestFunctor = std::function<bool()>;

  struct TestUnit
  {
    Mut<String> name;
    Mut<TestFunctor> functor;
  };

  class Block
  {
public:
    virtual ~Block() = default;
    [[nodiscard]] virtual auto get_name() const -> const char * = 0;
    virtual auto declare_tests() -> void = 0;

    auto units() -> Vec<TestUnit> &
    {
      return m_units;
    }

    template<typename T1, typename T2> auto check_eq(const T1 &lhs, const T2 &rhs, const char *description) -> bool
    {
      if (lhs != rhs)
      {
        print_fail(description, to_string(lhs), to_string(rhs));
        return false;
      }
      return true;
    }

    template<typename T1, typename T2> auto check_neq(const T1 &lhs, const T2 &rhs, const char *description) -> bool
    {
      if (lhs == rhs)
      {
        print_fail(description, to_string(lhs), "NOT " + to_string(rhs));
        return false;
      }
      return true;
    }

    template<typename T>
    auto check_approx(const T lhs, const T rhs, const char *description, const T epsilon = static_cast<T>(0.001))
        -> bool
    {
      static_assert(std::is_floating_point_v<T>, "check_approx only works for floats/doubles");

      if (lhs == static_cast<T>(0.0) || rhs == static_cast<T>(0.0))
      {
        if (std::abs(lhs - rhs) > epsilon)
        {
          print_fail(description, to_string(lhs), to_string(rhs));
          return false;
        }
        return true;
      }

      const T diff = std::abs(lhs - rhs);
      const T larger = std::max(std::abs(lhs), std::abs(rhs));

      if (diff > (larger * epsilon))
      {
        print_fail(description, to_string(lhs), to_string(rhs));
        return false;
      }
      return true;
    }

    auto check(const bool value, const char *description) -> bool
    {
      if (!value)
      {
        std::println("{}    {}... {}FAILED{}", console::BLUE, std::string_view(description), console::RED,
                     console::RESET);
        return false;
      }
      return true;
    }

    auto check_not(const bool value, const char *description) -> bool
    {
      if (value)
      {
        std::println("{}    {}... {}FAILED{}", console::BLUE, std::string_view(description), console::RED,
                     console::RESET);
        return false;
      }
      return true;
    }

    auto add_test(const char *name, TestFunctor functor) -> void
    {
      m_units.push_back({String(name), std::move(functor)});
    }

private:
    auto print_fail(const char *desc, const String &v1, const String &v2) -> void
    {
      std::println("{}    {}... {}FAILED\n      Expected: {}\n      Actual:   {}{}", console::BLUE,
                   std::string_view(desc), console::RED, v2, v1, console::RESET);
    }

    Mut<Vec<TestUnit>> m_units;
  };

  template<typename T>
  concept ValidBlockClass = std::derived_from<T, Block>;

  template<bool StopOnFail = false, bool IsVerbose = false> class Runner
  {
public:
    Runner() = default;

    ~Runner()
    {
      summarize();
    }

    template<typename BlockClass>
      requires ValidBlockClass<BlockClass>
    auto test_block() -> void;

    [[nodiscard]] auto fail_count() const noexcept -> usize
    {
      return m_fail_count;
    }

    [[nodiscard]] auto test_count() const noexcept -> usize
    {
      return m_test_count;
    }

private:
    auto summarize() -> void;

    Mut<usize> m_test_count{0};
    Mut<usize> m_fail_count{0};
    Mut<usize> m_block_count{0};
  };

  template<bool StopOnFail, bool IsVerbose>
  template<typename BlockClass>
    requires ValidBlockClass<BlockClass>
  auto Runner<StopOnFail, IsVerbose>::test_block() -> void
  {
    m_block_count++;
    Mut<BlockClass> b;
    b.declare_tests();

    std::println("{}Testing [{}]...{}", console::MAGENTA, std::string_view(b.get_name()), console::RESET);

    for (TestUnit &v : b.units())
    {
      m_test_count++;
      if constexpr (IsVerbose)
      {
        std::print("{}  Testing {}...\n{}", console::YELLOW, v.name, console::RESET);
      }

      const bool result = v.functor();

      if (!result)
      {
        m_fail_count++;
        if constexpr (StopOnFail)
        {
          summarize();
          std::exit(-1);
        }
      }
    }
    std::println();
  }

  template<bool StopOnFail, bool IsVerbose> auto Runner<StopOnFail, IsVerbose>::summarize() -> void
  {
    std::println("{}\n-----------------------------------\n\t      SUMMARY\n-----------------------------------",
                 console::GREEN);

    if (m_fail_count == 0)
    {
      std::println("\n\tALL TESTS PASSED!\n");
    }
    else
    {
      const f64 success_rate =
          m_test_count == 0 ? 0.0
                            : (100.0 * static_cast<f64>(m_test_count - m_fail_count) / static_cast<f64>(m_test_count));
      std::println("{}{} OF {} TESTS FAILED\n{}Success Rate: {:.2f}%", console::RED, m_fail_count, m_test_count,
                   console::YELLOW, success_rate);
    }

    std::println("{}Ran {} test(s) across {} block(s)\n{}-----------------------------------{}", console::MAGENTA,
                 m_test_count, m_block_count, console::GREEN, console::RESET);
  }

  using DefaultRunner = Runner<false, true>;

  class TestRegistry
  {
public:
    using TestEntry = std::function<void(DefaultRunner &)>;

    static auto get_entries() -> Vec<TestEntry> &
    {
      static Mut<Vec<TestEntry>> entries;
      return entries;
    }

    static auto run_all() -> i32
    {
      Mut<DefaultRunner> r;
      Vec<TestEntry> &entries = get_entries();
      std::print("{}[AUTest] Discovered {} Test Blocks\n\n{}", console::CYAN, entries.size(), console::RESET);

      for (TestEntry &entry : entries)
      {
        entry(r);
      }

      return r.fail_count() == 0 ? 0 : 1;
    }
  };

  template<typename BlockType> struct AutoRegister
  {
    AutoRegister()
    {
      TestRegistry::get_entries().push_back([](DefaultRunner &r) { r.test_block<BlockType>(); });
    }
  };
} // namespace au::test
