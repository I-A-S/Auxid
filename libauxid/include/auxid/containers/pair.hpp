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

#include <auxid/pch.hpp>

namespace au::containers
{
  template<typename T1, typename T2> struct Pair
  {
    T1 first;
    T2 second;

    constexpr Pair() = default;

    constexpr Pair(const T1 &a, const T2 &b) : first(a), second(b)
    {
    }

    constexpr Pair(T1 &&a, T2 &&b) : first(std::move(a)), second(std::move(b))
    {
    }
  };
} // namespace au::containers