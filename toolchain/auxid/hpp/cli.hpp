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

#include <auxid/auxid.hpp>

#include <auxid/containers/vec.hpp>

namespace au
{
  class CLI
  {
public:
    enum class ECommand
    {
      Workspace_New,
      Workspace_Build,
      Workspace_Clean,
      Workspace_Repair,

      Package_Install,
      Package_InstallAll,
      Package_Remove,
      Package_Update,
    };

public:
    auto parse(i32 argc, char *argv[]) -> Result<ECommand>;

    [[nodiscard]] auto get_arg(i32 index) const -> Result<String>;

private:
    Vec<String> m_args;
  };
} // namespace au