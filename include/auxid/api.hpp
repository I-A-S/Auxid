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

#pragma once

#if defined(AUXID_SHARED_BUILD)
#  if defined(_WIN32) || defined(__CYGWIN__)
#    if defined(AUXID_BUILDING_LIBRARY)
#      define AUXID_API __declspec(dllexport)
#    else
#      define AUXID_API __declspec(dllimport)
#    endif
#  elif defined(AUXID_BUILDING_LIBRARY)
#    define AUXID_API __attribute__((visibility("default")))
#  else
#    define AUXID_API
#  endif
#else
#  define AUXID_API
#endif
