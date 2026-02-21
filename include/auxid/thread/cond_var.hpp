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

#include <auxid/thread/mutex.hpp>

namespace au
{
  class ConditionVariable
  {
    cnd_t m_handle;

public:
    ConditionVariable(const ConditionVariable &) = delete;
    ConditionVariable &operator=(const ConditionVariable &) = delete;

    ConditionVariable()
    {
      cnd_init(&m_handle);
    }

    ~ConditionVariable()
    {
      cnd_destroy(&m_handle);
    }

    void notify_one()
    {
      cnd_signal(&m_handle);
    }

    void notify_all()
    {
      cnd_broadcast(&m_handle);
    }

    void wait(Mutex &mutex)
    {
      cnd_wait(&m_handle, mutex.native_handle());
    }

    template<typename Predicate> void wait(Mutex &mutex, Predicate stop_waiting)
    {
      while (!stop_waiting())
      {
        wait(mutex);
      }
    }
  };
} // namespace au