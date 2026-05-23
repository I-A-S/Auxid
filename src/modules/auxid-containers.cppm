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

#include <auxid/macros.hpp>

#include <algorithm>
#include <atomic>
#include <bit>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <source_location>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <wyhash/wyhash.h>

static_assert(std::endian::native == std::endian::little,
              "Auxid String SSO is designed for Little-Endian architectures.");

export module auxid.containers;

export import auxid.core;
export import auxid.memory;

export namespace au::containers
{
  template<typename T> using Span = std::span<T>;
  template<typename T1, typename T2> using Pair = std::pair<T1, T2>;
}

export namespace au
{
  template<typename T> using Span = std::span<T>;
  template<typename T1, typename T2> using Pair = std::pair<T1, T2>;
}

export namespace au
{
  struct StringView;

  namespace containers
  {
    struct String;
  }
} // namespace au

namespace au
{
  namespace internal
  {
    inline int compare(const char *s1, const char *s2, usize n)
    {
      if (n == 0)
        return 0;
      return compiler::memcmp(s1, s2, n);
    }

    inline usize length(const char *s)
    {
      return compiler::strlen(s);
    }

    inline const char *find(const char *p, usize n, char c)
    {
      return static_cast<const char *>(compiler::memchr(p, c, n));
    }

    inline usize search_substring(const char *haystack, usize h_len, const char *needle, usize n_len, usize pos)
    {
      if (n_len == 0)
        return pos <= h_len ? pos : static_cast<usize>(-1);
      if (pos >= h_len)
        return static_cast<usize>(-1);

      const usize last_possible = h_len - n_len;
      if (pos > last_possible)
        return static_cast<usize>(-1);

      for (usize i = pos; i <= last_possible; ++i)
      {
        if (haystack[i] == needle[0])
        {
          if (compare(haystack + i + 1, needle + 1, n_len - 1) == 0)
          {
            return i;
          }
        }
      }
      return static_cast<usize>(-1);
    }
  } // namespace internal
}

export namespace au
{
  struct StringView
  {
    static constexpr usize npos = static_cast<usize>(-1);

    const char *m_ptr = nullptr;
    usize m_len = 0;

public:
    constexpr StringView() = default;

    constexpr StringView(const char *s, usize l) : m_ptr(s), m_len(l)
    {
    }

    constexpr StringView(const char *s) : m_ptr(s), m_len(s ? internal::length(s) : 0)
    {
    }

    constexpr StringView(containers::Span<const char> span) : m_ptr(span.data()), m_len(span.size())
    {
    }

public:
    using value_type = char;
    using size_type = usize;
    using difference_type = isize;
    using iterator = const char *;
    using const_iterator = const char *;
    using reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr char operator[](usize i) const
    {
      return m_ptr[i];
    }

    [[nodiscard]] constexpr const char *data() const
    {
      return m_ptr;
    }

    [[nodiscard]] constexpr usize size() const
    {
      return m_len;
    }

    [[nodiscard]] constexpr bool empty() const
    {
      return m_len == 0;
    }

    [[nodiscard]] constexpr iterator begin() const noexcept
    {
      return m_ptr;
    }

    [[nodiscard]] constexpr iterator end() const noexcept
    {
      return m_ptr + m_len;
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
      return begin();
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
      return end();
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() const noexcept
    {
      return reverse_iterator(end());
    }

    [[nodiscard]] constexpr reverse_iterator rend() const noexcept
    {
      return reverse_iterator(begin());
    }

    [[nodiscard]] constexpr reverse_iterator crbegin() const noexcept
    {
      return rbegin();
    }

    [[nodiscard]] constexpr reverse_iterator crend() const noexcept
    {
      return rend();
    }

    [[nodiscard]] constexpr char back() const
    {
      return m_ptr[m_len - 1];
    }

    [[nodiscard]] usize find(char c, usize pos = 0) const
    {
      if (pos >= m_len)
        return npos;
      const char *res = internal::find(m_ptr + pos, m_len - pos, c);
      return res ? static_cast<usize>(res - m_ptr) : npos;
    }

    [[nodiscard]] usize find(StringView v, usize pos = 0) const
    {
      return internal::search_substring(m_ptr, m_len, v.data(), v.size(), pos);
    }

    [[nodiscard]] usize find(const char *s, usize pos = 0) const
    {
      return find(StringView(s), pos);
    }

    constexpr bool operator==(StringView other) const
    {
      if (m_len != other.m_len)
        return false;
      return internal::compare(m_ptr, other.m_ptr, m_len) == 0;
    }

    constexpr operator containers::Span<const char>() const
    {
      return containers::Span<const char>(m_ptr, m_len);
    }

    [[nodiscard]] StringView substr(usize pos, usize count = npos) const
    {
      if (pos >= m_len)
        return StringView();
      usize rcount = (count > m_len - pos) ? (m_len - pos) : count;
      return StringView(m_ptr + pos, rcount);
    }

    [[nodiscard]] bool starts_with(StringView prefix) const
    {
      if (prefix.size() > m_len)
        return false;
      return internal::compare(m_ptr, prefix.data(), prefix.size()) == 0;
    }

    [[nodiscard]] bool ends_with(StringView suffix) const
    {
      if (suffix.size() > m_len)
        return false;
      return internal::compare(m_ptr + m_len - suffix.size(), suffix.data(), suffix.size()) == 0;
    }
  };

  u64 hash_string_view(StringView sv)
  {
    return ::wyhash(sv.data(), sv.size(), 0, ::_wyp);
  }
} // namespace au

export namespace au::containers
{
  struct String
  {
    static constexpr usize npos = StringView::npos;

    static constexpr usize SSO_CAPACITY = sizeof(usize) * 3 - 1;

    using value_type = char;
    using size_type = usize;
    using difference_type = isize;
    using reference = char &;
    using const_reference = const char &;
    using pointer = char *;
    using const_pointer = const char *;
    using iterator = char *;
    using const_iterator = const char *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    struct LongLayout
    {
      usize cap_flagged;
      usize size;
      char *ptr;
    };

    struct ShortLayout
    {
      u8 size_shifted;
      char data[SSO_CAPACITY + 1];
    };

    union {
      LongLayout l;
      ShortLayout s;
    } m_storage;

    AUXID_NO_UNIQUE_ADDRESS memory::HeapAllocator m_allocator;

    [[nodiscard]] bool is_short() const
    {
      return !(m_storage.s.size_shifted & 1);
    }

    [[nodiscard]] usize get_short_size() const
    {
      return m_storage.s.size_shifted >> 1;
    }

    void set_short_size(u8 size)
    {
      m_storage.s.size_shifted = static_cast<u8>(size << 1);
    }

    [[nodiscard]] char *get_data()
    {
      return is_short() ? m_storage.s.data : m_storage.l.ptr;
    }

    [[nodiscard]] const char *get_data() const
    {
      return is_short() ? m_storage.s.data : m_storage.l.ptr;
    }

    [[nodiscard]] usize get_size() const
    {
      return is_short() ? get_short_size() : m_storage.l.size;
    }

    [[nodiscard]] usize get_capacity() const
    {
      return is_short() ? SSO_CAPACITY : (m_storage.l.cap_flagged >> 1);
    }

