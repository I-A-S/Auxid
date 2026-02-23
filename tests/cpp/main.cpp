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

#include <auxid/auxid.hpp>

#include <auxid/utils/test.hpp>

#include <iostream>

using namespace au;

int main(int agrc, char *argv[])
{
  struct ThreadInitGuard
  {
    ThreadInitGuard()
    {
      auxid::initialize_main_thread();
    }

    ~ThreadInitGuard()
    {
      auxid::terminate_main_thread();
    }
  } _thread_init_guard;

  std::cout << console::GREEN << "\n================================\n";
  std::cout << "   LibAuxid - Unit Test Suite\n";
  std::cout << "================================\n" << console::RESET << "\n";

  return test::TestRegistry::run_all();
}