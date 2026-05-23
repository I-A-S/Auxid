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

module;

#include <cmath>
#include <concepts>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <type_traits>

export module auxid.test;

import auxid.core;
import auxid.containers;

export namespace au::console
{
  inline constexpr const char *RESET = "\033[0m";
  inline constexpr const char *RED = "\033[31m";
  inline constexpr const char *GREEN = "\033[32m";
  inline constexpr const char *YELLOW = "\033[33m";
  inline constexpr const char *BLUE = "\033[34m";
  inline constexpr const char *MAGENTA = "\033[35m";
  inline constexpr const char *CYAN = "\033[36m";
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
        char buffer[32];
        int len = snprintf(buffer, sizeof(buffer), "ptr(%p)", static_cast<const void *>(value));
        if (len > 0 && len < static_cast<int>(sizeof(buffer)))
          return String(buffer, static_cast<usize>(len));
        return String("ptr(err)");
      }
    }
    else if constexpr (std::is_arithmetic_v<T>)
    {
      char buffer[64];
      int len = 0;

      if constexpr (std::is_floating_point_v<T>)
      {
        len = snprintf(buffer, sizeof(buffer), "%f", static_cast<f64>(value));
      }
      else if constexpr (std::is_signed_v<T>)
      {
        len = snprintf(buffer, sizeof(buffer), "%lld", static_cast<long long>(value));
      }
      else if constexpr (std::is_unsigned_v<T>)
      {
        len = snprintf(buffer, sizeof(buffer), "%llu", static_cast<unsigned long long>(value));
      }

      if (len > 0 && len < static_cast<int>(sizeof(buffer)))
      {
        return String(buffer, static_cast<usize>(len));
      }
      return String("0");
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

    template<typename T1, typename T2>
    auto check_eq(const T1 &lhs, const T2 &rhs, const char *description) -> bool
    {
      if (lhs != rhs)
      {
        print_fail(description, to_string(lhs), to_string(rhs));
        return false;
      }
      return true;
    }

    template<typename T1, typename T2>
    auto check_neq(const T1 &lhs, const T2 &rhs, const char *description) -> bool
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
        printf("%s    %s... %sFAILED%s\n", console::BLUE, description, console::RED, console::RESET);
        return false;
      }
      return true;
    }

    auto check_not(const bool value, const char *description) -> bool
    {
      if (value)
      {
        printf("%s    %s... %sFAILED%s\n", console::BLUE, description, console::RED, console::RESET);
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
      printf("%s    %s... %sFAILED\n      Expected: %s\n      Actual:   %s%s\n", console::BLUE, desc, console::RED,
             v2.c_str(), v1.c_str(), console::RESET);
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

    [[nodiscard]] auto fail_count() const noexcept -> usize { return m_fail_count; }
    [[nodiscard]] auto test_count() const noexcept -> usize { return m_test_count; }

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

    printf("%sTesting [%s]...%s\n", console::MAGENTA, b.get_name(), console::RESET);

    for (TestUnit &v : b.units())
    {
      m_test_count++;
      if constexpr (IsVerbose)
      {
        printf("%s  Testing %s...\n%s", console::YELLOW, v.name.c_str(), console::RESET);
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
    putchar('\n');
  }

  template<bool StopOnFail, bool IsVerbose> auto Runner<StopOnFail, IsVerbose>::summarize() -> void
  {
    printf("%s\n-----------------------------------\n\t      SUMMARY\n-----------------------------------\n",
           console::GREEN);

    if (m_fail_count == 0)
    {
      printf("\n\tALL TESTS PASSED!\n\n");
    }
    else
    {
      const f64 success_rate =
          m_test_count == 0 ? 0.0
                            : (100.0 * static_cast<f64>(m_test_count - m_fail_count) / static_cast<f64>(m_test_count));
      printf("%s%zu OF %zu TESTS FAILED\n%sSuccess Rate: %.2f%%\n", console::RED, m_fail_count, m_test_count,
             console::YELLOW, success_rate);
    }

    printf("%sRan %zu test(s) across %zu block(s)\n%s-----------------------------------%s\n", console::MAGENTA,
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
      printf("%s[AUTest] Discovered %zu Test Blocks\n\n%s", console::CYAN, entries.size(), console::RESET);

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