    void set_long_capacity(usize cap)
    {
      m_storage.l.cap_flagged = (cap << 1) | 1;
    }

public:
    String()
    {
      m_storage.s.size_shifted = 0;
      m_storage.s.data[0] = '\0';
    }

    String(const char *str)
    {
      m_storage.s.size_shifted = 0;
      if (str)
        assign(StringView(str));
      else
        m_storage.s.data[0] = '\0';
    }

    String(const char *str, usize len)
    {
      m_storage.s.size_shifted = 0;

      if (str != nullptr && len > 0)
      {
        assign(StringView(str, len));
      }
      else
      {
        m_storage.s.data[0] = '\0';
      }
    }

    String(StringView sv)
    {
      m_storage.s.size_shifted = 0;
      assign(sv);
    }

    String(String &&other) noexcept : m_allocator(static_cast<memory::HeapAllocator &&>(other.m_allocator))
    {
      std::memcpy(&m_storage, &other.m_storage, sizeof(m_storage));

      other.m_storage.s.size_shifted = 0;
      other.m_storage.s.data[0] = '\0';
    }

    String &operator=(String &&other) noexcept
    {
      if (this != &other)
      {
        destroy();
        m_allocator = static_cast<memory::HeapAllocator &&>(other.m_allocator);

        std::memcpy(&m_storage, &other.m_storage, sizeof(m_storage));
        other.m_storage.s.size_shifted = 0;
        other.m_storage.s.data[0] = '\0';
      }
      return *this;
    }

    String(const String &other)
    {
      m_storage.s.size_shifted = 0;
      assign(StringView(other.data(), other.size()));
    }

    String &operator=(const String &other)
    {
      if (this != &other)
      {
        assign(StringView(other.data(), other.size()));
      }
      return *this;
    }

    ~String()
    {
      destroy();
    }

    [[nodiscard]] String clone() const
    {
      String new_str;
      new_str.assign(StringView(get_data(), get_size()));
      return new_str;
    }

    void destroy()
    {
      if (!is_short())
      {
        m_allocator.free(m_storage.l.ptr, get_capacity() + 1, 1);
      }
      m_storage.s.size_shifted = 0;
      m_storage.s.data[0] = '\0';
    }

    void reserve(usize new_cap)
    {
      usize current_cap = get_capacity();
      if (new_cap <= current_cap)
        return;

      const usize new_alloc_size = new_cap + 1;

      if (is_short())
      {
        char *new_mem = (char *) m_allocator.alloc(new_alloc_size, 1);

        usize current_len = get_short_size();
        if (current_len > 0)
        {
          std::memcpy(new_mem, m_storage.s.data, current_len);
        }
        new_mem[current_len] = '\0';

        m_storage.l.ptr = new_mem;
        m_storage.l.size = current_len;
        set_long_capacity(new_cap);
      }
      else
      {
        const usize old_alloc_size = current_cap + 1;

        void *expanded = m_allocator.realloc(m_storage.l.ptr, old_alloc_size, new_alloc_size, 1);

        if (expanded)
        {
          m_storage.l.ptr = (char *) expanded;
          set_long_capacity(new_cap);
        }
        else
        {
          char *new_mem = (char *) m_allocator.alloc(new_alloc_size, 1);

          usize current_len = m_storage.l.size;
          std::memcpy(new_mem, m_storage.l.ptr, current_len);
          new_mem[current_len] = '\0';

          m_allocator.free(m_storage.l.ptr, old_alloc_size, 1);

          m_storage.l.ptr = new_mem;
          set_long_capacity(new_cap);
        }
      }
    }

    void assign(StringView sv)
    {
      usize len = sv.size();
      if (len > get_capacity())
      {
        reserve(len);
      }

      if (is_short())
      {
        set_short_size(static_cast<u8>(len));
        if (len > 0)
          std::memcpy(m_storage.s.data, sv.data(), len);
        m_storage.s.data[len] = '\0';
      }
      else
      {
        if (len > 0)
          std::memcpy(m_storage.l.ptr, sv.data(), len);
        m_storage.l.ptr[len] = '\0';
        m_storage.l.size = len;
      }
    }

    void append(StringView sv)
    {
      if (sv.empty())
        return;

      usize cur_len = size();
      usize req_len = cur_len + sv.size();

      if (req_len > get_capacity())
      {
        usize next_cap = get_capacity() * 2;
        if (next_cap < req_len)
          next_cap = req_len;
        reserve(next_cap);
      }

      char *dest = get_data() + cur_len;
      std::memcpy(dest, sv.data(), sv.size());

      if (is_short())
      {
        set_short_size(static_cast<u8>(req_len));
        m_storage.s.data[req_len] = '\0';
      }
      else
      {
        m_storage.l.size = req_len;
        m_storage.l.ptr[req_len] = '\0';
      }
    }

public:
    [[nodiscard]] const char *c_str() const
    {
      return get_data();
    }

    [[nodiscard]] const char *data() const
    {
      return get_data();
    }

    [[nodiscard]] char *data()
    {
      return get_data();
    }

    [[nodiscard]] usize size() const
    {
      return get_size();
    }

    [[nodiscard]] usize length() const
    {
      return get_size();
    }

    [[nodiscard]] bool empty() const
    {
      return get_size() == 0;
    }

public:
    void push(char c)
    {
      char tmp = c;
      append(StringView(&tmp, 1));
    }

    void pop()
    {
      const usize current_size = get_size();

      if (current_size > 0)
      {
        const usize new_size = current_size - 1;

        if (is_short())
        {
          m_storage.s.data[new_size] = '\0';
          set_short_size(static_cast<u8>(new_size));
        }
        else
        {
          m_storage.l.ptr[new_size] = '\0';
          m_storage.l.size = new_size;
        }
      }
    }

    void push_back(char c)
    {
      usize cur_len = get_size();
      if (cur_len >= get_capacity())
      {
        reserve(cur_len == 0 ? 8 : get_capacity() * 2);
      }

      if (is_short())
      {
        m_storage.s.data[cur_len] = c;
        m_storage.s.data[cur_len + 1] = '\0';
        set_short_size(static_cast<u8>(cur_len + 1));
      }
      else
      {
        m_storage.l.ptr[cur_len] = c;
        m_storage.l.ptr[cur_len + 1] = '\0';
        m_storage.l.size = cur_len + 1;
      }
    }

    void pop_back()
    {
      pop();
    }

public:
    char *begin()
    {
      return get_data();
    }

    char *end()
    {
      return get_data() + get_size();
    }

    [[nodiscard]] const char *begin() const
    {
      return get_data();
    }

    [[nodiscard]] const char *end() const
    {
      return get_data() + get_size();
    }

    [[nodiscard]] const_iterator cbegin() const noexcept
    {
      return begin();
    }

    [[nodiscard]] const_iterator cend() const noexcept
    {
      return end();
    }

    [[nodiscard]] reverse_iterator rbegin() noexcept
    {
      return reverse_iterator(end());
    }

    [[nodiscard]] reverse_iterator rend() noexcept
    {
      return reverse_iterator(begin());
    }

    [[nodiscard]] const_reverse_iterator rbegin() const noexcept
    {
      return const_reverse_iterator(end());
    }

    [[nodiscard]] const_reverse_iterator rend() const noexcept
    {
      return const_reverse_iterator(begin());
    }

    [[nodiscard]] const_reverse_iterator crbegin() const noexcept
    {
      return rbegin();
    }

