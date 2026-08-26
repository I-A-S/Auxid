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

// OS Foundation Layer: time (docs/OS-CONTRACT.md). Deliberately tiny — no
// timers or schedulers; products own their run loops. None of these can fail
// on any supported target, so none return Result.
//
// Declarations-only on purpose: keeping <chrono>/<thread> out of this
// interface's global module fragment avoids MSVC's textual-include-vs-
// imported-module header collisions in consumer TUs.

module;

#include <auxid/macros.hpp>

export module auxid.time;

export import auxid.core;

export namespace au::time
{
  // Monotonic nanoseconds since an unspecified epoch. Comparable only within
  // one process lifetime; never goes backwards; unaffected by wall-clock
  // adjustments.
  [[nodiscard]] AUXID_API auto monotonic_ns() -> u64;

  // Milliseconds since the Unix epoch, UTC. Wall time: may jump on clock
  // adjustment — never use for measuring durations.
  [[nodiscard]] AUXID_API auto wall_unix_ms() -> i64;

  // Blocks the calling thread for AT LEAST the requested duration; the OS
  // scheduler may add latency. sleep_ms(0) yields the remainder of the slice.
  AUXID_API auto sleep_ms(u32 milliseconds) -> void;
} // namespace au::time
