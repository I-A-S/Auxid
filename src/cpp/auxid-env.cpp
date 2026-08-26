// Auxid: The Rigid C++ Platform.
//
// Copyright (C) 2026 I-A-S (ias@iasoft.dev)
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

module;

#include <auxid/macros.hpp>
#include <auxid_win32.hpp>

#include <cerrno>
#include <cstdlib>
#include <string_view>
#include <utility>

#if !AU_PLATFORM_WINDOWS
#  include <unistd.h>
#  if defined(AU_PLATFORM_APPLE)
#    include <mach-o/dyld.h>
#  endif
#endif

module auxid.env;

import auxid.memory;
import auxid.containers;

namespace au::env
{
  namespace
  {
    [[nodiscard]] auto path_from_utf8(StringView sv) -> filesystem::Path
    {
      return filesystem::Path(
          std::u8string_view(reinterpret_cast<const char8_t *>(sv.data()), sv.size()));
    }

#if AU_PLATFORM_WINDOWS

    [[nodiscard]] auto utf8_to_wide(StringView utf8) -> Vec<wchar_t>
    {
      Vec<wchar_t> wide;
      if (!utf8.empty())
      {
        const int needed = ::MultiByteToWideChar(win32::CP_UTF8_, 0, utf8.data(),
                                                 static_cast<int>(utf8.size()), nullptr, 0);
        if (needed > 0)
        {
          wide.resize(static_cast<usize>(needed));
          ::MultiByteToWideChar(win32::CP_UTF8_, 0, utf8.data(), static_cast<int>(utf8.size()),
                                wide.data(), needed);
        }
      }
      wide.push_back(L'\0');
      return wide;
    }

    [[nodiscard]] auto wide_to_utf8(const wchar_t *wide, usize wide_len) -> String
    {
      if (wide_len == 0)
        return String();
      const int needed = ::WideCharToMultiByte(win32::CP_UTF8_, 0, wide, static_cast<int>(wide_len),
                                               nullptr, 0, nullptr, nullptr);
      if (needed <= 0)
        return String();
      Vec<char> utf8(static_cast<usize>(needed));
      ::WideCharToMultiByte(win32::CP_UTF8_, 0, wide, static_cast<int>(wide_len), utf8.data(), needed,
                            nullptr, nullptr);
      return String(utf8.data(), utf8.size());
    }

#endif
  } // namespace

#if AU_PLATFORM_WINDOWS

  AUXID_API auto find(StringView name) -> Option<String>
  {
    const Vec<wchar_t> wname = utf8_to_wide(name);

    unsigned long size = ::GetEnvironmentVariableW(wname.data(), nullptr, 0);
    if (size == 0)
      return {}; // includes ERROR_ENVVAR_NOT_FOUND

    // size includes the terminator on the query call. Loop in case the value
    // changes between the two calls.
    for (int attempt = 0; attempt < 4; attempt++)
    {
      Vec<wchar_t> buffer(static_cast<usize>(size));
      const unsigned long written = ::GetEnvironmentVariableW(wname.data(), buffer.data(), size);
      if (written == 0)
        return {};
      if (written < size)
        return wide_to_utf8(buffer.data(), static_cast<usize>(written));
      size = written + 1;
    }
    return {};
  }

  AUXID_API auto set(StringView name, StringView value) -> Result<void>
  {
    const Vec<wchar_t> wname = utf8_to_wide(name);
    const Vec<wchar_t> wvalue = utf8_to_wide(value);
    if (::SetEnvironmentVariableW(wname.data(), wvalue.data()) == 0)
      return fail(std::move(Error::os_last()).ctx("env::set"));
    return {};
  }

  AUXID_API auto unset(StringView name) -> Result<void>
  {
    const Vec<wchar_t> wname = utf8_to_wide(name);
    if (::SetEnvironmentVariableW(wname.data(), nullptr) == 0)
    {
      Error err = Error::os_last();
      if (static_cast<unsigned long>(err.code) == win32::ERROR_ENVVAR_NOT_FOUND_)
        return {}; // unsetting a missing variable is success (contract)
      return fail(std::move(err).ctx("env::unset"));
    }
    return {};
  }

  AUXID_API auto executable_path() -> Result<filesystem::Path>
  {
    Vec<wchar_t> buffer(512);
    for (int attempt = 0; attempt < 8; attempt++)
    {
      const unsigned long written =
          ::GetModuleFileNameW(nullptr, buffer.data(), static_cast<unsigned long>(buffer.size()));
      if (written == 0)
        return fail(std::move(Error::os_last()).ctx("env::executable_path"));
      if (static_cast<usize>(written) < buffer.size())
        return filesystem::Path(buffer.data(), buffer.data() + written);
      buffer.resize(buffer.size() * 2);
    }
    return fail("env::executable_path: path length is unreasonable");
  }

#else

  AUXID_API auto find(StringView name) -> Option<String>
  {
    const String zname(name);
    const char *value = ::getenv(zname.c_str());
    if (value == nullptr)
      return {};
    return String(value);
  }

  AUXID_API auto set(StringView name, StringView value) -> Result<void>
  {
    const String zname(name);
    const String zvalue(value);
    if (::setenv(zname.c_str(), zvalue.c_str(), 1) != 0)
      return fail(Error::os(errno).ctx("env::set"));
    return {};
  }

  AUXID_API auto unset(StringView name) -> Result<void>
  {
    const String zname(name);
    if (::unsetenv(zname.c_str()) != 0)
      return fail(Error::os(errno).ctx("env::unset"));
    return {};
  }

