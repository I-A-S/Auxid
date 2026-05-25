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
#include <stdexcept>

#include <benchmark/benchmark.h>

#include "result_vs_exceptions_shared.hpp"

namespace
{
#if defined(_MSC_VER)
  __declspec(noinline)
#else
  [[gnu::noinline]]
#endif
  static auto compute_throw(std::int32_t in, bool fail_this_call) -> std::int32_t
  {
    if (fail_this_call)
    {
      throw std::runtime_error("compute_failed");
    }
    return in * 3 + 7;
  }

  auto run_throw(benchmark::State &state, std::uint32_t fail_threshold) -> void
  {
    for (auto _ : state)
    {
      au_bench::Lcg rng{au_bench::LCG_SEED};
      std::int64_t acc = 0;
      std::int64_t err_count = 0;

      for (std::size_t i = 0; i < au_bench::WORKLOAD_SIZE; ++i)
      {
        const auto r = rng.next();
        const bool should_fail = r < fail_threshold;
        try
        {
          acc += compute_throw(static_cast<std::int32_t>(i), should_fail);
        }
        catch (const std::runtime_error &)
        {
          ++err_count;
        }
      }
      benchmark::DoNotOptimize(acc);
      benchmark::DoNotOptimize(err_count);
    }
    state.SetItemsProcessed(static_cast<std::int64_t>(state.iterations()) *
                            static_cast<std::int64_t>(au_bench::WORKLOAD_SIZE));
  }
} // namespace

static auto BM_Exception_HappyPath(benchmark::State &state) -> void
{
  run_throw(state, au_bench::THRESHOLD_0_PCT);
}

BENCHMARK(BM_Exception_HappyPath);

static auto BM_Exception_ErrorRate_1pct(benchmark::State &state) -> void
{
  run_throw(state, au_bench::THRESHOLD_1_PCT);
}

BENCHMARK(BM_Exception_ErrorRate_1pct);

static auto BM_Exception_ErrorRate_50pct(benchmark::State &state) -> void
{
  run_throw(state, au_bench::THRESHOLD_50_PCT);
}

BENCHMARK(BM_Exception_ErrorRate_50pct);
