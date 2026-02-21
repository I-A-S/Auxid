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

namespace au::memory
{
  template<typename T>
  concept AllocatorType = requires(T v, void *ptr, usize size, usize align) {
    { v.alloc(size) } -> std::same_as<void *>;
    { v.alloc(size, align) } -> std::same_as<void *>;
    { v.realloc(ptr, size, size, align) } -> std::same_as<void *>;
    { v.free(ptr, size, align) } -> std::same_as<void>;
  };
} // namespace au::memory