    [[nodiscard]] const_reverse_iterator crend() const noexcept
    {
      return rend();
    }

    [[nodiscard]] char &back()
    {
#if !defined(NDEBUG)
      if (get_size() == 0)
        panic("Called back() on empty String");
#endif
      return get_data()[get_size() - 1];
    }

    [[nodiscard]] const char &back() const
    {
#if !defined(NDEBUG)
      if (get_size() == 0)
        panic("Called back() on empty String");
#endif
      return get_data()[get_size() - 1];
    }

    operator Span<char>()
    {
      return Span<char>(get_data(), get_size());
    }

    operator Span<const char>() const
    {
      return Span<const char>(get_data(), get_size());
    }

    operator StringView() const
    {
      return StringView(get_data(), get_size());
    }

    [[nodiscard]] Span<const u8> as_bytes() const
    {
      auto bytes = std::as_bytes(Span<const char>(get_data(), get_size()));
      return Span<const u8>(reinterpret_cast<const u8 *>(bytes.data()), bytes.size());
    }

    [[nodiscard]] usize find(char c, usize pos = 0) const
    {
      return StringView(get_data(), get_size()).find(c, pos);
    }

    [[nodiscard]] usize find(StringView v, usize pos = 0) const
    {
      return StringView(get_data(), get_size()).find(v, pos);
    }

    [[nodiscard]] usize find(const char *s, usize pos = 0) const
    {
      return StringView(get_data(), get_size()).find(StringView(s), pos);
    }

    auto operator+=(StringView other) -> void
    {
      append(other);
    }

public:
    void clear()
    {
      if (is_short())
        set_short_size(0);
      else
        m_storage.l.size = 0;

      get_data()[0] = '\0';
    }

    [[nodiscard]] StringView substr(usize pos, usize count = npos) const
    {
      return StringView(get_data(), get_size()).substr(pos, count);
    }

public:
    static String vformat(const char *fmt, va_list args)
    {
      String res;
      va_list args_copy;

      va_copy(args_copy, args);
      int req_len = vsnprintf(res.m_storage.s.data, SSO_CAPACITY + 1, fmt, args_copy);
      va_end(args_copy);

      if (req_len < 0)
        return res;

      usize len = static_cast<usize>(req_len);
      if (len <= SSO_CAPACITY)
      {
        res.set_short_size(static_cast<u8>(len));
        return res;
      }

      res.reserve(len);
      vsnprintf(res.get_data(), len + 1, fmt, args);
      res.m_storage.l.size = len;
      return res;
    }

    static String format(const char *fmt, ...)
    {
      va_list args;
      va_start(args, fmt);
      String res = vformat(fmt, args);
      va_end(args);
      return res;
    }
  };
} // namespace au::containers

export namespace au
{
  constexpr bool operator==(StringView lhs, const char *rhs)
  {
    return lhs == StringView(rhs);
  }

  constexpr bool operator==(const char *lhs, StringView rhs)
  {
    return StringView(lhs) == rhs;
  }
} // namespace au

export namespace au::containers
{
  inline bool operator==(const String &lhs, const String &rhs)
  {
    if (&lhs == &rhs)
      return true;
    if (lhs.size() != rhs.size())
      return false;
    return StringView(lhs.data(), lhs.size()) == StringView(rhs.data(), rhs.size());
  }

  inline bool operator==(const String &lhs, StringView rhs)
  {
    return StringView(lhs.data(), lhs.size()) == rhs;
  }

  inline bool operator==(StringView lhs, const String &rhs)
  {
    return lhs == StringView(rhs.data(), rhs.size());
  }

  inline bool operator==(const String &lhs, const char *rhs)
  {
    return StringView(lhs.data(), lhs.size()) == StringView(rhs);
  }

  inline bool operator==(const char *lhs, const String &rhs)
  {
    return StringView(lhs) == StringView(rhs.data(), rhs.size());
  }

  inline String operator+(const String &lhs, StringView rhs)
  {
    String result;
    result.reserve(lhs.size() + rhs.size());
    result.append(lhs);
    result.append(rhs);
    return result;
  }

  inline String operator+(StringView lhs, const String &rhs)
  {
    String result;
    result.reserve(lhs.size() + rhs.size());
    result.append(lhs);
    result.append(rhs);
    return result;
  }

  inline String operator+(const String &lhs, const String &rhs)
  {
    String result;
    result.reserve(lhs.size() + rhs.size());
    result.append(lhs);
    result.append(rhs);
    return result;
  }

  inline String operator+(const char *lhs, const String &rhs)
  {
    return StringView(lhs) + rhs;
  }

  inline String operator+(const String &lhs, const char *rhs)
  {
    return lhs + StringView(rhs);
  }

  inline String operator+(const String &lhs, char rhs)
  {
    String result;
    result.reserve(lhs.size() + 1);
    result.append(lhs);
    result.push(rhs);
    return result;
  }
} // namespace au::containers

export template<>
inline constexpr bool std::ranges::enable_borrowed_range<au::StringView> = true;

export namespace au
{
  using String = containers::String;
}

export namespace au::containers
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

    constexpr Option(const std::optional<T> &opt) : m_dummy(0), m_has_value(false)
    {
      if (opt.has_value())
      {
        au::construct_at(&m_val, *opt);
        m_has_value = true;
      }
    }

    constexpr Option(std::optional<T> &&opt) : m_dummy(0), m_has_value(false)
    {
      if (opt.has_value())
      {
        au::construct_at(&m_val, std::move(*opt));
        m_has_value = true;
      }
    }

    [[nodiscard]] constexpr operator std::optional<T>() const &
    {
      if (m_has_value)
        return std::optional<T>{m_val};
      return std::nullopt;
    }

    [[nodiscard]] constexpr operator std::optional<T>() &&
    {
      if (m_has_value)
        return std::optional<T>{std::move(m_val)};
      return std::nullopt;
    }

    constexpr Option(const Option &other)
    {
      if (other.m_has_value)
      {
        au::construct_at(&m_val, other.m_val);
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
        au::construct_at(&m_val, std::move(other.m_val));
        m_has_value = true;
      }
      else
      {
        m_has_value = false;
      }
    }

    constexpr ~Option()
      requires(!std::is_trivially_destructible_v<T>)
    {
      reset();
    }

    constexpr ~Option() = default;

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
            au::construct_at(&m_val, other.m_val);
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
            au::construct_at(&m_val, std::move(other.m_val));
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
          au::destroy_at(&m_val);
        }
        m_has_value = false;
      }
    }
  };
} // namespace au::containers

export namespace au
{
  template<typename T> using Option = containers::Option<T>;
}

export namespace au::containers
{
  template<typename T, typename IndexT, typename AllocatorT = memory::HeapAllocator>
    requires memory::AllocatorType<AllocatorT>
  class CompactVecBase
  {
public:
    using value_type             = T;
    using size_type              = IndexT;
    using difference_type        = std::make_signed_t<IndexT>;
    using reference              = T &;
    using const_reference        = const T &;
    using pointer                = T *;
    using const_pointer          = const T *;
    using iterator               = T *;
    using const_iterator         = const T *;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr CompactVecBase() noexcept = default;

    explicit CompactVecBase(size_type init_size, const T &init_value = T{})
    {
      resize(init_size, init_value);
    }

