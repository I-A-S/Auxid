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

#if !defined(AUXID_USE_SYSTEM_MALLOC)
#  include <auxid/vendor/rpmalloc/rpmalloc.h>
#endif

namespace au::memory
{
#if defined(AUXID_USE_SYSTEM_MALLOC)
  constexpr usize MALLOC_ALIGNMENT = alignof(max_align_t);

  struct HeapAllocator
  {
    inline void *alloc(usize size)
    {
      return ::malloc(size);
    }

    inline void *alloc(usize size, usize align)
    {
      if (align <= MALLOC_ALIGNMENT)
      {
        return ::malloc(size);
      }
      return aligned_alloc(align, size);
    }

    inline void *realloc(void *ptr, usize old_size, usize new_size, usize align)
    {
      return nullptr;
    }

    inline void free(void *ptr, usize size, usize align)
    {
      if (align <= MALLOC_ALIGNMENT)
      {
        ::free(ptr);
      }
      else
      {
        aligned_free(ptr);
      }
    }
  };
#else
  constexpr usize RPMALLOC_NATURAL_ALIGN = 16;

  struct HeapAllocator
  {
    inline void *alloc(usize size)
    {
      return rpmalloc(size);
    }

    inline void *alloc(usize size, usize align)
    {
      if (align <= RPMALLOC_NATURAL_ALIGN)
      {
        return ::rpmalloc(size);
      }
      return rpaligned_alloc(align, size);
    }

    inline void *realloc(void *ptr, usize old_size, usize new_size, usize align)
    {
      return rpaligned_realloc(ptr, align, new_size, old_size, 0);
    }

    inline void free(void *ptr, usize size, usize align)
    {
      AU_UNUSED(size);
      AU_UNUSED(align);
      rpfree(ptr);
    }
  };
#endif

  static_assert(AllocatorType<HeapAllocator>, "Allocator class must conform to AllocatorT");
} // namespace au::memory