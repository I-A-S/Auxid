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

#include <workspace/workspace.hpp>

#include <platform/platform.hpp>

namespace au::workspace
{
  auto create_new(StringView name) -> Result<void>
  {
    if (platform::is_file_or_directory(name))
      return fail("file named '%s' already exists", name.data());

    AU_TRY_PURE(platform::create_directory(name));
    AU_TRY_PURE(platform::create_directory(String::format("%s/src", name)));
    AU_TRY_PURE(platform::create_directory(String::format("%s/include", name)));
    AU_TRY_PURE(platform::create_directory(String::format("%s/config", name)));

    AU_TRY_PURE(platform::change_dir(name));

    AU_TRY_PURE(platform::download_file("https://pkg.auxid.dev/libauxid.zip", "./libauxid.zip"));

    return {};
  }

  auto build() -> Result<void>
  {
    return {};
  }

  auto clean() -> Result<void>
  {
    return {};
  }

  auto repair() -> Result<void>
  {
    return {};
  }
} // namespace au::workspace