    CompactVecBase(std::initializer_list<T> init)
    {
      reserve_exact(static_cast<size_type>(init.size()));
      if constexpr (std::is_trivially_copyable_v<T>)
      {
        if (init.size() > 0)
          std::memcpy(m_data, init.begin(), init.size() * sizeof(T));
        m_size = static_cast<size_type>(init.size());
      }
      else
      {
        for (const auto &item : init)
          push_back(item);
      }
    }

    CompactVecBase(CompactVecBase &&other) noexcept
        : m_data(other.m_data),
          m_size(other.m_size),
          m_capacity(other.m_capacity),
          m_allocator(std::move(other.m_allocator))
    {
      other.m_data     = nullptr;
      other.m_size     = 0;
      other.m_capacity = 0;
    }

    auto operator=(CompactVecBase &&other) noexcept -> CompactVecBase &
    {
      if (this != &other)
      {
        clear();
        if (m_data)
          m_allocator.free(m_data, m_capacity * sizeof(T), alignof(T));
        m_allocator      = std::move(other.m_allocator);
        m_data           = other.m_data;
        m_size           = other.m_size;
        m_capacity       = other.m_capacity;
        other.m_data     = nullptr;
        other.m_size     = 0;
        other.m_capacity = 0;
      }
      return *this;
    }

    CompactVecBase(const CompactVecBase &other)
    {
      reserve_exact(other.m_size);
      if constexpr (std::is_trivially_copyable_v<T>)
      {
        if (other.m_size > 0)
          std::memcpy(m_data, other.m_data, other.m_size * sizeof(T));
        m_size = other.m_size;
      }
      else
      {
        for (size_type i = 0; i < other.m_size; ++i)
          au::construct_at(&m_data[i], other.m_data[i]);
        m_size = other.m_size;
      }
    }

    auto operator=(const CompactVecBase &other) -> CompactVecBase &
    {
      if (this != &other)
      {
        clear();
        reserve_exact(other.m_size);
        if constexpr (std::is_trivially_copyable_v<T>)
        {
          if (other.m_size > 0)
            std::memcpy(m_data, other.m_data, other.m_size * sizeof(T));
          m_size = other.m_size;
        }
        else
        {
          for (size_type i = 0; i < other.m_size; ++i)
            au::construct_at(&m_data[i], other.m_data[i]);
          m_size = other.m_size;
        }
      }
      return *this;
    }

    ~CompactVecBase()
    {
      clear();
      if (m_data)
        m_allocator.free(m_data, m_capacity * sizeof(T), alignof(T));
    }

    template<typename... Args> auto emplace_back(Args &&...args) -> T &
    {
      if (m_size >= m_capacity)
        grow();
      au::construct_at(&m_data[m_size], std::forward<Args>(args)...);
      return m_data[m_size++];
    }

    auto push_back(const T &val) -> void { emplace_back(val); }
    auto push_back(T &&val) -> void      { emplace_back(std::move(val)); }
    auto push(const T &val) -> void      { emplace_back(val); }
    auto push(T &&val) -> void           { emplace_back(std::move(val)); }

    auto pop_back() -> void
    {
      if (m_size > 0)
      {
        --m_size;
        if constexpr (!std::is_trivially_destructible_v<T>)
          au::destroy_at(&m_data[m_size]);
      }
    }
    auto pop() -> void { pop_back(); }

    auto clear() -> void
    {
      if constexpr (!std::is_trivially_destructible_v<T>)
      {
        for (size_type i = 0; i < m_size; ++i)
          au::destroy_at(&m_data[i]);
      }
      m_size = 0;
    }

    auto reserve_exact(size_type new_cap) -> void
    {
      if (new_cap <= m_capacity) return;
      reallocate(new_cap);
    }

    auto reserve_at_least(size_type new_cap) -> void
    {
      if (new_cap <= m_capacity) return;
      const size_type geom = (m_capacity == 0)
          ? static_cast<size_type>(8)
          : static_cast<size_type>(m_capacity + (m_capacity / 2) + 1);
      reallocate(geom > new_cap ? geom : new_cap);
    }

    auto reserve(size_type new_cap) -> void { reserve_at_least(new_cap); }

    auto resize(size_type new_size) -> void
    {
      if (new_size > m_size)
      {
        if (new_size > m_capacity)
          reserve_at_least(new_size);
        for (size_type i = m_size; i < new_size; ++i)
          au::construct_at(&m_data[i]);
      }
      else if (new_size < m_size)
      {
        if constexpr (!std::is_trivially_destructible_v<T>)
          for (size_type i = new_size; i < m_size; ++i)
            au::destroy_at(&m_data[i]);
      }
      m_size = new_size;
    }

    auto resize(size_type new_size, const T &fill_val) -> void
    {
      if (new_size > m_size)
      {
        if (new_size > m_capacity)
          reserve_at_least(new_size);
        for (size_type i = m_size; i < new_size; ++i)
          au::construct_at(&m_data[i], fill_val);
      }
      else if (new_size < m_size)
      {
        if constexpr (!std::is_trivially_destructible_v<T>)
          for (size_type i = new_size; i < m_size; ++i)
            au::destroy_at(&m_data[i]);
      }
      m_size = new_size;
    }

    [[nodiscard]] auto size() const noexcept     -> size_type { return m_size; }
    [[nodiscard]] auto capacity() const noexcept -> size_type { return m_capacity; }
    [[nodiscard]] auto empty() const noexcept    -> bool      { return m_size == 0; }

    [[nodiscard]] auto data() noexcept           -> T *       { return m_data; }
    [[nodiscard]] auto data() const noexcept     -> const T * { return m_data; }

    auto operator[](size_type idx) -> T &
    {
#if !defined(NDEBUG)
      if (idx >= m_size) panic("CompactVecBase index out of bounds");
#endif
      return m_data[idx];
    }

    auto operator[](size_type idx) const -> const T &
    {
#if !defined(NDEBUG)
      if (idx >= m_size) panic("CompactVecBase index out of bounds");
#endif
      return m_data[idx];
    }

    auto begin() noexcept  -> T *       { return m_data; }
    auto end() noexcept    -> T *       { return m_data + m_size; }
    auto begin() const noexcept -> const T * { return m_data; }
    auto end() const noexcept   -> const T * { return m_data + m_size; }
    auto cbegin() const noexcept -> const_iterator { return begin(); }
    auto cend()   const noexcept -> const_iterator { return end(); }

    auto back() -> T &             { return m_data[m_size - 1]; }
    auto back() const -> const T & { return m_data[m_size - 1]; }

    operator std::span<T>()       { return std::span<T>(m_data, m_size); }
    operator std::span<const T>() const { return std::span<const T>(m_data, m_size); }

