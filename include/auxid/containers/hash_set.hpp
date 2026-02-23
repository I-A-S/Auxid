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

#include <auxid/containers/hash_base.hpp>
#include <auxid/containers/vec.hpp>

namespace au::containers
{
  template<typename K, typename Hasher = Hash<K>, typename KeyEq = EqualTo<K>,
           typename AllocatorT = memory::HeapAllocator>
    requires memory::AllocatorType<AllocatorT>
  class HashSet
  {
public:
    using value_type = K;
    using size_type = usize;

private:
    VecT<value_type, usize, AllocatorT> m_entries;
    VecT<u32, usize, AllocatorT> m_buckets;

    size_type m_mask = 0;
    size_type m_max_probe_dist = 0;

    Hasher m_hasher;
    KeyEq m_eq;

public:
    explicit HashSet()
    {
    }

    HashSet(size_type cap)
    {
      reserve(cap);
    }

public:
    void reserve(size_type new_cap)
    {
      if (new_cap <= m_entries.capacity())
        return;
      m_entries.reserve(new_cap);

      size_type buckets_cap = 8;
      while (buckets_cap < new_cap * 2)
        buckets_cap *= 2;
      rehash_buckets(buckets_cap);
    }

    void clear()
    {
      m_entries.clear();
      if (!m_buckets.empty())
      {
        std::fill(m_buckets.begin(), m_buckets.end(), INDEX_INVALID);
      }
    }

    bool insert(const K &key)
    {
      if (contains(key))
        return false;

      if (should_grow())
        grow();

      u32 entry_idx = static_cast<u32>(m_entries.size());
      m_entries.push(value_type{key});

      insert_into_buckets(entry_idx, key);
      return true;
    }

    bool contains(const K &key)
    {
      if (m_buckets.empty())
        return false;

      auto h = hash_key(key);
      auto idx = h & m_mask;
      auto dist = 0;

      while (true)
      {
        u32 entry_idx = m_buckets[idx];

        if (entry_idx == INDEX_INVALID)
          return false;

        if (m_eq(m_entries[entry_idx], key))
        {
          return true;
        }

        dist++;
        idx = (idx + 1) & m_mask;

        if (dist > (i32) m_mask)
          return false;
      }
    }

    bool erase(const K &key)
    {
      if (m_buckets.empty())
        return false;

      auto h = hash_key(key);
      auto idx = h & m_mask;

      while (true)
      {
        u32 entry_idx = m_buckets[idx];

        if (entry_idx == INDEX_INVALID)
          return false;

        if (m_eq(m_entries[entry_idx], key))
        {
          remove_at_bucket(idx, entry_idx);
          return true;
        }

        idx = (idx + 1) & m_mask;
      }
    }

    value_type *begin()
    {
      return m_entries.begin();
    }

    value_type *end()
    {
      return m_entries.end();
    }

    const value_type *begin() const
    {
      return m_entries.begin();
    }

    const value_type *end() const
    {
      return m_entries.end();
    }

    [[nodiscard]] size_type size() const
    {
      return m_entries.size();
    }

    [[nodiscard]] bool empty() const
    {
      return m_entries.empty();
    }

private:
    u64 hash_key(const K &key) const
    {
      return m_hasher(key);
    }

    [[nodiscard]] bool should_grow() const
    {
      return m_entries.size() * 10 >= m_buckets.size() * 8 || m_buckets.empty();
    }

    void grow()
    {
      size_type new_cap = (m_buckets.empty()) ? 16 : m_buckets.size() * 2;
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
        insert_into_buckets(i, m_entries[i]);
      }
    }

    void insert_into_buckets(u32 entry_idx, const K &key)
    {
      auto h = hash_key(key);
      auto idx = h & m_mask;

      while (m_buckets[idx] != INDEX_INVALID)
      {
        idx = (idx + 1) & m_mask;
      }

      m_buckets[idx] = entry_idx;
    }

    void remove_at_bucket(u32 bucket_idx, u32 entry_idx_to_remove)
    {
      backward_shift(bucket_idx);

      u32 last_idx = static_cast<u32>(m_entries.size() - 1);

      if (entry_idx_to_remove != last_idx)
      {
        m_entries[entry_idx_to_remove] = std::move(m_entries[last_idx]);

        update_bucket_pointer(m_entries[entry_idx_to_remove], last_idx, entry_idx_to_remove);
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

        auto h = hash_key(m_entries[entry_idx]);
        auto ideal_idx = h & m_mask;

        auto dist_current = (next - ideal_idx) & m_mask;
        auto dist_hole = (hole_idx - ideal_idx) & m_mask;

        if (dist_hole < dist_current)
        {
          m_buckets[hole_idx] = entry_idx;
          hole_idx = next;
        }

        next = (next + 1) & m_mask;
      }

      m_buckets[hole_idx] = INDEX_INVALID;
    }

    void update_bucket_pointer(const K &key, u32 old_idx, u32 new_idx)
    {
      auto h = hash_key(key);
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

namespace au
{
  template<typename T> using HashSet = containers::HashSet<T>;
}