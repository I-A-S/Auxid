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

#include <cstdint>
#include <random>
#include <string>
#include <unordered_map>

#include <benchmark/benchmark.h>

import auxid;

using namespace au;

namespace
{
  constexpr u32 RANGE_MIN = 1u << 6;
  constexpr u32 RANGE_MAX = 1u << 16;

  inline auto make_keys_i32(usize n) -> std::vector<i32>
  {
    std::mt19937_64 rng{0xA0C1D54Eu};
    std::uniform_int_distribution<i32> dist(1, 1'000'000'000);
    std::vector<i32> keys;
    keys.reserve(n);
    for (usize i = 0; i < n; ++i)
    {
      keys.push_back(dist(rng));
    }
    return keys;
  }

  inline auto make_keys_string(usize n) -> std::vector<std::string>
  {
    std::mt19937_64 rng{0x5EEDB055u};
    std::uniform_int_distribution<int> len_dist(3, 24);
    std::uniform_int_distribution<int> chr_dist('a', 'z');
    std::vector<std::string> keys;
    keys.reserve(n);
    for (usize i = 0; i < n; ++i)
    {
      const auto len = static_cast<usize>(len_dist(rng));
      std::string s;
      s.resize(len);
      for (usize k = 0; k < len; ++k)
      {
        s[k] = static_cast<char>(chr_dist(rng));
      }
      keys.push_back(std::move(s));
    }
    return keys;
  }

  struct AuStringStdHash
  {
    [[nodiscard]] auto operator()(const String &s) const noexcept -> usize
    {
      containers::Hash<String> h{};
      return static_cast<usize>(h(StringView{s.data(), s.size()}));
    }
  };

  struct AuStringStdEq
  {
    [[nodiscard]] auto operator()(const String &a, const String &b) const noexcept -> bool
    {
      return StringView{a.data(), a.size()} == StringView{b.data(), b.size()};
    }
  };
} // namespace

static auto BM_AuHashMap_Insert_i32(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  const auto keys = make_keys_i32(n);

  for (auto _ : state)
  {
    HashMap<i32, i32> map;
    map.reserve(n);
    for (usize i = 0; i < n; ++i)
    {
      map.insert(keys[i], static_cast<i32>(i));
    }
    benchmark::DoNotOptimize(map);
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(n));
}

BENCHMARK(BM_AuHashMap_Insert_i32)->Range(RANGE_MIN, RANGE_MAX);

static auto BM_StdUnorderedMap_Insert_i32(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  const auto keys = make_keys_i32(n);

  for (auto _ : state)
  {
    std::unordered_map<i32, i32> map;
    map.reserve(n);
    for (usize i = 0; i < n; ++i)
    {
      map.emplace(keys[i], static_cast<i32>(i));
    }
    benchmark::DoNotOptimize(map);
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(n));
}

BENCHMARK(BM_StdUnorderedMap_Insert_i32)->Range(RANGE_MIN, RANGE_MAX);

static auto BM_AuHashMap_Lookup_i32(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  const auto keys = make_keys_i32(n);

  HashMap<i32, i32> map;
  map.reserve(n);
  for (usize i = 0; i < n; ++i)
  {
    map.insert(keys[i], static_cast<i32>(i));
  }

  usize idx = 0;
  for (auto _ : state)
  {
    auto *v = map.find(keys[idx]);
    benchmark::DoNotOptimize(v);
    idx = (idx + 1) % n;
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));
}

BENCHMARK(BM_AuHashMap_Lookup_i32)->Range(RANGE_MIN, RANGE_MAX);

static auto BM_StdUnorderedMap_Lookup_i32(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  const auto keys = make_keys_i32(n);

  std::unordered_map<i32, i32> map;
  map.reserve(n);
  for (usize i = 0; i < n; ++i)
  {
    map.emplace(keys[i], static_cast<i32>(i));
  }

  usize idx = 0;
  for (auto _ : state)
  {
    auto it = map.find(keys[idx]);
    benchmark::DoNotOptimize(it);
    idx = (idx + 1) % n;
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));
}

BENCHMARK(BM_StdUnorderedMap_Lookup_i32)->Range(RANGE_MIN, RANGE_MAX);

