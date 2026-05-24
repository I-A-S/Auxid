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

import auxid;
import auxid.test;

using namespace au;

namespace
{
  struct HeapBlock final : test::Block
  {
    [[nodiscard]] auto get_name() const -> const char * override
    {
      return "memory::heap";
    }

    auto declare_tests() -> void override
    {
      add_test("alloc_free", [this] { return alloc_free(); });
      add_test("aligned_alloc", [this] { return aligned_alloc_(); });
    }

    auto alloc_free() -> bool
    {
      memory::HeapAllocator heap;
      void *ptr = heap.alloc(128);
      if (!check_not(ptr == nullptr, "alloc(128) != nullptr"))
        return false;

      u8 *bytes = static_cast<u8 *>(ptr);
      bytes[0] = 0xFF;
      bytes[127] = 0xAA;

      heap.free(ptr, 128, memory::HeapAllocator::natural_align);
      return true;
    }

    auto aligned_alloc_() -> bool
    {
      memory::HeapAllocator heap;
      void *ptr = heap.alloc(64, 64);
      if (!check_not(ptr == nullptr, "alloc(64, 64) != nullptr"))
        return false;

      uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
      bool aligned = (addr % 64) == 0;

      heap.free(ptr, 64, 64);
      return check(aligned, "alloc(64, 64) is 64-byte aligned");
    }
  };

  const test::AutoRegister<HeapBlock> _registered;
} // namespace
