// Auxid: Rust like safety and syntax for C++
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

#include <PCH.hpp>

#define REGISTER_MATCHER(name, ...)                                                                                    \
  class __matcher_##name : public MatchFinder::MatchCallback                                                           \
  {                                                                                                                    \
public:                                                                                                                \
    virtual void run(Auxid::Ref<MatchFinder::MatchResult> result) override;                                            \
  };                                                                                                                   \
  Auxid::ValidationMatcher __##name{__VA_ARGS__, Auxid::make_box<__matcher_##name>()};                                 \
  void __matcher_##name ::run(Auxid::Ref<MatchFinder::MatchResult> result)

namespace auxid
{
  struct ValidationMatcher
  {
    DeclarationMatcher pattern;
    Box<MatchFinder::MatchCallback> callback;

    ValidationMatcher(ForwardRef<DeclarationMatcher> pattern, ForwardRef<Box<MatchFinder::MatchCallback>> callback);
  };

  class Validator
  {
public:
    static Validator &instance()
    {
      static Validator s_instance{};
      return s_instance;
    }

    ~Validator() = default;

public:
    auto run(i32 argc, const char *argv[]) -> Result<i32>;

    auto add_matcher(ValidationMatcher *matcher) -> void;

private:
    Vec<ValidationMatcher *> m_matchers;

    auto get_clang_resource_dir() -> String;

private:
    Validator() = default;
  };
} // namespace auxid