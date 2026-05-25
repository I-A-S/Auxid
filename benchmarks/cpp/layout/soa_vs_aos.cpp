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

#include <benchmark/benchmark.h>

import auxid;

using namespace au;

namespace
{
  struct Particle
  {
    f32 pos_x;
    f32 pos_y;
    f32 pos_z;
    f32 vel_x;
    f32 vel_y;
    f32 vel_z;
    f32 mass;
  };

  static_assert(sizeof(Particle) == 7 * sizeof(f32), "Unexpected Particle layout");

  struct ParticleSoA
  {
    Vec<f32> pos_x;
    Vec<f32> pos_y;
    Vec<f32> pos_z;
    Vec<f32> vel_x;
    Vec<f32> vel_y;
    Vec<f32> vel_z;
    Vec<f32> mass;

    explicit ParticleSoA(usize n) : pos_x(n), pos_y(n), pos_z(n), vel_x(n), vel_y(n), vel_z(n), mass(n)
    {
    }

    auto seed() -> void
    {
      for (usize i = 0; i < pos_x.size(); ++i)
      {
        const auto fi = static_cast<f32>(i);
        pos_x[i] = fi;
        pos_y[i] = fi * 0.5f;
        pos_z[i] = fi * 0.25f;
        vel_x[i] = 0.1f;
        vel_y[i] = 0.2f;
        vel_z[i] = 0.3f;
        mass[i] = 1.0f;
      }
    }
  };

  inline auto make_aos(usize n) -> Vec<Particle>
  {
    Vec<Particle> v(n);
    for (usize i = 0; i < n; ++i)
    {
      const auto fi = static_cast<f32>(i);
      v[i] = Particle{fi, fi * 0.5f, fi * 0.25f, 0.1f, 0.2f, 0.3f, 1.0f};
    }
    return v;
  }

  constexpr f32 DT = 0.016f;
} // namespace

static auto BM_AoS_Integrate(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  auto particles = make_aos(n);

  for (auto _ : state)
  {
    auto *__restrict p = particles.data();
    for (usize i = 0; i < n; ++i)
    {
      p[i].pos_x += p[i].vel_x * DT;
      p[i].pos_y += p[i].vel_y * DT;
      p[i].pos_z += p[i].vel_z * DT;
    }
    benchmark::DoNotOptimize(p);
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(n));
  state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(n) *
                          static_cast<int64_t>(sizeof(Particle)));
}

BENCHMARK(BM_AoS_Integrate)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18)->Arg(1 << 22);

static auto BM_SoA_Integrate(benchmark::State &state) -> void
{
  const auto n = static_cast<usize>(state.range(0));
  ParticleSoA particles{n};
  particles.seed();

  for (auto _ : state)
  {
    auto *__restrict px = particles.pos_x.data();
    auto *__restrict py = particles.pos_y.data();
    auto *__restrict pz = particles.pos_z.data();
    const auto *__restrict vx = particles.vel_x.data();
    const auto *__restrict vy = particles.vel_y.data();
    const auto *__restrict vz = particles.vel_z.data();

    for (usize i = 0; i < n; ++i)
    {
      px[i] += vx[i] * DT;
      py[i] += vy[i] * DT;
      pz[i] += vz[i] * DT;
    }
    benchmark::DoNotOptimize(px);
    benchmark::DoNotOptimize(py);
    benchmark::DoNotOptimize(pz);
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(n));
  state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(n) *
                          static_cast<int64_t>(6 * sizeof(f32)));
}

BENCHMARK(BM_SoA_Integrate)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18)->Arg(1 << 22);
