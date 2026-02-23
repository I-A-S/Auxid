// Auxid: The Orthodox C++ Platform.
// Copyright (C) 2026 IAS (ias@iasoft.dev)
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

#pragma once

#include <functional>

#include <auxid/auxid.hpp>
#include <auxid/containers/vec.hpp>
#include <auxid/containers/string.hpp>

#define __aut_micro_test(call)                                                                                         \
  do                                                                                                                   \
  {                                                                                                                    \
    if (!(call))                                                                                                       \
      return false;                                                                                                    \
  } while (false)

#define AUT_CHECK(v) __aut_micro_test(_test((v), #v))
#define AUT_CHECK_NOT(v) __aut_micro_test(_test_not((v), "NOT " #v))
#define AUT_CHECK_EQ(lhs, rhs) __aut_micro_test(_test_eq((lhs), (rhs), #lhs " == " #rhs))
#define AUT_CHECK_NEQ(lhs, rhs) __aut_micro_test(_test_neq((lhs), (rhs), #lhs " != " #rhs))

#define AUT_CHECK_APPROX(lhs, rhs) __aut_micro_test(_test_approx((lhs), (rhs), #lhs " ~= " #rhs))
#define AUT_CHECK_APPROX_EPS(lhs, rhs, eps) __aut_micro_test(_test_approx((lhs), (rhs), #lhs " ~= " #rhs, (eps)))

#define AUT_UNIT(func) _test_unit([this]() { return this->func(); }, #func)
#define AUT_NAMED_UNIT(n, func) _test_unit([this]() { return this->func(); }, n)

#define AUT_BLOCK(name) class name : public au::test::Block

#define AUT_BEGIN_BLOCK(_group, _name)                                                                                 \
  class _group##_##_name : public au::test::Block                                                                      \
  {                                                                                                                    \
public:                                                                                                                \
    [[nodiscard]] auto get_name() const -> const char * override                                                       \
    {                                                                                                                  \
      return #_group "::" #_name;                                                                                      \
    }                                                                                                                  \
                                                                                                                       \
private:

#define AUT_END_BLOCK()                                                                                                \
  }                                                                                                                    \
  ;

#define AUT_BEGIN_TEST_LIST()                                                                                          \
  public:                                                                                                              \
  auto declare_tests() -> void override                                                                                \
  {
#define AUT_ADD_TEST(name) AUT_UNIT(name)
#define AUT_END_TEST_LIST()                                                                                            \
  }                                                                                                                    \
                                                                                                                       \
  private:

namespace au::test
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
        if (len > 0 && len < sizeof(buffer))
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

      if (len > 0 && len < sizeof(buffer))
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

protected:
    template<typename T1, typename T2> auto _test_eq(const T1 &lhs, const T2 &rhs, const char *description) -> bool
    {
      if (lhs != rhs)
      {
        print_fail(description, to_string(lhs), to_string(rhs));
        return false;
      }
      return true;
    }

    template<typename T1, typename T2> auto _test_neq(const T1 &lhs, const T2 &rhs, const char *description) -> bool
    {
      if (lhs == rhs)
      {
        print_fail(description, to_string(lhs), "NOT " + to_string(rhs));
        return false;
      }
      return true;
    }

    template<typename T>
    auto _test_approx(const T lhs, const T rhs, const char *description, const T epsilon = static_cast<T>(0.001))
        -> bool
    {
      static_assert(std::is_floating_point_v<T>, "Approx only works for floats/doubles");

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

    auto _test(const bool value, const char *description) -> bool
    {
      if (!value)
      {
        printf("%s    %s... %sFAILED%s\n", console::BLUE, description, console::RED, console::RESET);
        return false;
      }
      return true;
    }

    auto _test_not(const bool value, const char *description) -> bool
    {
      if (value)
      {
        printf("%s    %s... %sFAILED%s\n", console::BLUE, description, console::RED, console::RESET);
        return false;
      }
      return true;
    }

    auto _test_unit(Mut<TestFunctor> functor, const char *name) -> void
    {
      m_units.push_back({name, std::move(functor)});
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

      bool result = false;

      try
      {
        result = v.functor();
      }
      catch (const std::exception &e)
      {
        printf("%s    [EXCEPTION] %s: %s%s\n", console::RED, v.name.c_str(), e.what(), console::RESET);
        result = false;
      }
      catch (...)
      {
        printf("%s    [UNKNOWN EXCEPTION] %s%s\n", console::RED, v.name.c_str(), console::RESET);
        result = false;
      }

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

      return 0;
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

#define AUT_REGISTER_ENTRY(Group, Name) static au::test::AutoRegister<Group##_##Name> _aut_reg_##Group##_##Name;
