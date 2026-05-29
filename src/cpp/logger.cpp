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

#include <format>
#include <print>

module auxid.thread;

namespace au
{
#define CC_RESET "\033[0m"
#define CC_RED "\033[31m"
#define CC_GREEN "\033[32m"
#define CC_YELLOW "\033[33m"
#define CC_BLUE "\033[34m"
#define CC_MAGENTA "\033[35m"
#define CC_CYAN "\033[36m"

  Logger::Logger(Mutex &logger_mutex) : m_logger_mutex_ref(logger_mutex)
  {
  }

  auto Logger::log_impl(ELevel level, StringView fmt, std::format_args args) -> void
  {
    const auto msg = String::vformat(fmt, args);
    m_logger_mutex_ref.lock();
    m_handler(msg.c_str(), level);
    m_logger_mutex_ref.unlock();
  }

  auto Logger::default_handler(const char *msg, ELevel level) -> void
  {
    switch (level)
    {
    case LEVEL_TRACE:
      std::println(stdout, CC_RESET "[TRCE]: {}" CC_RESET, msg);
      break;
    case LEVEL_DEBUG:
      std::println(stdout, CC_CYAN "[DBUG]: {}" CC_RESET, msg);
      break;
    case LEVEL_INFO:
      std::println(stdout, CC_GREEN "[INFO]: {}" CC_RESET, msg);
      break;
    case LEVEL_WARN:
      std::println(stdout, CC_YELLOW "[WARN]: {}" CC_RESET, msg);
      break;
    case LEVEL_ERROR:
      std::println(stdout, CC_RED "[EROR]: {}" CC_RESET, msg);
      break;
    }
  }
} // namespace au
