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
#include <auxid/memory/arena.hpp>

using namespace au;

AUT_BEGIN_BLOCK(memory, arena)

auto test_arena_alloc() -> bool
{
  u8 buffer[1024];
  memory::ArenaAllocator arena;
  arena.init(buffer, sizeof(buffer));

  void *ptr1 = arena.alloc(16);
  AUT_CHECK_NOT(ptr1 == nullptr);
  AUT_CHECK(arena.offset >= 16);

  void *ptr2 = arena.alloc(32);
  AUT_CHECK_NOT(ptr2 == nullptr);
  AUT_CHECK(arena.offset >= 48);

  return true;
}

auto test_arena_exhaustion() -> bool
{
  u8 buffer[64];
  memory::ArenaAllocator arena;
  arena.init(buffer, sizeof(buffer));

  void *ptr1 = arena.alloc(64);
  AUT_CHECK_NOT(ptr1 == nullptr);

  void *ptr2 = arena.alloc(8);
  AUT_CHECK_EQ(ptr2, nullptr);

  return true;
}

auto test_arena_clear() -> bool
{
  u8 buffer[128];
  memory::ArenaAllocator arena;
  arena.init(buffer, sizeof(buffer));

  arena.alloc(64);
  AUT_CHECK(arena.offset >= 64);

  arena.clear();
  AUT_CHECK_EQ(arena.offset, 0);

  return true;
}

AUT_BEGIN_TEST_LIST()
AUT_ADD_TEST(test_arena_alloc);
AUT_ADD_TEST(test_arena_exhaustion);
AUT_ADD_TEST(test_arena_clear);
AUT_END_TEST_LIST()

AUT_END_BLOCK()

AUT_REGISTER_ENTRY(memory, arena);