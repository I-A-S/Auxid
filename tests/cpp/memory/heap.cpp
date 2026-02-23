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

#include <auxid/utils/test.hpp>
#include <auxid/memory/heap.hpp>

using namespace au;

AUT_BEGIN_BLOCK(memory, heap)

auto test_heap_alloc_free() -> bool
{
  memory::HeapAllocator heap;
  void *ptr = heap.alloc(128);
  AUT_CHECK_NOT(ptr == nullptr);

  u8 *bytes = static_cast<u8 *>(ptr);
  bytes[0] = 0xFF;
  bytes[127] = 0xAA;

  heap.free(ptr, 128, memory::RPMALLOC_NATURAL_ALIGN);
  return true;
}

auto test_heap_aligned_alloc() -> bool
{
  memory::HeapAllocator heap;
  void *ptr = heap.alloc(64, 64);
  AUT_CHECK_NOT(ptr == nullptr);

  uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
  AUT_CHECK_EQ(addr % 64, 0);

  heap.free(ptr, 64, 64);
  return true;
}

AUT_BEGIN_TEST_LIST()
AUT_ADD_TEST(test_heap_alloc_free);
AUT_ADD_TEST(test_heap_aligned_alloc);
AUT_END_TEST_LIST()

AUT_END_BLOCK()

AUT_REGISTER_ENTRY(memory, heap);