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

#include <auxid/builder/base.hpp>

namespace au::builder
{
  enum class EAuxidTargetKind
  {
    Interface,
    Executable,
    StaticLib,
    SharedLib,
    ObjectSet,
  };

  struct AuxidTargetBuildInfo
  {
    String name;
    EAuxidTargetKind kind;
    Vec<String> sources;
    Vec<String> include_dirs;
    Vec<String> library_dirs;
    Vec<String> compile_flags;
    Vec<String> link_flags;
    Vec<String> linked_packages;
  };

  template<typename TargetT>
  concept CAuxidTarget = requires(TargetT t, Span<const String> const_string_span_val, String string_val) {
    { t.generate_build_info() } -> std::same_as<AuxidTargetBuildInfo>;
  };

  template<typename Derived> class CRTP_AuxidTarget
  {
protected:
    String m_name;
    Vec<String> m_sources;
    Vec<String> m_include_dirs;
    Vec<String> m_library_dirs;
    Vec<String> m_compile_flags;
    Vec<String> m_link_flags;
    Vec<String> m_linked_packages;

public:
    auto add_sources(Span<const String> values) -> void
    {
      m_sources.reserve(m_sources.capacity() + values.size());
      for (auto t : values)
        m_sources.push_back(t);
    }

    auto add_include_dirs(Span<const String> values) -> void
    {
      m_include_dirs.reserve(m_include_dirs.capacity() + values.size());
      for (auto t : values)
        m_include_dirs.push_back(t);
    }

    auto add_library_dirs(Span<const String> values) -> void
    {
      m_library_dirs.reserve(m_library_dirs.capacity() + values.size());
      for (auto t : values)
        m_library_dirs.push_back(t);
    }

    auto add_compile_flags(Span<const String> values) -> void
    {
      m_compile_flags.reserve(m_compile_flags.capacity() + values.size());
      for (auto t : values)
        m_compile_flags.push_back(t);
    }

    auto add_link_flags(Span<const String> values) -> void
    {
      m_link_flags.reserve(m_link_flags.capacity() + values.size());
      for (auto t : values)
        m_link_flags.push_back(t);
    }

    auto link_package(String name) -> void
    {
      m_linked_packages.push_back(std::move(name));
    }

    static auto get_kind() -> EAuxidTargetKind
    {
      return Derived::TARGET_KIND;
    }

    auto generate_build_info() -> AuxidTargetBuildInfo
    {
      AuxidTargetBuildInfo info;
      info.name = this->m_name;
      info.kind = Derived::TARGET_KIND;

      info.sources = this->m_sources;
      info.include_dirs = this->m_include_dirs;
      info.library_dirs = this->m_library_dirs;
      info.compile_flags = this->m_compile_flags;
      info.link_flags = this->m_link_flags;
      info.linked_packages = this->m_linked_packages;

      return info;
    }
  };

#define DECLARE_TARGET(kind)                                                                                           \
  class Auxid##kind##Target : public CRTP_AuxidTarget<Auxid##kind##Target>                                             \
  {                                                                                                                    \
public:                                                                                                                \
    static constexpr auto TARGET_KIND = EAuxidTargetKind::kind;                                                        \
  };                                                                                                                   \
  AU_ENSURE_CLASS_HAS_CONCEPT(Auxid##kind##Target, CAuxidTarget);

  DECLARE_TARGET(Interface);
  DECLARE_TARGET(Executable);
  DECLARE_TARGET(StaticLib);
  DECLARE_TARGET(SharedLib);
  DECLARE_TARGET(ObjectSet);

#undef DECLARE_TARGET
} // namespace au::builder