static auto BM_AuHashMap_Insert_String(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  const auto keys_std = make_keys_string(n);

  std::vector<String> keys;
  keys.reserve(n);
  for (const auto &s : keys_std)
  {
    keys.emplace_back(s.data(), s.size());
  }

  for (auto _ : state)
  {
    HashMap<String, i32> map;
    map.reserve(n);
    for (usize i = 0; i < n; ++i)
    {
      map.insert(keys[i], static_cast<i32>(i));
    }
    benchmark::DoNotOptimize(map);
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(n));
}

BENCHMARK(BM_AuHashMap_Insert_String)->Range(RANGE_MIN, RANGE_MAX);

static auto BM_StdUnorderedMap_Insert_String(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  const auto keys_std = make_keys_string(n);

  std::vector<String> keys;
  keys.reserve(n);
  for (const auto &s : keys_std)
  {
    keys.emplace_back(s.data(), s.size());
  }

  for (auto _ : state)
  {
    std::unordered_map<String, i32, AuStringStdHash, AuStringStdEq> map;
    map.reserve(n);
    for (usize i = 0; i < n; ++i)
    {
      map.emplace(keys[i], static_cast<i32>(i));
    }
    benchmark::DoNotOptimize(map);
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(n));
}

BENCHMARK(BM_StdUnorderedMap_Insert_String)->Range(RANGE_MIN, RANGE_MAX);

static auto BM_AuHashMap_Lookup_String(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  const auto keys_std = make_keys_string(n);

  std::vector<String> keys;
  keys.reserve(n);
  for (const auto &s : keys_std)
  {
    keys.emplace_back(s.data(), s.size());
  }

  HashMap<String, i32> map;
  map.reserve(n);
  for (usize i = 0; i < n; ++i)
  {
    map.insert(keys[i], static_cast<i32>(i));
  }

  usize idx = 0;
  for (auto _ : state)
  {
    auto *v = map.find(keys[idx]);
    benchmark::DoNotOptimize(v);
    idx = (idx + 1) % n;
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));
}

BENCHMARK(BM_AuHashMap_Lookup_String)->Range(RANGE_MIN, RANGE_MAX);

static auto BM_StdUnorderedMap_Lookup_String(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  const auto keys_std = make_keys_string(n);

  std::vector<String> keys;
  keys.reserve(n);
  for (const auto &s : keys_std)
  {
    keys.emplace_back(s.data(), s.size());
  }

  std::unordered_map<String, i32, AuStringStdHash, AuStringStdEq> map;
  map.reserve(n);
  for (usize i = 0; i < n; ++i)
  {
    map.emplace(keys[i], static_cast<i32>(i));
  }

  usize idx = 0;
  for (auto _ : state)
  {
    auto it = map.find(keys[idx]);
    benchmark::DoNotOptimize(it);
    idx = (idx + 1) % n;
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));
}

BENCHMARK(BM_StdUnorderedMap_Lookup_String)->Range(RANGE_MIN, RANGE_MAX);

static auto BM_AuHashMap_Erase_i32(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  const auto keys = make_keys_i32(n);

  for (auto _ : state)
  {
    state.PauseTiming();
    HashMap<i32, i32> map;
    map.reserve(n);
    for (usize i = 0; i < n; ++i)
      map.insert(keys[i], static_cast<i32>(i));
    state.ResumeTiming();

    for (usize i = 0; i < n; ++i)
    {
      const bool ok = map.erase(keys[i]);
      benchmark::DoNotOptimize(ok);
    }
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(n));
}

BENCHMARK(BM_AuHashMap_Erase_i32)->Range(RANGE_MIN, RANGE_MAX);

static auto BM_StdUnorderedMap_Erase_i32(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  const auto keys = make_keys_i32(n);

  for (auto _ : state)
  {
    state.PauseTiming();
    std::unordered_map<i32, i32> map;
    map.reserve(n);
    for (usize i = 0; i < n; ++i)
      map.emplace(keys[i], static_cast<i32>(i));
    state.ResumeTiming();

    for (usize i = 0; i < n; ++i)
    {
      const auto removed = map.erase(keys[i]);
      benchmark::DoNotOptimize(removed);
    }
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(n));
}

BENCHMARK(BM_StdUnorderedMap_Erase_i32)->Range(RANGE_MIN, RANGE_MAX);

