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

#include <auxid/builder/target.hpp>
#include <auxid/builder/package.hpp>

namespace au::builder
{
#define RUN_BUILDER(BuilderT)                                                                                          \
  AU_ENSURE_CLASS_HAS_CONCEPT(BuilderT, au::builder::CAuxidBuilder);                                                   \
  struct MainThreadGuard                                                                                               \
  {                                                                                                                    \
    MainThreadGuard()                                                                                                  \
    {                                                                                                                  \
      au::auxid::initialize_main_thread();                                                                             \
    }                                                                                                                  \
    ~MainThreadGuard()                                                                                                 \
    {                                                                                                                  \
      au::auxid::terminate_main_thread();                                                                              \
    }                                                                                                                  \
  };                                                                                                                   \
  extern "C" int main(int argc, char *argv[])                                                                          \
  {                                                                                                                    \
    MainThreadGuard main_thread_gurad{};                                                                               \
    BuilderT builder{};                                                                                                \
    const auto init_result = builder.init(argc, (const char **) argv);                                                 \
    if (!init_result)                                                                                                  \
    {                                                                                                                  \
      printf("[ERROR]: %s\n", init_result.error().c_str());                                                            \
      return -1;                                                                                                       \
    }                                                                                                                  \
    builder.run();                                                                                                     \
    const auto finalize_result = builder.finalize();                                                                   \
    if (!finalize_result)                                                                                              \
    {                                                                                                                  \
      printf("[ERROR]: %s\n", finalize_result.error().c_str());                                                        \
      return -1;                                                                                                       \
    }                                                                                                                  \
    return 0;                                                                                                          \
  }

  template<typename BuilderT>
  concept CAuxidBuilder = requires(BuilderT b, i32 argc, const char **argv) {
    { b.init(argc, argv) } -> std::same_as<Result<void>>;
    { b.run() } -> std::same_as<void>;
    { b.finalize() } -> std::same_as<Result<void>>;
  };

  enum class EAuxidRunMode
  {
    Invalid,
    DumpMetadata,
    GenerateNinja
  };

  class AuxidBuilder_Base
  {
private:
    EAuxidRunMode m_run_mode{EAuxidRunMode::Invalid};

    AuxidPackageInfo m_package_info;
    HashMap<String, String> m_required_packages;

    Vec<AuxidExecutableTarget *> m_executables;
    Vec<AuxidStaticLibTarget *> m_static_libs;

public:
    auto init(i32 argc, const char *argv[]) -> Result<void>;
    auto finalize() -> Result<void>;

protected:
    auto set_package_name(String name) -> void
    {
      m_package_info.name = name;
    }

    auto set_package_version(String version) -> void
    {
      m_package_info.version = version;
    }

    auto set_package_author(String author) -> void
    {
      m_package_info.author = author;
    }

    auto set_package_license(String license) -> void
    {
      m_package_info.license = license;
    }

    auto set_package_website(String website) -> void
    {
      m_package_info.website = website;
    }

    auto set_package_description(String description) -> void
    {
      m_package_info.description = description;
    }

    [[nodiscard]] auto get_package_info() const -> const AuxidPackageInfo &
    {
      return m_package_info;
    }

protected:
    auto require_package(String name, String version) -> void
    {
      m_required_packages[name] = version;
    }

protected:
    auto add_executable(String name) -> AuxidExecutableTarget &
    {
      const auto target = new AuxidExecutableTarget();
      m_executables.push_back(target);
      return *target;
    }

    auto add_static_lib(String name) -> AuxidStaticLibTarget &
    {
      const auto target = new AuxidStaticLibTarget();
      m_static_libs.push_back(target);
      return *target;
    }
  };
} // namespace au::builder