    [[nodiscard]] auto as_span() -> std::span<T> { return std::span<T>(m_data, m_size); }
    [[nodiscard]] auto as_span() const -> std::span<const T> { return std::span<const T>(m_data, m_size); }

private:
    auto reallocate(size_type new_cap) -> void
    {
      if constexpr (std::is_trivially_copyable_v<T>)
      {
        if (m_data)
        {
          void *ptr = m_allocator.realloc(m_data, m_capacity * sizeof(T), new_cap * sizeof(T), alignof(T));
          m_data    = static_cast<T *>(ptr);
          m_capacity = new_cap;
          return;
        }
      }
      T *new_data = static_cast<T *>(m_allocator.alloc(new_cap * sizeof(T), alignof(T)));
      if (m_data)
      {
        if constexpr (std::is_trivially_copyable_v<T>)
        {
          std::memcpy(new_data, m_data, m_size * sizeof(T));
        }
        else
        {
          for (size_type i = 0; i < m_size; ++i)
          {
            au::construct_at(&new_data[i], std::move(m_data[i]));
            au::destroy_at(&m_data[i]);
          }
        }
        m_allocator.free(m_data, m_capacity * sizeof(T), alignof(T));
      }
      m_data     = new_data;
      m_capacity = new_cap;
    }

    auto grow() -> void
    {
      reserve_at_least(static_cast<size_type>(m_size + 1));
    }

    T                                 *m_data      = nullptr;
    size_type                          m_size      = 0;
    size_type                          m_capacity  = 0;
    AUXID_NO_UNIQUE_ADDRESS AllocatorT m_allocator{};
  };

  template<typename T, typename IndexT = u32, typename AllocatorT = memory::HeapAllocator>
  using VecT = CompactVecBase<T, IndexT, AllocatorT>;
} // namespace au::containers

export namespace au
{
  template<typename T, memory::AllocatorType A = memory::HeapAllocator>
  using Vec = std::vector<T, memory::StdAllocatorAdapter<T, A>>;

  template<typename T, memory::AllocatorType A = memory::HeapAllocator>
  using TinyVec = containers::CompactVecBase<T, u16, A>;

  template<typename T, memory::AllocatorType A = memory::HeapAllocator>
  using CompactVec = containers::CompactVecBase<T, u32, A>;
} // namespace au

export namespace au
{
  template<typename T, typename A>
  [[nodiscard]] inline auto clone(const std::vector<T, A> &v) -> std::vector<T, A>
  {
    return v;
  }

  template<typename T, typename I, typename A>
  [[nodiscard]] inline auto clone(const containers::CompactVecBase<T, I, A> &v)
      -> containers::CompactVecBase<T, I, A>
  {
    return v;
  }

  template<typename T, typename A>
  inline auto reserve_exact(std::vector<T, A> &v, typename std::vector<T, A>::size_type n) -> void
  {
    v.reserve(n);
  }

  template<typename T, typename A>
  inline auto reserve_at_least(std::vector<T, A> &v, typename std::vector<T, A>::size_type n) -> void
  {
    if (n <= v.capacity()) return;
    const auto geom = v.capacity() + (v.capacity() / 2) + 1;
    v.reserve(geom > n ? geom : n);
  }

  template<typename T, typename I, typename A>
  inline auto reserve_exact(containers::CompactVecBase<T, I, A> &v, I n) -> void
  {
    v.reserve_exact(n);
  }

  template<typename T, typename I, typename A>
  inline auto reserve_at_least(containers::CompactVecBase<T, I, A> &v, I n) -> void
  {
    v.reserve_at_least(n);
  }

  template<typename T, typename A> inline auto push(std::vector<T, A> &v, T &&x) -> void
  {
    v.push_back(std::forward<T>(x));
  }
  template<typename T, typename A> inline auto push(std::vector<T, A> &v, const T &x) -> void
  {
    v.push_back(x);
  }
  template<typename T, typename A> inline auto pop(std::vector<T, A> &v) -> void
  {
    if (!v.empty()) v.pop_back();
  }

  template<typename T, typename I, typename A> inline auto push(containers::CompactVecBase<T, I, A> &v, T &&x) -> void
  {
    v.push(std::forward<T>(x));
  }
  template<typename T, typename I, typename A> inline auto push(containers::CompactVecBase<T, I, A> &v, const T &x) -> void
  {
    v.push(x);
  }
  template<typename T, typename I, typename A> inline auto pop(containers::CompactVecBase<T, I, A> &v) -> void
  {
    v.pop();
  }

  template<typename T, typename A> [[nodiscard]] inline auto as_span(std::vector<T, A> &v) noexcept -> std::span<T>
  {
    return std::span<T>(v);
  }
  template<typename T, typename A> [[nodiscard]] inline auto as_span(const std::vector<T, A> &v) noexcept -> std::span<const T>
  {
    return std::span<const T>(v);
  }
} // namespace au

export namespace au::containers
{
  inline constexpr u32 INDEX_INVALID = UINT32_MAX;

  [[nodiscard]] u64 hash_bytes(const void *data, usize len, u64 seed = 0) noexcept
  {
    return ::wyhash(data, len, seed, ::_wyp);
  }

  template<typename T> struct Hash;

  template<typename T>
    requires std::has_unique_object_representations_v<T> && (!std::is_pointer_v<T>)
  struct Hash<T>
  {
    [[nodiscard]] u64 operator()(const T &val) const noexcept
    {
      return hash_bytes(&val, sizeof(T));
    }
  };

  namespace detail
  {
    [[nodiscard]] inline constexpr u64 int_mix(u64 x) noexcept
    {
      auto t = x * 11400714819323198485ULL;
      t ^= (t >> 32);
      return t;
    }
  } // namespace detail

#define AU_DEFINE_INT_HASH(T)                                                                                          \
  template<> struct Hash<T>                                                                                            \
  {                                                                                                                    \
    [[nodiscard]] u64 operator()(T x) const noexcept                                                                   \
    {                                                                                                                  \
      return detail::int_mix(static_cast<u64>(x));                                                                     \
    }                                                                                                                  \
  }

  AU_DEFINE_INT_HASH(bool);
  AU_DEFINE_INT_HASH(char);
  AU_DEFINE_INT_HASH(i8);
  AU_DEFINE_INT_HASH(u8);
  AU_DEFINE_INT_HASH(i16);
  AU_DEFINE_INT_HASH(u16);
  AU_DEFINE_INT_HASH(i32);
  AU_DEFINE_INT_HASH(u32);
  AU_DEFINE_INT_HASH(i64);
  AU_DEFINE_INT_HASH(u64);

#undef AU_DEFINE_INT_HASH

  template<typename T> struct Hash<T *>
  {
    [[nodiscard]] u64 operator()(const T *p) const noexcept
    {
      return detail::int_mix(reinterpret_cast<uintptr_t>(p));
    }
  };

  template<> struct Hash<String>
  {
    using is_transparent = void;

    [[nodiscard]] u64 operator()(StringView sv) const noexcept
    {
      return hash_string_view(sv);
    }

    [[nodiscard]] u64 operator()(const char *s) const noexcept
    {
      return hash_string_view(StringView(s));
    }
  };

  template<> struct Hash<StringView>
  {
    using is_transparent = void;

    [[nodiscard]] u64 operator()(StringView sv) const noexcept
    {
      return hash_string_view(sv);
    }

    [[nodiscard]] u64 operator()(const char *s) const noexcept
    {
      return hash_string_view(StringView(s));
    }
  };

  template<> struct Hash<const char *>
  {
    [[nodiscard]] u64 operator()(const char *s) const noexcept
    {
      return hash_string_view(StringView(s));
    }
  };

  template<typename T> struct EqualTo
  {
    constexpr bool operator()(const T &lhs, const T &rhs) const
    {
      return lhs == rhs;
    }
  };

