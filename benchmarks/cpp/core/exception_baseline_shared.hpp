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

#include <cstdint>
#include <stdexcept>
#include <string_view>

#include "result_vs_exceptions_shared.hpp"

namespace au_bench
{
#if defined(_MSC_VER)
  __declspec(noinline)
#else
  [[gnu::noinline]]
#endif
  inline auto compute_throw_leaf(std::int32_t in, bool fail_this_call) -> std::int32_t
  {
    if (fail_this_call)
    {
      throw std::runtime_error(FAIL_MESSAGE);
    }
    return in * 3 + 7;
  }

#if defined(_MSC_VER)
  __declspec(noinline)
#else
  [[gnu::noinline]]
#endif
  inline auto chain_compute_throw(std::uint32_t depth, std::int32_t in, bool fail_this_call) -> std::int32_t
  {
    if (depth <= 1)
    {
      return compute_throw_leaf(in, fail_this_call);
    }
    return chain_compute_throw(depth - 1, in, fail_this_call);
  }

  inline auto verify_fail_message_lengths() -> void
  {
    if (std::string_view{FAIL_MESSAGE}.size() != FAIL_MESSAGE_LEN)
    {
      throw std::logic_error("benchmark fail message length mismatch");
    }
    if (std::string_view{std::runtime_error{FAIL_MESSAGE}.what()}.size() != FAIL_MESSAGE_LEN)
    {
      throw std::logic_error("benchmark exception message length mismatch");
    }
  }
} // namespace au_bench
