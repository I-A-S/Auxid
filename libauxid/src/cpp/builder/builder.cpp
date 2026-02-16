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

#include <builder/ninja_generator.hpp>

namespace au::builder
{
  auto AuxidBuilder_Base::init(i32 argc, const char *argv[]) -> Result<void>
  {
    for (i32 i = 1; i < argc; i++)
    {
      String arg(argv[i]);
      if (arg == "--auxid-dump-metadata")
      {
        m_run_mode = EAuxidRunMode::DumpMetadata;
        return {};
      }
      else if (arg == "--auxid-generate-ninja")
      {
        m_run_mode = EAuxidRunMode::GenerateNinja;
        return {};
      }
    }

    m_run_mode = EAuxidRunMode::Invalid;
    return {};
  }

  auto generate_ninja_for_target(NinjaGenerator &ninja, const AuxidTargetBuildInfo &info) -> void
  {
    String target_cflags = "";
    for (const auto &inc : info.include_dirs)
    {
      target_cflags.append("-I" + inc + " ");
    }
    for (const auto &flag : info.compile_flags)
    {
      target_cflags.append(flag + " ");
    }

    String target_ldflags = "";
    for (const auto &lib : info.library_dirs)
    {
      target_ldflags.append("-L" + lib + " ");
    }
    for (const auto &flag : info.link_flags)
    {
      target_ldflags.append(flag + " ");
    }

    Vec<String> object_files;
    for (const auto &source_file : info.sources)
    {
      String obj_path = ".auxid/obj/" + info.name + "/" + source_file + ".o";
      object_files.push_back(obj_path);

      NinjaBuildEdge compile_edge;
      compile_edge.outputs.push_back(obj_path);
      compile_edge.rule_name = "cxx";
      compile_edge.inputs.push_back(source_file);

      compile_edge.edge_variables.push_back({"cflags", target_cflags});

      ninja.add_edge(compile_edge);
    }

    if (info.kind == EAuxidTargetKind::Executable)
    {
      String exe_path = ".auxid/bin/" + info.name;

      NinjaBuildEdge link_edge;
      link_edge.outputs.push_back(exe_path);
      link_edge.rule_name = "link_exe";
      link_edge.inputs = object_files;

      if (!target_ldflags.empty())
      {
        link_edge.edge_variables.push_back({"ldflags", target_ldflags});
      }

      ninja.add_edge(link_edge);
    }
    else if (info.kind == EAuxidTargetKind::StaticLib)
    {
      String lib_path = ".auxid/lib/" + info.name + ".a";

      NinjaBuildEdge link_edge;
      link_edge.outputs.push_back(lib_path);
      link_edge.rule_name = "link_static_lib";
      link_edge.inputs = object_files;

      ninja.add_edge(link_edge);
    }
  }

  auto AuxidBuilder_Base::finalize() -> Result<void>
  {
    if (m_run_mode == EAuxidRunMode::DumpMetadata)
    {
      printf("{\n");
      printf("  \"name\": \"%s\",\n", m_package_info.name.c_str());
      printf("  \"dependencies\": [\n");
      for (size_t i = 0; i < m_required_packages.size(); ++i)
      {
        printf("    { \"name\": \"%s\", \"version\": \"%s\" }%s\n", (m_required_packages.begin() + i)->first.c_str(),
               (m_required_packages.begin() + i)->second.c_str(), (i == m_required_packages.size() - 1) ? "" : ",");
      }
      printf("  ]\n");
      printf("}\n");

      return {};
    }

    if (m_run_mode == EAuxidRunMode::GenerateNinja)
    {
      NinjaGenerator ninja;

      ninja.add_global_variable("cxx_compiler", "clang++");
      ninja.add_global_variable("cxx_archiver", "llvm-ar");

      NinjaRule cxx_rule;
      cxx_rule.name = "cxx";
      cxx_rule.command = "$cxx_compiler $cflags -MD -MF $out.d -c $in -o $out";
      cxx_rule.description = "CXX $out";
      cxx_rule.depfile = "$out.d";
      cxx_rule.deps = "gcc";
      ninja.add_rule(cxx_rule);

      NinjaRule link_exe_rule;
      link_exe_rule.name = "link_exe";
      link_exe_rule.command = "$cxx_compiler $ldflags $in -o $out";
      link_exe_rule.description = "LINK $out";
      ninja.add_rule(link_exe_rule);

      NinjaRule link_static_lib_rule;
      link_static_lib_rule.name = "link_static_lib";
      link_static_lib_rule.command = "$cxx_archiver rcs $out $in";
      link_static_lib_rule.description = "Archive $out";
      ninja.add_rule(link_static_lib_rule);

      for (auto &exe : m_executables)
      {
        generate_ninja_for_target(ninja, exe->generate_build_info());
      }
      for (auto &lib : m_static_libs)
      {
        generate_ninja_for_target(ninja, lib->generate_build_info());
      }

      String manifest_content = ninja.generate_manifest();
      printf("!\n%s!\n", manifest_content.c_str());

      return {};
    }

    for (auto t : m_executables)
      delete t;
    m_executables.clear();

    for (auto t : m_static_libs)
      delete t;
    m_static_libs.clear();

    printf("Auxid builder executed standalone. Run via 'auxid build' to compile.\n");
    return {};
  }
} // namespace au::builder