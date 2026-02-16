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

#include <auxid/result.hpp>

#include <auxid/vendor/tinycthread/tinycthread.h>

namespace au
{
  class Mutex
  {
    mtx_t m_handle;

public:
    Mutex(const Mutex &) = delete;
    Mutex &operator=(const Mutex &) = delete;

    Mutex(bool recursive = false)
    {
      mtx_init(&m_handle, recursive ? mtx_recursive : mtx_plain);
    }

    ~Mutex()
    {
      mtx_destroy(&m_handle);
    }

    void lock()
    {
      mtx_lock(&m_handle);
    }

    void unlock()
    {
      mtx_unlock(&m_handle);
    }

    bool try_lock()
    {
      return (mtx_trylock(&m_handle) == thrd_success);
    }

    mtx_t *native_handle()
    {
      return &m_handle;
    }
  };

  template<typename MutexType> class LockGuard
  {
    MutexType &m_mutex;

public:
    explicit LockGuard(MutexType &m) : m_mutex(m)
    {
      m_mutex.lock();
    }

    ~LockGuard()
    {
      m_mutex.unlock();
    }

    LockGuard(const LockGuard &) = delete;
    LockGuard &operator=(const LockGuard &) = delete;
  };
} // namespace au