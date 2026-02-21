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
#include <auxid/containers/pair.hpp>

namespace au::platform
{
  auto create_directory(StringView path) -> Result<void>;
  auto remove_directory(StringView path, bool recursive) -> Result<void>;

  auto is_file(StringView path) -> bool;
  auto is_directory(StringView path) -> bool;
  auto is_file_or_directory(StringView path) -> bool;
  auto get_file_modify_time(StringView path) -> Result<u64>;

  auto change_dir(StringView path) -> Result<void>;

  auto download_file(StringView url, StringView dst_path) -> Result<void>;
  auto spawn_process(std::initializer_list<StringView> command_line) -> Result<Pair<i32, String>>;
} // namespace au::platform