  template<> struct EqualTo<String>
  {
    using is_transparent = void;

    constexpr bool operator()(StringView lhs, StringView rhs) const noexcept
    {
      return lhs == rhs;
    }
  };

  template<> struct EqualTo<StringView>
  {
    using is_transparent = void;

    constexpr bool operator()(StringView lhs, StringView rhs) const noexcept
    {
      return lhs == rhs;
    }
  };

  [[nodiscard]] inline u64 hash_combine(u64 seed, u64 value) noexcept
  {
    return seed ^ (detail::int_mix(value) + 0x9E3779B97F4A7C15ULL + (seed << 6) + (seed >> 2));
  }

  template<typename It>
  [[nodiscard]] inline u64 hash_combine_range(u64 seed, It first, It last) noexcept
  {
    using value_type = std::remove_cvref_t<decltype(*first)>;
    Hash<value_type> hasher{};
    for (; first != last; ++first)
      seed = hash_combine(seed, hasher(*first));
    return seed;
  }

  template<typename K, typename V> struct Hash<Pair<K, V>>
  {
    [[nodiscard]] u64 operator()(const Pair<K, V> &p) const noexcept
    {
      const u64 h1 = Hash<K>{}(p.first);
      const u64 h2 = Hash<V>{}(p.second);
      return hash_combine(h1, h2);
    }
  };

  template<typename... Ts> struct Hash<std::tuple<Ts...>>
  {
    [[nodiscard]] u64 operator()(const std::tuple<Ts...> &t) const noexcept
    {
      return hash_impl(t, std::index_sequence_for<Ts...>{});
    }

private:
    template<usize... Is>
    [[nodiscard]] static u64 hash_impl(const std::tuple<Ts...> &t, std::index_sequence<Is...>) noexcept
    {
      if constexpr (sizeof...(Ts) == 0)
        return 0;
      else
      {
        u64 seed = 0;
        ((seed = hash_combine(seed, Hash<Ts>{}(std::get<Is>(t)))), ...);
        return seed;
      }
    }
  };

  template<> struct Hash<Span<const u8>>
  {
    [[nodiscard]] u64 operator()(Span<const u8> bytes) const noexcept
    {
      return hash_bytes(bytes.data(), bytes.size());
    }
  };
} // namespace au::containers

export namespace au::containers
{
  struct PairFirstKeyOf
  {
    template<class P>[[nodiscard]] constexpr auto operator()(const P &p) const noexcept -> const auto &
    {
      return p.first;
    }
  };

  struct IdentityKeyOf
  {
    template<class T>[[nodiscard]] constexpr auto operator()(const T &v) const noexcept -> const T &
    {
      return v;
    }
  };

  template<class Hasher, class KeyEq, class Key, class K2>
  concept TransparentLookup = requires { typename Hasher::is_transparent; } && requires {
    typename KeyEq::is_transparent;
  } && requires(const Hasher &h, const KeyEq &e, const K2 &k2, const Key &k) {
    { h(k2) } -> std::convertible_to<u64>;
    { e(k2, k) } -> std::convertible_to<bool>;
  };

  template<class Entry, class Key, class KeyOf, class Hasher = Hash<Key>, class KeyEq = EqualTo<Key>,
           class AllocatorT = memory::HeapAllocator>
    requires memory::AllocatorType<AllocatorT>
  class HashTable
  {
public:
    using entry_type             = Entry;
    using key_type               = Key;
    using size_type              = usize;
    using difference_type        = isize;
    using reference              = Entry &;
    using const_reference        = const Entry &;
    using pointer                = Entry *;
    using const_pointer          = const Entry *;
    using iterator               = Entry *;
    using const_iterator         = const Entry *;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    VecT<Entry, usize, AllocatorT> m_entries;
    VecT<u32, usize, AllocatorT>   m_buckets;
    size_type                      m_mask = 0;

    AUXID_NO_UNIQUE_ADDRESS Hasher m_hasher{};
    AUXID_NO_UNIQUE_ADDRESS KeyEq  m_eq{};
    AUXID_NO_UNIQUE_ADDRESS KeyOf  m_key_of{};

public:
    constexpr HashTable() = default;

    explicit HashTable(size_type cap)
    {
      reserve(cap);
    }

    void reserve_at_least(size_type new_cap)
    {
      if (new_cap <= m_entries.capacity())
        return;
      m_entries.reserve(new_cap);

      size_type buckets_cap = 8;
      while (buckets_cap < new_cap * 2)
        buckets_cap *= 2;
      rehash_buckets(buckets_cap);
    }

    void reserve_exact(size_type new_cap) { reserve_at_least(new_cap); }
    void reserve(size_type new_cap)       { reserve_at_least(new_cap); }

    void clear()
    {
      m_entries.clear();
      if (!m_buckets.empty())
      {
        std::fill(m_buckets.begin(), m_buckets.end(), INDEX_INVALID);
      }
    }

    bool try_insert(Entry &&entry)
    {
      const Key &key = m_key_of(entry);
      if (contains(key))
        return false;

      if (should_grow())
        grow();

      const u32 entry_idx = static_cast<u32>(m_entries.size());
      m_entries.push(std::move(entry));
      insert_into_buckets(entry_idx, m_key_of(m_entries[entry_idx]));
      return true;
    }

    bool try_insert(const Entry &entry)
    {
      Entry copy = entry;
      return try_insert(std::move(copy));
    }

    template<class Factory> Entry &find_or_emplace(const Key &key, Factory &&factory)
    {
      if (should_grow())
        grow();

      auto h   = hash_key(key);
      auto idx = h & m_mask;

      while (true)
      {
        u32 entry_idx = m_buckets[idx];

        if (entry_idx == INDEX_INVALID)
        {
          m_buckets[idx] = static_cast<u32>(m_entries.size());
          m_entries.push(static_cast<Factory &&>(factory)());
          return m_entries[m_entries.size() - 1];
        }

        if (m_eq(m_key_of(m_entries[entry_idx]), key))
        {
          return m_entries[entry_idx];
        }

        idx = (idx + 1) & m_mask;
      }
    }

    [[nodiscard]] Entry *find_entry(const Key &key) { return find_entry_impl(key); }
    [[nodiscard]] const Entry *find_entry(const Key &key) const { return find_entry_impl(key); }

    template<class K2>
      requires TransparentLookup<Hasher, KeyEq, Key, K2>
    [[nodiscard]] Entry *find_entry(const K2 &key)
    {
      return find_entry_impl(key);
    }

    template<class K2>
      requires TransparentLookup<Hasher, KeyEq, Key, K2>
    [[nodiscard]] const Entry *find_entry(const K2 &key) const
    {
      return find_entry_impl(key);
    }

    [[nodiscard]] bool contains(const Key &key) const { return find_entry(key) != nullptr; }

    template<class K2>
      requires TransparentLookup<Hasher, KeyEq, Key, K2>
    [[nodiscard]] bool contains(const K2 &key) const
    {
      return find_entry(key) != nullptr;
    }

    bool erase(const Key &key) { return erase_impl(key); }

    template<class K2>
      requires TransparentLookup<Hasher, KeyEq, Key, K2>
    bool erase(const K2 &key)
    {
      return erase_impl(key);
    }

