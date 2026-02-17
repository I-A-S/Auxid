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

// https://pkg.auxid.dev/libauxid.zip

namespace au
{
  auto main(i32 argc, char *argv[]) -> Result<void>
  {
    CLI cli;

    const auto cmd = AU_TRY(cli.parse(argc, argv));

    switch (cmd)
    {
    case CLI::ECommand::Workspace_New:
      break;
    case CLI::ECommand::Workspace_Build:
      break;
    case CLI::ECommand::Workspace_Clean:
      break;
    case CLI::ECommand::Workspace_Repair:
      break;

    case CLI::ECommand::Package_Install:
      break;
    case CLI::ECommand::Package_InstallAll:
      break;
    case CLI::ECommand::Package_Remove:
      break;
    case CLI::ECommand::Package_Update:
      break;
    }

    return {};
  }
} // namespace au

extern "C" int main(int argc, char *argv[])
{
  au::auxid::initialize_main_thread();
  const auto result = au::main(argc, argv);
  if (!result)
  {
    printf("[FATAL ERROR]: %s\n", result.error().c_str());
    au::auxid::terminate_main_thread();
    return -1;
  }
  au::auxid::terminate_main_thread();
  return 0;
}