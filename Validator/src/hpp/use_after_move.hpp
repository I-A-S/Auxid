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

  enum class VarStatus
  {
    Alive,
    Moved
  };

  struct UAMSloverState
  {
    HashMap<const fixpoint::VarDecl *, VarStatus> vars;

    auto operator==(const UAMSloverState &other) const -> bool
    {
      return vars == other.vars;
    }
  };

  class UseAfterMoveSolver : public fixpoint::DataFlowSolver<UAMSloverState>
  {
public:
    [[nodiscard]] auto get_initial_state() -> UAMSloverState override;

    auto merge(Ref<UAMSloverState> current, Ref<UAMSloverState> incoming) -> UAMSloverState override;

    auto transfer(const fixpoint::Stmt *stmt, MutRef<UAMSloverState> state) -> void override;
    auto transfer_initializer(const fixpoint::CXXCtorInitializer *init, MutRef<UAMSloverState> state) -> void override;
    auto transfer_implicit_dtor(const fixpoint::CFGImplicitDtor *dtor, MutRef<UAMSloverState> state) -> void override;

    [[nodiscard]] auto get_matcher() const -> fixpoint::DeclarationMatcher override;
  };
} // namespace auxid