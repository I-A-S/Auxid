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

#include <condition_variable>
#include <format>
#include <functional>
#include <mutex>
#include <thread>
#include <tuple>
#include <utility>

export module auxid.thread;

export import auxid.core;
import auxid.containers;

export namespace au
{
  template<typename MutexType> class LockGuard
  {
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
    auto operator=(const LockGuard &) -> LockGuard & = delete;

private:
    MutexType &m_mutex;
  };

  class ConditionVariable
  {
public:
    ConditionVariable() noexcept = default;
    ~ConditionVariable() = default;
    ConditionVariable(const ConditionVariable &) = delete;
    auto operator=(const ConditionVariable &) -> ConditionVariable & = delete;

    auto notify_one() noexcept -> void
    {
      m_cv.notify_one();
    }

    auto notify_all() noexcept -> void
    {
      m_cv.notify_all();
    }

    auto wait(Mutex &mutex) -> void
    {
      m_cv.wait(mutex);
    }

    template<typename Predicate> auto wait(Mutex &mutex, Predicate stop_waiting) -> void
    {
      m_cv.wait(mutex, std::move(stop_waiting));
    }

private:
    std::condition_variable_any m_cv{};
  };

  class AUXID_API Logger
  {
public:
    enum ELevel
    {
      LEVEL_TRACE,
      LEVEL_DEBUG,
      LEVEL_INFO,
      LEVEL_WARN,
      LEVEL_ERROR
    };

    typedef void (*LogHandler_FuncT)(const char *msg, ELevel level);

public:
    template<typename... Args> auto trace(std::format_string<Args...> fmt, Args &&...args) -> void
    {
      log_impl(LEVEL_TRACE, fmt.get(), std::make_format_args(args...));
    }

    template<typename... Args> auto debug(std::format_string<Args...> fmt, Args &&...args) -> void
    {
      log_impl(LEVEL_DEBUG, fmt.get(), std::make_format_args(args...));
    }

    template<typename... Args> auto info(std::format_string<Args...> fmt, Args &&...args) -> void
    {
      log_impl(LEVEL_INFO, fmt.get(), std::make_format_args(args...));
    }

    template<typename... Args> auto warn(std::format_string<Args...> fmt, Args &&...args) -> void
    {
      log_impl(LEVEL_WARN, fmt.get(), std::make_format_args(args...));
    }

    template<typename... Args> auto error(std::format_string<Args...> fmt, Args &&...args) -> void
    {
      log_impl(LEVEL_ERROR, fmt.get(), std::make_format_args(args...));
    }

public:
    Logger(Mutex &logger_mutex);

    auto set_log_handler(LogHandler_FuncT handler) -> void
    {
      m_handler = handler;
    }

private:
    auto log_impl(ELevel level, StringView fmt, std::format_args args) -> void;
    static auto default_handler(const char *msg, ELevel level) -> void;

    Mutex &m_logger_mutex_ref;
    LogHandler_FuncT m_handler{default_handler};
  };
} // namespace au

export namespace au::auxid
{
  AUXID_API auto initialize_main_thread() -> void;
  AUXID_API auto terminate_main_thread() -> void;
  AUXID_API auto initialize_worker_thread() -> void;
  AUXID_API auto terminate_worker_thread() -> void;

  struct MainThreadGuard
  {
    MainThreadGuard()
    {
      initialize_main_thread();
    }

    ~MainThreadGuard()
    {
      terminate_main_thread();
    }
  };

  struct WorkerThreadGuard
  {
    WorkerThreadGuard()
    {
      initialize_worker_thread();
    }

    ~WorkerThreadGuard()
    {
      terminate_worker_thread();
    }
  };

  AUXID_API auto is_main_thread() -> bool;
  AUXID_API auto is_thread_initialized() -> bool;
  AUXID_API auto get_thread_logger() -> Logger &;
} // namespace au::auxid

export namespace au
{
  template<bool JoinOnDestroy> class ThreadT
  {
public:
    using ThreadID = std::thread::id;

    template<typename F, typename... Args> static auto create(F &&f, Args &&...args) -> Result<ThreadT>
    {
      auto wrap = [func = std::forward<F>(f), tup = std::make_tuple(std::forward<Args>(args)...)]() mutable {
        auxid::WorkerThreadGuard _g;
        std::apply(std::move(func), std::move(tup));
      };
      return ThreadT(std::thread(std::move(wrap)));
    }

    static auto get_calling_thread_id() noexcept -> ThreadID
    {
      return std::this_thread::get_id();
    }

    ThreadT() noexcept = default;

    ThreadT(const ThreadT &) = delete;
    auto operator=(const ThreadT &) -> ThreadT & = delete;

    ThreadT(ThreadT &&other) noexcept : m_thread(std::move(other.m_thread))
    {
    }

    auto operator=(ThreadT &&other) noexcept -> ThreadT &
    {
      if (this != &other)
      {
        if (m_thread.joinable())
        {
          if constexpr (JoinOnDestroy)
            m_thread.join();
          else
            panic("ThreadT move-assigned over a still-joinable thread");
        }
        m_thread = std::move(other.m_thread);
      }
      return *this;
    }

    ~ThreadT()
    {
      if (m_thread.joinable())
      {
        if constexpr (JoinOnDestroy)
          m_thread.join();
        else
          panic("ThreadT destroyed while still joinable (use JThread or call join())");
      }
    }

    [[nodiscard]] auto joinable() const noexcept -> bool
    {
      return m_thread.joinable();
    }

    auto join() -> void
    {
      if (m_thread.joinable())
        m_thread.join();
    }

    [[nodiscard]] auto get_id() const noexcept -> ThreadID
    {
      return m_thread.get_id();
    }

private:
    explicit ThreadT(std::thread &&t) noexcept : m_thread(std::move(t))
    {
    }

    std::thread m_thread{};
  };

  using Thread = ThreadT<false>;
  using JThread = ThreadT<true>;
} // namespace au

export namespace au::containers
{
  template<> struct Hash<std::thread::id>
  {
    auto operator()(std::thread::id id) const noexcept -> u64
    {
      return static_cast<u64>(std::hash<std::thread::id>{}(id));
    }
  };
} // namespace au::containers
