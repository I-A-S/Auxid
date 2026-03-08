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

#define LOG_FUNC_IMPL(name, level)                                                                                     \
  auto Logger::name(const char *fmt, ...) -> void                                                                      \
  {                                                                                                                    \
    va_list args;                                                                                                      \
    va_start(args, fmt);                                                                                               \
    const auto msg = containers::String::vformat(fmt, args);                                                           \
    va_end(args);                                                                                                      \
    m_logger_mutex_ref.lock();                                                                                         \
    m_handler(msg.c_str(), ELevel::LEVEL_##level);                                                                     \
    m_logger_mutex_ref.unlock();                                                                                       \
  }

  LOG_FUNC_IMPL(trace, TRACE);
  LOG_FUNC_IMPL(debug, DEBUG);
  LOG_FUNC_IMPL(info, INFO);
  LOG_FUNC_IMPL(warn, WARN);
  LOG_FUNC_IMPL(error, ERROR);

#undef LOG_FUNC_IMPL

  auto Logger::default_handler(const char *msg, ELevel level) -> void
  {
    switch (level)
    {
    case LEVEL_TRACE:
      fputs(CC_RESET "[TRCE]: ", stdout);
      break;
    case LEVEL_DEBUG:
      fputs(CC_CYAN "[DBUG]: ", stdout);
      break;
    case LEVEL_INFO:
      fputs(CC_GREEN "[INFO]: ", stdout);
      break;
    case LEVEL_WARN:
      fputs(CC_YELLOW "[WARN]: ", stdout);
      break;
    case LEVEL_ERROR:
      fputs(CC_RED "[EROR]: ", stdout);
      break;
    }
    fputs(msg, stdout);
    fputs(CC_RESET "\n", stdout);
  }
} // namespace au