// Auxid: The Rigid C++ Platform.
//
// Copyright (C) 2026 I-A-S (ias@iasoft.dev)
// Copyright (C) 2026 IASoft (PVT) LTD (contact@iasoft.dev)
//
// This source code is licensed under the PolyForm Noncommercial License 1.0.0.
// A copy of this license is included in the LICENSE file at the root of this project,
// and is also available at <https://polyformproject.org/licenses/noncommercial/1.0.0>.

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

  // kernel32 — anonymous pipes & file I/O (security attributes passed as
  // void*; Auxid only ever passes nullptr)
  int __stdcall CreatePipe(void **read_pipe, void **write_pipe, void *security_attributes,
                           unsigned long size);
  int __stdcall ReadFile(void *handle, void *buffer, unsigned long to_read, unsigned long *read,
                         void *overlapped);
  int __stdcall WriteFile(void *handle, const void *buffer, unsigned long to_write,
                          unsigned long *written, void *overlapped);
  int __stdcall FlushFileBuffers(void *handle);

  // kernel32 — named pipes
  void *__stdcall CreateNamedPipeW(const wchar_t *name, unsigned long open_mode,
                                   unsigned long pipe_mode, unsigned long max_instances,
                                   unsigned long out_buffer_size, unsigned long in_buffer_size,
                                   unsigned long default_timeout, void *security_attributes);
  int __stdcall ConnectNamedPipe(void *handle, void *overlapped);
  int __stdcall DisconnectNamedPipe(void *handle);
  void *__stdcall CreateFileW(const wchar_t *name, unsigned long desired_access,
                              unsigned long share_mode, void *security_attributes,
                              unsigned long creation_disposition, unsigned long flags,
                              void *template_file);
  int __stdcall WaitNamedPipeW(const wchar_t *name, unsigned long timeout_ms);

  // kernel32 — named mutex (single-instance arbitration)
  void *__stdcall CreateMutexW(void *security_attributes, int initial_owner, const wchar_t *name);
}

namespace au::win32
{
  inline constexpr unsigned int CP_UTF8_ = 65001;
  inline constexpr unsigned long ERROR_ENVVAR_NOT_FOUND_ = 203;
  inline constexpr unsigned long ERROR_INSUFFICIENT_BUFFER_ = 122;
  inline constexpr unsigned long ERROR_ALREADY_EXISTS_ = 183;
  inline constexpr unsigned long ERROR_BROKEN_PIPE_ = 109;
  inline constexpr unsigned long ERROR_NO_DATA_ = 232;
  inline constexpr unsigned long ERROR_PIPE_BUSY_ = 231;
  inline constexpr unsigned long ERROR_PIPE_CONNECTED_ = 535;
  inline constexpr unsigned long GENERIC_READ_ = 0x80000000ul;
  inline constexpr unsigned long GENERIC_WRITE_ = 0x40000000ul;
  inline constexpr unsigned long OPEN_EXISTING_ = 3;
  inline constexpr unsigned long PIPE_ACCESS_DUPLEX_ = 0x00000003ul;
  inline constexpr unsigned long PIPE_TYPE_BYTE_STREAM_ = 0x00000000ul; // TYPE_BYTE|READMODE_BYTE|WAIT
  inline constexpr unsigned long PIPE_UNLIMITED_INSTANCES_ = 255;

  inline void *invalid_handle() noexcept
  {
    return reinterpret_cast<void *>(-1ll);
  }
} // namespace au::win32

#endif // AU_PLATFORM_WINDOWS