  AUXID_API auto executable_path() -> Result<filesystem::Path>
  {
#  if defined(AU_PLATFORM_APPLE)
    u32 size = 0;
    ::_NSGetExecutablePath(nullptr, &size); // reports required size
    Vec<char> buffer(static_cast<usize>(size) + 1);
    if (::_NSGetExecutablePath(buffer.data(), &size) != 0)
      return fail("env::executable_path: _NSGetExecutablePath failed after sizing");
    return path_from_utf8(StringView(buffer.data(), compiler::strlen(buffer.data())));
#  else
    Vec<char> buffer(1024);
    for (int attempt = 0; attempt < 8; attempt++)
    {
      const isize written = ::readlink("/proc/self/exe", buffer.data(), buffer.size());
      if (written < 0)
        return fail(Error::os(errno).ctx("env::executable_path"));
      if (static_cast<usize>(written) < buffer.size())
        return path_from_utf8(StringView(buffer.data(), static_cast<usize>(written)));
      buffer.resize(buffer.size() * 2);
    }
    return fail("env::executable_path: path length is unreasonable");
#  endif
  }

#endif

  AUXID_API auto get_or(StringView name, StringView fallback) -> String
  {
    auto found = find(name);
    if (found.has_value())
      return std::move(*found);
    return String(fallback);
  }

  namespace
  {
    [[nodiscard]] auto required_var(const char *name) -> Result<String>
    {
      auto found = find(name);
      if (!found.has_value())
        return fail("standard_dir: required environment variable {} is not set", name);
      return std::move(*found);
    }

    [[nodiscard]] auto temp_base() -> Result<filesystem::Path>
    {
#if AU_PLATFORM_WINDOWS
      auto temp = find("TEMP");
      if (!temp.has_value())
        temp = find("TMP");
      if (!temp.has_value())
        return fail("standard_dir: neither TEMP nor TMP is set");
      return path_from_utf8(StringView(temp->data(), temp->size()));
#else
      auto temp = find("TMPDIR");
      if (temp.has_value())
        return path_from_utf8(StringView(temp->data(), temp->size()));
      return filesystem::Path("/tmp");
#endif
    }

    [[nodiscard]] auto var_or_home_suffix(const char *var, const char *home_suffix)
        -> Result<filesystem::Path>
    {
      auto explicit_dir = find(var);
      if (explicit_dir.has_value())
        return path_from_utf8(StringView(explicit_dir->data(), explicit_dir->size()));
      AU_TRY_VAR(home, required_var("HOME"));
      return path_from_utf8(StringView(home.data(), home.size())) / home_suffix;
    }
  } // namespace

  // The per-OS mapping table is normative in docs/OS-CONTRACT.md; this is it.
  AUXID_API auto standard_dir(StandardDir dir, StringView app_name) -> Result<filesystem::Path>
  {
    const filesystem::Path app = path_from_utf8(app_name);

#if AU_PLATFORM_WINDOWS
    switch (dir)
    {
    case StandardDir::Home: {
      AU_TRY_VAR(home, required_var("USERPROFILE"));
      return path_from_utf8(StringView(home.data(), home.size()));
    }
    case StandardDir::Temp:
      return temp_base();
    case StandardDir::Config: {
      AU_TRY_VAR(base, required_var("APPDATA"));
      return path_from_utf8(StringView(base.data(), base.size())) / app;
    }
    case StandardDir::Data: {
      AU_TRY_VAR(base, required_var("LOCALAPPDATA"));
      return path_from_utf8(StringView(base.data(), base.size())) / app;
    }
    case StandardDir::Cache: {
      AU_TRY_VAR(base, required_var("LOCALAPPDATA"));
      return path_from_utf8(StringView(base.data(), base.size())) / app / "cache";
    }
    case StandardDir::Runtime: {
      AU_TRY_VAR(base, required_var("LOCALAPPDATA"));
      return path_from_utf8(StringView(base.data(), base.size())) / app / "runtime";
    }
    }
#elif defined(AU_PLATFORM_APPLE)
    switch (dir)
    {
    case StandardDir::Home: {
      AU_TRY_VAR(home, required_var("HOME"));
      return path_from_utf8(StringView(home.data(), home.size()));
    }
    case StandardDir::Temp:
      return temp_base();
    case StandardDir::Config:
    case StandardDir::Data: {
      AU_TRY_VAR(home, required_var("HOME"));
      return path_from_utf8(StringView(home.data(), home.size())) / "Library/Application Support" / app;
    }
    case StandardDir::Cache: {
      AU_TRY_VAR(home, required_var("HOME"));
      return path_from_utf8(StringView(home.data(), home.size())) / "Library/Caches" / app;
    }
    case StandardDir::Runtime: {
      AU_TRY_VAR(temp, temp_base());
      return temp / app;
    }
    }
#else
    switch (dir)
    {
    case StandardDir::Home: {
      AU_TRY_VAR(home, required_var("HOME"));
      return path_from_utf8(StringView(home.data(), home.size()));
    }
    case StandardDir::Temp:
      return temp_base();
    case StandardDir::Config: {
      AU_TRY_VAR(base, var_or_home_suffix("XDG_CONFIG_HOME", ".config"));
      return base / app;
    }
    case StandardDir::Data: {
      AU_TRY_VAR(base, var_or_home_suffix("XDG_DATA_HOME", ".local/share"));
      return base / app;
    }
    case StandardDir::Cache: {
      AU_TRY_VAR(base, var_or_home_suffix("XDG_CACHE_HOME", ".cache"));
      return base / app;
    }
    case StandardDir::Runtime: {
      auto runtime = find("XDG_RUNTIME_DIR");
      if (runtime.has_value())
        return path_from_utf8(StringView(runtime->data(), runtime->size())) / app;
      AU_TRY_VAR(temp, temp_base());
      return temp / app; // honest fallback, documented in the contract
    }
    }
#endif

    return fail("standard_dir: unknown StandardDir value");
  }
} // namespace au::env
