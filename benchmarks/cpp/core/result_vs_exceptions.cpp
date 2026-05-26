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

#include <auxid/macros.hpp>
#include <benchmark/benchmark.h>

#include "result_vs_exceptions_shared.hpp"

import auxid;

using namespace au;

namespace
{
#if defined(_MSC_VER)
  __declspec(noinline)
#else
  [[gnu::noinline]]
#endif
  static auto compute_result(i32 in, bool fail_this_call) -> Result<i32>
  {
    if AU_UNLIKELY (fail_this_call)
    {
      return fail(String{"compute_failed"});
    }
    return in * 3 + 7;
  }

  auto run_result(benchmark::State &state, u32 fail_threshold) -> void
  {
    for (auto _ : state)
    {
      au_bench::Lcg rng{au_bench::LCG_SEED};
      i64 acc = 0;
      i64 err_count = 0;

      for (usize i = 0; i < au_bench::WORKLOAD_SIZE; ++i)
      {
        const auto r = rng.next();
        const bool should_fail = r < fail_threshold;
        Result<i32> out = compute_result(static_cast<i32>(i), should_fail);
        if (out.is_ok())
        {
          acc += out.unwrap();
        }
        else
        {
          ++err_count;
        }
      }
      benchmark::DoNotOptimize(acc);
      benchmark::DoNotOptimize(err_count);
    }
    state.SetItemsProcessed(static_cast<i64>(state.iterations()) * static_cast<i64>(au_bench::WORKLOAD_SIZE));
  }
} // namespace

static auto BM_AuResult_HappyPath(benchmark::State &state) -> void
{
  run_result(state, au_bench::THRESHOLD_0_PCT);
}

BENCHMARK(BM_AuResult_HappyPath);

static auto BM_AuResult_ErrorRate_1pct(benchmark::State &state) -> void
{
  run_result(state, au_bench::THRESHOLD_1_PCT);
}

BENCHMARK(BM_AuResult_ErrorRate_1pct);

static auto BM_AuResult_ErrorRate_50pct(benchmark::State &state) -> void
{
  run_result(state, au_bench::THRESHOLD_50_PCT);
}

BENCHMARK(BM_AuResult_ErrorRate_50pct);
