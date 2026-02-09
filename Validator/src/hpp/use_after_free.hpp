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

  enum class PtrStatus : uint8_t
  {
    Valid,
    Dangling
  };

  struct UAFState
  {
    std::unordered_map<const fixpoint::VarDecl *, PtrStatus> pointers;

    auto operator==(const UAFState &other) const -> bool
    {
      return pointers == other.pointers;
    }
  };

  class UseAfterFreeSolver : public fixpoint::DataFlowSolver<UAFState>
  {
public:
    [[nodiscard]] auto get_initial_state() -> UAFState override;

    auto merge(Ref<UAFState> current, Ref<UAFState> incoming) -> UAFState override;

    auto transfer(const fixpoint::Stmt *stmt, MutRef<UAFState> state) -> void override;
    auto transfer_initializer(const fixpoint::CXXCtorInitializer *init, MutRef<UAFState> state) -> void override;
    auto transfer_implicit_dtor(const fixpoint::CFGImplicitDtor *dtor, MutRef<UAFState> state) -> void override;

    [[nodiscard]] auto get_matcher() const -> fixpoint::DeclarationMatcher override;
  };
} // namespace auxid