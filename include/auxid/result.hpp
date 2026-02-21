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
#include <auxid/containers/string.hpp>

namespace au
{
  template<typename T> using Result = ResultT<T, String>;

  template<typename... Args> [[nodiscard]] inline auto fail(const char *fmt, Args &&...args)
  {
    return fail(String::format(fmt, std::forward<Args>(args)...));
  }

  template<typename... Args> [[noreturn]] void panic(const char *fmt, Args &&...args)
  {
    panic(String::format(fmt, std::forward(args)...));
  }
} // namespace au