    [[nodiscard]] iterator       begin() noexcept       { return m_entries.begin(); }
    [[nodiscard]] iterator       end()   noexcept       { return m_entries.end(); }
    [[nodiscard]] const_iterator begin() const noexcept { return m_entries.begin(); }
    [[nodiscard]] const_iterator end()   const noexcept { return m_entries.end(); }
    [[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }
    [[nodiscard]] const_iterator cend()   const noexcept { return end(); }

    [[nodiscard]] reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    [[nodiscard]] reverse_iterator rend()   noexcept { return reverse_iterator(begin()); }
    [[nodiscard]] const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    [[nodiscard]] const_reverse_iterator rend()   const noexcept { return const_reverse_iterator(begin()); }
    [[nodiscard]] const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    [[nodiscard]] const_reverse_iterator crend()   const noexcept { return rend(); }

    [[nodiscard]] size_type size()  const noexcept { return m_entries.size(); }
    [[nodiscard]] bool      empty() const noexcept { return m_entries.empty(); }

private:
    template<class K> [[nodiscard]] u64 hash_key(const K &key) const noexcept
    {
      return m_hasher(key);
    }

    [[nodiscard]] bool should_grow() const noexcept
    {
      return m_entries.size() * 10 >= m_buckets.size() * 8 || m_buckets.empty();
    }

    void grow()
    {
      size_type new_cap = m_buckets.empty() ? 16 : m_buckets.size() * 2;
      rehash_buckets(new_cap);
    }

    void rehash_buckets(size_type new_cap)
    {
      m_buckets.clear();
      m_buckets.reserve(new_cap);
      m_buckets.resize(new_cap, INDEX_INVALID);
      m_mask = new_cap - 1;

      for (u32 i = 0; i < m_entries.size(); ++i)
      {
        insert_into_buckets(i, m_key_of(m_entries[i]));
      }
    }

    template<class K> void insert_into_buckets(u32 entry_idx, const K &key)
    {
      auto h   = hash_key(key);
      auto idx = h & m_mask;

      while (m_buckets[idx] != INDEX_INVALID)
        idx = (idx + 1) & m_mask;

      m_buckets[idx] = entry_idx;
    }

    template<class K> [[nodiscard]] Entry *find_entry_impl(const K &key) const
    {
      if (m_buckets.empty())
        return nullptr;

      auto h    = hash_key(key);
      auto idx  = h & m_mask;
      auto dist = static_cast<size_type>(0);

      while (true)
      {
        u32 entry_idx = m_buckets[idx];

        if (entry_idx == INDEX_INVALID)
          return nullptr;

        if (m_eq(m_key_of(m_entries[entry_idx]), key))
        {
          return const_cast<Entry *>(&m_entries[entry_idx]);
        }

        ++dist;
        idx = (idx + 1) & m_mask;

        if (dist > m_mask)
          return nullptr;
      }
    }

    template<class K> bool erase_impl(const K &key)
    {
      if (m_buckets.empty())
        return false;

      auto h   = hash_key(key);
      auto idx = h & m_mask;

      while (true)
      {
        u32 entry_idx = m_buckets[idx];

        if (entry_idx == INDEX_INVALID)
          return false;

        if (m_eq(m_key_of(m_entries[entry_idx]), key))
        {
          remove_at_bucket(static_cast<u32>(idx), entry_idx);
          return true;
        }

        idx = (idx + 1) & m_mask;
      }
    }

    void remove_at_bucket(u32 bucket_idx, u32 entry_idx_to_remove)
    {
      backward_shift(bucket_idx);

      const u32 last_idx = static_cast<u32>(m_entries.size() - 1);

      if (entry_idx_to_remove != last_idx)
      {
        m_entries[entry_idx_to_remove] = std::move(m_entries[last_idx]);
        update_bucket_pointer(m_key_of(m_entries[entry_idx_to_remove]), last_idx, entry_idx_to_remove);
      }

      m_entries.pop();
    }

    void backward_shift(u32 hole_idx)
    {
      u32 next = (hole_idx + 1) & m_mask;

      while (true)
      {
        u32 entry_idx = m_buckets[next];

        if (entry_idx == INDEX_INVALID)
          break;

        auto h         = hash_key(m_key_of(m_entries[entry_idx]));
        auto ideal_idx = h & m_mask;

        auto dist_current = (static_cast<size_type>(next) - ideal_idx) & m_mask;
        auto dist_hole    = (static_cast<size_type>(hole_idx) - ideal_idx) & m_mask;

        if (dist_hole < dist_current)
        {
          m_buckets[hole_idx] = entry_idx;
          hole_idx            = next;
        }

        next = (next + 1) & m_mask;
      }

      m_buckets[hole_idx] = INDEX_INVALID;
    }

    template<class K> void update_bucket_pointer(const K &key, u32 old_idx, u32 new_idx)
    {
      auto h   = hash_key(key);
      auto idx = h & m_mask;

      while (true)
      {
        if (m_buckets[idx] == old_idx)
        {
          m_buckets[idx] = new_idx;
          return;
        }
        idx = (idx + 1) & m_mask;
      }
    }
  };
} // namespace au::containers

export namespace au::containers
{
  template<typename K, typename V, typename Hasher = Hash<K>, typename KeyEq = EqualTo<K>,
           typename AllocatorT = memory::HeapAllocator>
    requires memory::AllocatorType<AllocatorT>
  class HashMap
  {
public:
    using value_type             = Pair<K, V>;
    using key_type               = K;
    using mapped_type            = V;
    using size_type              = usize;
    using difference_type        = isize;
    using reference              = value_type &;
    using const_reference        = const value_type &;
    using pointer                = value_type *;
    using const_pointer          = const value_type *;
    using iterator               = value_type *;
    using const_iterator         = const value_type *;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    using Table = HashTable<value_type, K, PairFirstKeyOf, Hasher, KeyEq, AllocatorT>;
    Table m_table;

public:
    HashMap() = default;
    explicit HashMap(size_type cap) : m_table(cap) {}

    void reserve_at_least(size_type new_cap) { m_table.reserve_at_least(new_cap); }
    void reserve_exact(size_type new_cap)    { m_table.reserve_exact(new_cap); }
    void reserve(size_type new_cap)          { m_table.reserve(new_cap); }
    void clear()                             { m_table.clear(); }

    V &operator[](const K &key)
    {
      return m_table.find_or_emplace(key, [&key]() { return value_type{key, V{}}; }).second;
    }

    bool insert(const K &key, const V &val)
    {
      return m_table.try_insert(value_type{key, val});
    }

    bool insert(const K &key, V &&val)
    {
      return m_table.try_insert(value_type{key, std::move(val)});
    }

    [[nodiscard]] V *find(const K &key)
    {
      auto *entry = m_table.find_entry(key);
      return entry ? &entry->second : nullptr;
    }

    [[nodiscard]] const V *find(const K &key) const
    {
      auto *entry = m_table.find_entry(key);
      return entry ? &entry->second : nullptr;
    }

    template<class K2>
      requires TransparentLookup<Hasher, KeyEq, K, K2>
    [[nodiscard]] V *find(const K2 &key)
    {
      auto *entry = m_table.find_entry(key);
      return entry ? &entry->second : nullptr;
    }

    template<class K2>
      requires TransparentLookup<Hasher, KeyEq, K, K2>
    [[nodiscard]] const V *find(const K2 &key) const
    {
      auto *entry = m_table.find_entry(key);
      return entry ? &entry->second : nullptr;
    }

