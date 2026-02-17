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

#include <cli.hpp>

#include <vendor/cofyc/argparse.h>

namespace au
{
  auto CLI::parse(i32 argc, char *argv[]) -> Result<ECommand>
  {
    static const char *const usages[] = {
        "auxid <command> [args]",
        NULL,
    };

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, ARGPARSE_STOP_AT_NON_OPTION);
    argc = argparse_parse(&argparse, argc, (const char **) argv);

    if (argc == 0)
      return fail("no command provided. Run 'auxid --help' for usage.");

    const char *cmd = argv[0];

    argc--;
    argv++;

    m_args.clear();
    m_args.reserve(argc);
    for (i32 i = 0; i < argc; ++i)
    {
      m_args.push_back(String(argv[i]));
    }

    if (std::strcmp(cmd, "new") == 0)
    {
      if (m_args.size() != 1)
        return fail("'new' requires exactly 1 argument <name>");
      return ECommand::Workspace_New;
    }
    else if (std::strcmp(cmd, "build") == 0)
    {
      if (m_args.size() > 0)
        return fail("'build' takes no arguments");
      return ECommand::Workspace_Build;
    }
    else if (std::strcmp(cmd, "clean") == 0)
    {
      if (m_args.size() > 0)
        return fail("'clean' takes no arguments");
      return ECommand::Workspace_Clean;
    }
    else if (std::strcmp(cmd, "repair") == 0)
    {
      if (m_args.size() > 0)
        return fail("'repair' takes no arguments");
      return ECommand::Workspace_Repair;
    }
    else if (std::strcmp(cmd, "install") == 0)
    {
      if (m_args.size() == 0)
        return ECommand::Package_InstallAll;
      if (m_args.size() == 1)
        return ECommand::Package_Install;
      return fail("'install' takes 0 or 1 argument <pkg_name>");
    }
    else if (std::strcmp(cmd, "remove") == 0)
    {
      if (m_args.size() != 1)
        return fail("'remove' requires exactly 1 argument <pkg_name>");
      return ECommand::Package_Remove;
    }
    else if (std::strcmp(cmd, "update") == 0)
    {
      if (m_args.size() != 1)
        return fail("'update' requires exactly 1 argument <pkg_name>");
      return ECommand::Package_Update;
    }

    return fail("unknown command: '%s'", cmd);
  }

  auto CLI::get_arg(i32 index) const -> Result<String>
  {
    if (index < 0 || static_cast<size_t>(index) >= m_args.size())
      return fail("argument index %d is out of bounds", index);

    return m_args[index];
  }
} // namespace au