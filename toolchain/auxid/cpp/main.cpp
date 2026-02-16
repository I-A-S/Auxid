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

#include <auxid/builder/builder.hpp>

class Builder : public au::builder::AuxidBuilder_Base
{
  public:
  auto run() -> void
  {
    set_package_name("LibAuxid");
    set_package_author("I-A-S");
    set_package_version("1.0.0");

    // require_package("PlatformOps", ">=1.2.0");

    auto &lib = add_static_lib("LibAuxid");
    lib.add_include_dirs(std::initializer_list<au::String>{
        "include",
        "src/hpp",
    });
    lib.add_sources(std::initializer_list<au::String>{
        "src/cpp/builder/builder.cpp",
        "src/cpp/builder/target.cpp",
        "src/cpp/builder/ninja_generator.cpp",
        "src/cpp/rpmalloc/rpmalloc.c",
        "src/cpp/tinycthread/tinycthread.c",
        "src/cpp/auxid.cpp",
    });
  }
};

RUN_BUILDER(Builder);

// namespace au::compiler
//{
//   auto main(i32 argc, const char *argv[]) -> Result<void>
//   {
//     return {};
//   }
// } // namespace au::compiler
//
// extern "C" int main(int argc, char *argv[])
//{
//   au::auxid::initialize_main_thread();
//   const auto result = au::compiler::main(argc, (const char **) argv);
//   if (!result)
//   {
//     printf("[FATAL ERROR]: %s\n", result.error().c_str());
//     au::auxid::terminate_main_thread();
//     return -1;
//   }
//   au::auxid::terminate_main_thread();
//   return 0;
// }