    [[nodiscard]] bool contains(const K &key) const { return m_table.contains(key); }

    template<class K2>
      requires TransparentLookup<Hasher, KeyEq, K, K2>
    [[nodiscard]] bool contains(const K2 &key) const
    {
      return m_table.contains(key);
    }

    bool erase(const K &key) { return m_table.erase(key); }

    template<class K2>
      requires TransparentLookup<Hasher, KeyEq, K, K2>
    bool erase(const K2 &key)
    {
      return m_table.erase(key);
    }

    [[nodiscard]] iterator       begin() noexcept       { return m_table.begin(); }
    [[nodiscard]] iterator       end()   noexcept       { return m_table.end(); }
    [[nodiscard]] const_iterator begin() const noexcept { return m_table.begin(); }
    [[nodiscard]] const_iterator end()   const noexcept { return m_table.end(); }
    [[nodiscard]] const_iterator cbegin() const noexcept { return m_table.cbegin(); }
    [[nodiscard]] const_iterator cend()   const noexcept { return m_table.cend(); }

    [[nodiscard]] reverse_iterator rbegin() noexcept { return m_table.rbegin(); }
    [[nodiscard]] reverse_iterator rend()   noexcept { return m_table.rend(); }
    [[nodiscard]] const_reverse_iterator rbegin() const noexcept { return m_table.rbegin(); }
    [[nodiscard]] const_reverse_iterator rend()   const noexcept { return m_table.rend(); }
    [[nodiscard]] const_reverse_iterator crbegin() const noexcept { return m_table.crbegin(); }
    [[nodiscard]] const_reverse_iterator crend()   const noexcept { return m_table.crend(); }

    [[nodiscard]] size_type size()  const noexcept { return m_table.size(); }
    [[nodiscard]] bool      empty() const noexcept { return m_table.empty(); }
  };

  template<typename K, typename Hasher = Hash<K>, typename KeyEq = EqualTo<K>,
           typename AllocatorT = memory::HeapAllocator>
    requires memory::AllocatorType<AllocatorT>
  class HashSet
  {
public:
    using value_type             = K;
    using key_type               = K;
    using size_type              = usize;
    using difference_type        = isize;
    using reference              = value_type &;
    using const_reference        = const value_type &;
    using pointer                = value_type *;
    using const_pointer          = const value_type *;
    using iterator               = value_type *;
    using const_iterator         = const value_type *;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    using Table = HashTable<K, K, IdentityKeyOf, Hasher, KeyEq, AllocatorT>;
    Table m_table;

public:
    HashSet() = default;
    explicit HashSet(size_type cap) : m_table(cap) {}

    void reserve_at_least(size_type new_cap) { m_table.reserve_at_least(new_cap); }
    void reserve_exact(size_type new_cap)    { m_table.reserve_exact(new_cap); }
    void reserve(size_type new_cap)          { m_table.reserve(new_cap); }
    void clear()                             { m_table.clear(); }

    bool insert(const K &key)             { return m_table.try_insert(K{key}); }
    bool insert(K &&key)                  { return m_table.try_insert(std::move(key)); }

    [[nodiscard]] bool contains(const K &key) const { return m_table.contains(key); }

    template<class K2>
      requires TransparentLookup<Hasher, KeyEq, K, K2>
    [[nodiscard]] bool contains(const K2 &key) const
    {
      return m_table.contains(key);
    }

    bool erase(const K &key) { return m_table.erase(key); }

    template<class K2>
      requires TransparentLookup<Hasher, KeyEq, K, K2>
    bool erase(const K2 &key)
    {
      return m_table.erase(key);
    }

    [[nodiscard]] iterator       begin() noexcept       { return m_table.begin(); }
    [[nodiscard]] iterator       end()   noexcept       { return m_table.end(); }
    [[nodiscard]] const_iterator begin() const noexcept { return m_table.begin(); }
    [[nodiscard]] const_iterator end()   const noexcept { return m_table.end(); }
    [[nodiscard]] const_iterator cbegin() const noexcept { return m_table.cbegin(); }
    [[nodiscard]] const_iterator cend()   const noexcept { return m_table.cend(); }

    [[nodiscard]] reverse_iterator rbegin() noexcept { return m_table.rbegin(); }
    [[nodiscard]] reverse_iterator rend()   noexcept { return m_table.rend(); }
    [[nodiscard]] const_reverse_iterator rbegin() const noexcept { return m_table.rbegin(); }
    [[nodiscard]] const_reverse_iterator rend()   const noexcept { return m_table.rend(); }
    [[nodiscard]] const_reverse_iterator crbegin() const noexcept { return m_table.crbegin(); }
    [[nodiscard]] const_reverse_iterator crend()   const noexcept { return m_table.crend(); }

    [[nodiscard]] size_type size()  const noexcept { return m_table.size(); }
    [[nodiscard]] bool      empty() const noexcept { return m_table.empty(); }
  };
} // namespace au::containers

export namespace au
{
  template<typename K, typename V> using HashMap = containers::HashMap<K, V>;
  template<typename T>             using HashSet = containers::HashSet<T>;
} // namespace au

export namespace au::containers
{
  template<typename T, usize Capacity>
    requires((Capacity != 0) && ((Capacity & (Capacity - 1)) == 0))
  class SpscQueue
  {
public:
    SpscQueue() = default;

    ~SpscQueue()
    {
      T dummy;
      while (pop(dummy))
      {
      }
    }

    SpscQueue(const SpscQueue &) = delete;
    SpscQueue &operator=(const SpscQueue &) = delete;

    [[nodiscard]] bool push(T value)
    {
      const usize write_idx = m_write_pos.load(std::memory_order_relaxed);

      const usize read_idx = m_read_pos.load(std::memory_order_acquire);

      if (write_idx - read_idx == Capacity)
        return false;

      T *slot = reinterpret_cast<T *>(&m_slots[(write_idx & K_MASK) * sizeof(T)]);

      new (slot) T(std::move(value));

      m_write_pos.store(write_idx + 1, std::memory_order_release);
      return true;
    }

    [[nodiscard]] bool pop(T &out_value)
    {
      const usize read_idx = m_read_pos.load(std::memory_order_relaxed);

      const usize write_idx = m_write_pos.load(std::memory_order_acquire);

      if (read_idx == write_idx)
        return false;

      T *slot = reinterpret_cast<T *>(&m_slots[(read_idx & K_MASK) * sizeof(T)]);

      out_value = std::move(*slot);

      slot->~T();

      m_read_pos.store(read_idx + 1, std::memory_order_release);
      return true;
    }

private:
    static constexpr usize K_MASK = Capacity - 1;

    alignas(64) std::atomic<usize> m_write_pos{0};
    alignas(64) std::atomic<usize> m_read_pos{0};

    alignas(T) u8 m_slots[Capacity * sizeof(T)];
  };
} // namespace au::containers

export namespace au
{
  template<typename T> using Result = ResultT<T, String>;

  template<typename... Args> [[nodiscard]] inline auto fail(const char *fmt, Args &&...args)
  {
    return fail(String::format(fmt, std::forward<Args>(args)...));
  }
} // namespace au

export namespace au::containers
{
  template<typename T> using Result = ::au::ResultT<T, String>;
}
