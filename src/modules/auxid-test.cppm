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
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <new>
#include <print>
#include <string>
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
        char buf[32];
        std::snprintf(buf, sizeof(buf), "ptr(%p)", static_cast<const void *>(value));
        return String(buf);
      }
    }
    else if constexpr (std::is_arithmetic_v<T>)
    {
      if constexpr (std::is_same_v<Decayed, bool>)
        return Decayed{value} ? String("true") : String("false");
      else if constexpr (std::is_integral_v<Decayed>)
      {
        using Wide = std::conditional_t<std::is_signed_v<Decayed>, long long, unsigned long long>;
        return String(std::to_string(static_cast<Wide>(value)).c_str());
      }
      else
        return String(std::to_string(static_cast<long double>(value)).c_str());
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
    String name;
    TestFunctor functor;
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

    Vec<TestUnit> m_units;
  };

  template<typename T>
  concept ValidBlockClass = std::derived_from<T, Block>;

  namespace impl
  {
    auto print_block_header(std::string_view name) -> void;
    auto print_test_progress(std::string_view name) -> void;
    auto print_blank_line() -> void;
    auto print_discovered(usize count) -> void;
    auto print_summary(usize fail_count, usize test_count, usize block_count) -> void;
  } // namespace impl

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

    usize m_test_count{0};
    usize m_fail_count{0};
    usize m_block_count{0};
  };

  template<bool StopOnFail, bool IsVerbose>
  template<typename BlockClass>
    requires ValidBlockClass<BlockClass>
  auto Runner<StopOnFail, IsVerbose>::test_block() -> void
  {
    m_block_count++;
    BlockClass b;
    b.declare_tests();

    impl::print_block_header(std::string_view(b.get_name()));

    Vec<TestUnit> &units = b.units();
    for (usize i = 0; i < units.size(); ++i)
    {
      TestUnit &v = units[i];
      m_test_count++;
      if constexpr (IsVerbose)
      {
        impl::print_test_progress(std::string_view(v.name.data(), v.name.size()));
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
    impl::print_blank_line();
  }

  template<bool StopOnFail, bool IsVerbose> auto Runner<StopOnFail, IsVerbose>::summarize() -> void
  {
    impl::print_summary(m_fail_count, m_test_count, m_block_count);
  }

  using DefaultRunner = Runner<false, true>;

  class TestRegistry
  {
public:
    using TestEntry = void (*)(DefaultRunner &);

    static auto get_entries() -> Vec<TestEntry> &
    {
      static Vec<TestEntry> entries;
      return entries;
    }

    static auto run_all() -> i32
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
  };

  auto register_test_block(TestRegistry::TestEntry entry) -> void;

  template<typename BlockType> void invoke_test_block(DefaultRunner &r)
  {
    r.test_block<BlockType>();
  }

  template<typename BlockType> struct AutoRegister
  {
    AutoRegister()
    {
      register_test_block(&invoke_test_block<BlockType>);
    }
  };
} // namespace au::test
