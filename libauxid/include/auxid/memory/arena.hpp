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

#include <auxid/memory/allocator.hpp>

namespace au::memory
{
  struct ArenaAllocator
  {
    u8 *buffer = nullptr;
    usize length = 0;
    usize offset = 0;

    void init(u8 *buf, usize len)
    {
      buffer = buf;
      length = len;
      offset = 0;
    }

public:
    inline void *alloc(usize size)
    {
      return alloc(size, 8);
    }

    inline void *alloc(usize size, usize align)
    {
      uintptr_t curr_addr = reinterpret_cast<uintptr_t>(buffer) + offset;
      uintptr_t misalignment = curr_addr & (align - 1);
      ptrdiff_t padding = (align - misalignment) & (align - 1);

      usize total = size + padding;

      if (offset > length || total > length - offset)
        return nullptr;

      void *ptr = buffer + offset + padding;
      offset += total;
      return ptr;
    }

    inline void *realloc(void *ptr, usize old_size, usize new_size, usize align)
    {
      AU_UNUSED(ptr);
      AU_UNUSED(old_size);
      AU_UNUSED(new_size);
      AU_UNUSED(align);

      return nullptr;
    }

    inline void free(void *ptr, usize size, usize align)
    {
      AU_UNUSED(ptr);
      AU_UNUSED(size);
      AU_UNUSED(align);
    }
  };

  static_assert(AllocatorType<ArenaAllocator>, "Allocator class must conform to AllocatorT");
} // namespace au::memory