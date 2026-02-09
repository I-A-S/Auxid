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

#include <fixpoint/fixpoint.hpp>

namespace auxid
{
  using namespace ia;

  class ViolationReporter
  {
public:
    template<typename... Args>
    static auto report_decl_violation(fixpoint::FullSourceLoc full_loc, const fixpoint::VarDecl *decl,
                                      std::format_string<Args...> fmt, ForwardRef<Args>... args) -> void
    {
      internal_report_decl_violation(full_loc, decl, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    static auto report_ref_violation(fixpoint::FullSourceLoc full_loc, const fixpoint::DeclRefExpr *ref,
                                     std::format_string<Args...> fmt, ForwardRef<Args>... args) -> void
    {
      internal_report_ref_violation(full_loc, ref, std::format(fmt, std::forward<Args>(args)...));
    }

private:
    static auto internal_report_decl_violation(fixpoint::FullSourceLoc full_loc, const fixpoint::VarDecl *decl,
                                               Ref<String> message) -> void;
    static auto internal_report_ref_violation(fixpoint::FullSourceLoc full_loc, const fixpoint::DeclRefExpr *ref,
                                              Ref<String> message) -> void;
  };
} // namespace auxid