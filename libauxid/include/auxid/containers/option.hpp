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

#include <auxid/pch.hpp>

namespace au::containers
{
  struct NullOptType
  {
    explicit constexpr NullOptType(int)
    {
    }
  };

  inline constexpr NullOptType nullopt{0};

  template<typename T> class [[nodiscard]] Option
  {
    static_assert(!std::is_reference_v<T>, "Option<T&> is not allowed. Use Option<T*>.");

    union {
      char m_dummy;
      T m_val;
    };

    bool m_has_value;

public:
    constexpr Option() noexcept : m_dummy(0), m_has_value(false)
    {
    }

    constexpr Option(NullOptType) noexcept : m_dummy(0), m_has_value(false)
    {
    }

    constexpr Option(const T &val) : m_val(val), m_has_value(true)
    {
    }

    constexpr Option(T &&val) : m_val(std::move(val)), m_has_value(true)
    {
    }

    constexpr Option(const Option &other)
    {
      if (other.m_has_value)
      {
        std::construct_at(&m_val, other.m_val);
        m_has_value = true;
      }
      else
      {
        m_has_value = false;
      }
    }

    constexpr Option(Option &&other) noexcept
    {
      if (other.m_has_value)
      {
        std::construct_at(&m_val, std::move(other.m_val));
        m_has_value = true;
      }
      else
      {
        m_has_value = false;
      }
    }

    ~Option()
    {
      reset();
    }

public:
    Option &operator=(NullOptType)
    {
      reset();
      return *this;
    }

    Option &operator=(const Option &other)
    {
      if (this != &other)
      {
        if (other.m_has_value)
        {
          if (m_has_value)
          {
            m_val = other.m_val;
          }
          else
          {
            std::construct_at(&m_val, other.m_val);
            m_has_value = true;
          }
        }
        else
        {
          reset();
        }
      }
      return *this;
    }

    Option &operator=(Option &&other) noexcept
    {
      if (this != &other)
      {
        if (other.m_has_value)
        {
          if (m_has_value)
          {
            m_val = std::move(other.m_val);
          }
          else
          {
            std::construct_at(&m_val, std::move(other.m_val));
            m_has_value = true;
          }
        }
        else
        {
          reset();
        }
      }
      return *this;
    }

public:
    [[nodiscard]] constexpr bool has_value() const
    {
      return m_has_value;
    }

    [[nodiscard]] constexpr bool is_some() const
    {
      return m_has_value;
    }

    [[nodiscard]] constexpr bool is_none() const
    {
      return !m_has_value;
    }

    constexpr explicit operator bool() const
    {
      return m_has_value;
    }

    constexpr T &operator*() &
    {
#if !defined(NDEBUG)
      if (!m_has_value)
        panic("Option::operator* on None");
#endif
      return m_val;
    }

    constexpr const T &operator*() const &
    {
#if !defined(NDEBUG)
      if (!m_has_value)
        panic("Option::operator* on None");
#endif
      return m_val;
    }

    constexpr T *operator->()
    {
#if !defined(NDEBUG)
      if (!m_has_value)
        panic("Option::operator-> on None");
#endif
      return &m_val;
    }

    constexpr const T *operator->() const
    {
#if !defined(NDEBUG)
      if (!m_has_value)
        panic("Option::operator-> on None");
#endif
      return &m_val;
    }

    constexpr T &unwrap(std::source_location loc = std::source_location::current()) &
    {
      if (!m_has_value)
        panic("Called unwrap() on None Option", loc);
      return m_val;
    }

    constexpr const T &unwrap(std::source_location loc = std::source_location::current()) const &
    {
      if (!m_has_value)
        panic("Called unwrap() on None Option", loc);
      return m_val;
    }

    constexpr T unwrap(std::source_location loc = std::source_location::current()) &&
    {
      if (!m_has_value)
        panic("Called unwrap() on None Option", loc);
      return std::move(m_val);
    }

    constexpr T &expect(const char *msg, std::source_location loc = std::source_location::current()) &
    {
      if (!m_has_value)
        panic(msg, loc);
      return m_val;
    }

    template<typename U> constexpr T value_or(U &&def) const &
    {
      return m_has_value ? m_val : static_cast<T>(std::forward<U>(def));
    }

    template<typename U> constexpr T value_or(U &&def) &&
    {
      return m_has_value ? std::move(m_val) : static_cast<T>(std::forward<U>(def));
    }

    template<typename F> constexpr auto map(F &&f) const & -> Option<std::invoke_result_t<F, const T &>>
    {
      if (m_has_value)
      {
        return std::invoke(std::forward<F>(f), m_val);
      }
      return nullopt;
    }

public:
    void reset()
    {
      if (m_has_value)
      {
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
          std::destroy_at(&m_val);
        }
        m_has_value = false;
      }
    }
  };

} // namespace au::containers

namespace au
{
  template<typename T> using Option = containers::Option<T>;
}
