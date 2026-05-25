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

#pragma once

#include <cstddef>
#include <cstdint>

namespace au_bench
{
  inline constexpr std::size_t WORKLOAD_SIZE = 1024;
  inline constexpr std::uint64_t LCG_SEED = 0xDEC0DECULL;

  struct Lcg
  {
    std::uint64_t state;

    constexpr explicit Lcg(std::uint64_t seed) noexcept : state(seed)
    {
    }

    constexpr auto next() noexcept -> std::uint32_t
    {
      state = state * 6364136223846793005ULL + 1442695040888963407ULL;
      return static_cast<std::uint32_t>(state >> 32);
    }
  };

  inline constexpr std::uint32_t THRESHOLD_0_PCT = 0u;
  inline constexpr std::uint32_t THRESHOLD_1_PCT = static_cast<std::uint32_t>(0.01 * static_cast<double>(UINT32_MAX));
  inline constexpr std::uint32_t THRESHOLD_50_PCT = static_cast<std::uint32_t>(0.50 * static_cast<double>(UINT32_MAX));
} // namespace au_bench
