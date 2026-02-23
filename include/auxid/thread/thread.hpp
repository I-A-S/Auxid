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

#include <auxid/auxid.hpp>

#include <auxid/vendor/tinycthread/tinycthread.h>

namespace au
{
  template<bool JoinOnDestroy> class ThreadT
  {
public:
    using ThreadID = thrd_t;

private:
    thrd_t m_handle;
    bool m_joinable = false;

    struct ThreadData
    {
      virtual ~ThreadData() = default;
      virtual void run() = 0;
      virtual void self_destroy() = 0;
    };

    template<typename Func> struct ThreadDataImpl : ThreadData
    {
      Func m_func;

      ThreadDataImpl(Func &&func) : m_func(std::move(func))
      {
      }

      auto run() -> void override
      {
        struct ThreadGuard
        {
          ThreadGuard()
          {
            auxid::initialize_worker_thread();
          }

          ~ThreadGuard()
          {
            auxid::terminate_worker_thread();
          }
        } _thread_guard;

        m_func();
      }

      void self_destroy() override
      {
        memory::HeapAllocator allocator;

        au::destroy_at(this);

        allocator.free(this, sizeof(ThreadDataImpl<Func>), alignof(ThreadDataImpl<Func>));
      }
    };

    static auto thread_proxy(void *arg) -> int
    {
      auto *data = static_cast<ThreadData *>(arg);
      data->run();
      data->self_destroy();
      return 0;
    }

public:
    template<typename F, typename... Args> static auto create(F &&f, Args &&...args) -> Result<ThreadT>
    {
      auto lambda = [func = std::forward<F>(f), args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
        std::apply(func, std::move(args));
      };

      memory::HeapAllocator allocator;

      auto data = (ThreadDataImpl<decltype(lambda)> *) allocator.alloc(sizeof(ThreadDataImpl<decltype(lambda)>));
      construct_at(data, std::move(lambda));

      thrd_t handle;
      const auto error = thrd_create(&handle, thread_proxy, data);
      if (error == thrd_success)
        return std::move(ThreadT(handle));

      au::destroy_at(data);
      allocator.free(data, sizeof(ThreadDataImpl<decltype(lambda)>), alignof(ThreadDataImpl<decltype(lambda)>));

      switch (error)
      {
      case thrd_timedout:
        return fail("failed to create thread: timed out");

      case thrd_busy:
        return fail("failed to create thread: resource busy");

      case thrd_nomem:
        return fail("failed to create thread: out of memory");

      default:
        break;
      }

      return fail("failed to create thread: unknown error");
    }

    static auto get_calling_thread_id() -> ThreadID
    {
      return thrd_current();
    }

public:
    ThreadT(const ThreadT &) = delete;
    ThreadT &operator=(const ThreadT &) = delete;

    ThreadT(ThreadT &&other) noexcept : m_handle(other.m_handle), m_joinable(other.m_joinable)
    {
      other.m_joinable = false;
    }

    ThreadT &operator=(ThreadT &&other) noexcept
    {
      assert(!joinable());
      m_handle = other.m_handle;
      m_joinable = other.m_joinable;
      other.m_joinable = false;
      return *this;
    }

    ~ThreadT()
    {
      if constexpr (JoinOnDestroy)
      {
        join();
      }
    }

    [[nodiscard]] auto joinable() const -> bool
    {
      return m_joinable;
    }

    auto join() -> void
    {
      if (joinable())
      {
        thrd_join(m_handle, nullptr);
        m_joinable = false;
      }
    }

    [[nodiscard]] auto get_id() const -> ThreadID
    {
      return m_handle;
    }

private:
    ThreadT(thrd_t handle) : m_handle(handle), m_joinable(true)
    {
    }
  };

  // Non Joining Thread
  using Thread = ThreadT<false>;
  // Joining Thread
  using JThread = ThreadT<true>;
} // namespace au