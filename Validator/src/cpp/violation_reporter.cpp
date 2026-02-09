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

#include <violation_reporter.hpp>

namespace auxid
{
  // [IANOTE]: Evaluate whether if these functions are actually needed. Might as
  // well use the template func for the impls.

  auto ViolationReporter::internal_report_decl_violation(fixpoint::FullSourceLoc full_loc,
                                                         const fixpoint::VarDecl *decl, Ref<String> message) -> void
  {
    const auto location = fixpoint::utils::get_loc_str_path_and_line(full_loc);
    const auto columns = fixpoint::utils::get_decl_str_start_and_end_cols(decl);
    llvm::outs() << location << ":" << columns << ": [Auxid] Violation: " << message << "\n";
  }

  auto ViolationReporter::internal_report_ref_violation(fixpoint::FullSourceLoc full_loc,
                                                        const fixpoint::DeclRefExpr *ref, Ref<String> message) -> void
  {
    const auto location = fixpoint::utils::get_loc_str_path_and_line(full_loc);
    const auto columns = fixpoint::utils::get_ref_str_start_and_end_cols(ref);
    llvm::outs() << location << ":" << columns << ": [Auxid] Violation: " << message << "\n";
  }
} // namespace auxid