static auto BM_AuHashMap_Mixed_i32(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  const auto keys = make_keys_i32(n);

  for (auto _ : state)
  {
    HashMap<i32, i32> map;
    map.reserve(n / 2);
    for (usize i = 0; i < n; ++i)
    {
      map.insert(keys[i], static_cast<i32>(i));
      if ((i & 0x3u) == 0x3u && i >= 4)
      {
        const bool ok = map.erase(keys[i - 4]);
        benchmark::DoNotOptimize(ok);
      }
      auto *v = map.find(keys[i >> 1]);
      benchmark::DoNotOptimize(v);
    }
    benchmark::DoNotOptimize(map);
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(n));
}

BENCHMARK(BM_AuHashMap_Mixed_i32)->Range(RANGE_MIN, RANGE_MAX);

static auto BM_StdUnorderedMap_Mixed_i32(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  const auto keys = make_keys_i32(n);

  for (auto _ : state)
  {
    std::unordered_map<i32, i32> map;
    map.reserve(n / 2);
    for (usize i = 0; i < n; ++i)
    {
      map.emplace(keys[i], static_cast<i32>(i));
      if ((i & 0x3u) == 0x3u && i >= 4)
      {
        const auto removed = map.erase(keys[i - 4]);
        benchmark::DoNotOptimize(removed);
      }
      auto it = map.find(keys[i >> 1]);
      benchmark::DoNotOptimize(it);
    }
    benchmark::DoNotOptimize(map);
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(n));
}

BENCHMARK(BM_StdUnorderedMap_Mixed_i32)->Range(RANGE_MIN, RANGE_MAX);

static auto BM_AuHashMap_Iterate_i32(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  const auto keys = make_keys_i32(n);

  HashMap<i32, i32> map;
  map.reserve(n);
  for (usize i = 0; i < n; ++i)
    map.insert(keys[i], static_cast<i32>(i));

  for (auto _ : state)
  {
    i64 sum = 0;
    for (const auto &p : map)
      sum += static_cast<i64>(p.second);
    benchmark::DoNotOptimize(sum);
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(n));
}

BENCHMARK(BM_AuHashMap_Iterate_i32)->Range(RANGE_MIN, RANGE_MAX);

static auto BM_StdUnorderedMap_Iterate_i32(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  const auto keys = make_keys_i32(n);

  std::unordered_map<i32, i32> map;
  map.reserve(n);
  for (usize i = 0; i < n; ++i)
    map.emplace(keys[i], static_cast<i32>(i));

  for (auto _ : state)
  {
    i64 sum = 0;
    for (const auto &p : map)
      sum += static_cast<i64>(p.second);
    benchmark::DoNotOptimize(sum);
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(n));
}

BENCHMARK(BM_StdUnorderedMap_Iterate_i32)->Range(RANGE_MIN, RANGE_MAX);

static auto BM_AuHashMap_HighCollisionH2_u64(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  std::vector<u64> keys;
  keys.reserve(n);
  for (usize i = 0; i < n; ++i)
    keys.push_back(static_cast<u64>(i) * 128ULL ^ 0xDEADC0DE12345678ULL);

  HashMap<u64, i32> map;
  map.reserve(n);
  for (usize i = 0; i < n; ++i)
    map.insert(keys[i], static_cast<i32>(i));

  usize idx = 0;
  for (auto _ : state)
  {
    auto *v = map.find(keys[idx]);
    benchmark::DoNotOptimize(v);
    idx = (idx + 1) % n;
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));
}

BENCHMARK(BM_AuHashMap_HighCollisionH2_u64)->Range(RANGE_MIN, RANGE_MAX);

static auto BM_StdUnorderedMap_HighCollisionH2_u64(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  std::vector<u64> keys;
  keys.reserve(n);
  for (usize i = 0; i < n; ++i)
    keys.push_back(static_cast<u64>(i) * 128ULL ^ 0xDEADC0DE12345678ULL);

  std::unordered_map<u64, i32> map;
  map.reserve(n);
  for (usize i = 0; i < n; ++i)
    map.emplace(keys[i], static_cast<i32>(i));

  usize idx = 0;
  for (auto _ : state)
  {
    auto it = map.find(keys[idx]);
    benchmark::DoNotOptimize(it);
    idx = (idx + 1) % n;
  }
  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));
}

BENCHMARK(BM_StdUnorderedMap_HighCollisionH2_u64)->Range(RANGE_MIN, RANGE_MAX);
