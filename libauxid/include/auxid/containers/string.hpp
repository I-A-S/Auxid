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

#include <auxid/containers/span.hpp>
#include <auxid/memory/heap.hpp>

#include <cstdarg>
#include <cstring>

namespace au
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
      return __builtin_memcmp(s1, s2, n);
    }

    inline usize length(const char *s)
    {
      return __builtin_strlen(s);
    }

    inline const char *find(const char *p, usize n, char c)
    {
      return static_cast<const char *>(__builtin_memchr(p, c, n));
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
  };

  inline u64 hash_string_view(StringView sv)
  {
    u64 hash = 14695981039346656037ULL;
    for (usize i = 0; i < sv.size(); ++i)
    {
      hash ^= static_cast<u8>(sv[i]);
      hash *= 1099511628211ULL;
    }
    return hash;
  }
} // namespace au

namespace au::containers
{
  struct String
  {
    static constexpr usize npos = StringView::npos;

    static constexpr usize SSO_CAPACITY = sizeof(usize) * 3 - 1;

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

    [[no_unique_address]] memory::HeapAllocator m_allocator;

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
      push(c);
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

    [[nodiscard]] char &back()
    {
      return get_data()[get_size() - 1];
    }

    [[nodiscard]] const char &back() const
    {
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
      return Span<const char>(get_data(), get_size()).as_bytes();
    }

    [[nodiscard]] usize find(char c, usize pos = 0) const
    {
      return StringView(*this).find(c, pos);
    }

    [[nodiscard]] usize find(StringView v, usize pos = 0) const
    {
      return StringView(*this).find(v, pos);
    }

    [[nodiscard]] usize find(const char *s, usize pos = 0) const
    {
      return StringView(*this).find(StringView(s), pos);
    }

    auto operator+=(StringView other) -> void
    {
      append(other);
    }

public:
    static String format(const char *fmt, ...)
    {
      String res;
      va_list args;

      va_start(args, fmt);
      int len_i = vsnprintf(nullptr, 0, fmt, args);
      va_end(args);

      if (len_i < 0)
        return res;

      usize len = static_cast<usize>(len_i);

      if (len > res.get_capacity())
        res.reserve(len);

      if (res.is_short())
        res.set_short_size(static_cast<u8>(len));
      else
        res.m_storage.l.size = len;

      va_start(args, fmt);
      vsnprintf(res.get_data(), len + 1, fmt, args);
      va_end(args);

      return res;
    }
  };
} // namespace au::containers

namespace au
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

namespace au::containers
{
  inline bool operator==(const String &lhs, const String &rhs)
  {
    if (&lhs == &rhs)
      return true;
    if (lhs.size() != rhs.size())
      return false;
    return StringView(lhs) == StringView(rhs);
  }

  inline bool operator==(const String &lhs, StringView rhs)
  {
    return StringView(lhs) == rhs;
  }

  inline bool operator==(StringView lhs, const String &rhs)
  {
    return lhs == StringView(rhs);
  }

  inline bool operator==(const String &lhs, const char *rhs)
  {
    return StringView(lhs) == StringView(rhs);
  }

  inline bool operator==(const char *lhs, const String &rhs)
  {
    return StringView(lhs) == StringView(rhs);
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

namespace au
{
  using String = containers::String;
}