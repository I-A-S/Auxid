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

// The ONLY sanctioned source of Win32/CRT declarations for Auxid module
// units. <windows.h> must never appear in a module unit's global fragment:
// mingw's winnt.h drags in the x86 intrinsics headers, which collide with
// the same headers snapshotted inside auxid.containers under GCC's modules
// implementation ("conflicting language linkage for imported declaration").
// Signatures below are canonical; 64-bit Windows has a single calling
// convention, so the bare prototypes are exact. Add entries here, never
// include <windows.h>.

#pragma once

#include <auxid/macros.hpp>

#if AU_PLATFORM_WINDOWS

extern "C"
{
  // kernel32 — errors
  unsigned long __stdcall GetLastError(void);

  // kernel32 — handles & threads
  int __stdcall CloseHandle(void *handle);
  unsigned long __stdcall WaitForSingleObject(void *handle, unsigned long milliseconds);
  unsigned long __stdcall GetCurrentThreadId(void);

  // CRT — thread spawn
  unsigned long long __cdecl _beginthreadex(void *security, unsigned stack_size,
                                            unsigned(__stdcall *start_address)(void *), void *arglist,
                                            unsigned initflag, unsigned *thrdaddr);

  // kernel32 — UTF-8 <-> UTF-16
  int __stdcall MultiByteToWideChar(unsigned int code_page, unsigned long flags, const char *multi_byte,
                                    int multi_byte_len, wchar_t *wide, int wide_len);
  int __stdcall WideCharToMultiByte(unsigned int code_page, unsigned long flags, const wchar_t *wide,
                                    int wide_len, char *multi_byte, int multi_byte_len,
                                    const char *default_char, int *used_default_char);

  // kernel32 — environment & module paths
  unsigned long __stdcall GetEnvironmentVariableW(const wchar_t *name, wchar_t *buffer,
                                                  unsigned long buffer_size);
  int __stdcall SetEnvironmentVariableW(const wchar_t *name, const wchar_t *value);
  unsigned long __stdcall GetModuleFileNameW(void *module, wchar_t *filename, unsigned long size);
}

namespace au::win32
{
  inline constexpr unsigned int CP_UTF8_ = 65001;
  inline constexpr unsigned long ERROR_ENVVAR_NOT_FOUND_ = 203;
  inline constexpr unsigned long ERROR_INSUFFICIENT_BUFFER_ = 122;
} // namespace au::win32

#endif // AU_PLATFORM_WINDOWS
