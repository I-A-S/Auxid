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

#include <use_after_move.hpp>
#include <violation_reporter.hpp>

#include <fixpoint/utils.hpp>

namespace auxid
{
  class TransferVisitor
  {
    MutRef<UAMSloverState> m_state;
    Ref<clang::ASTContext> m_ast_ctx;

    Vec<const fixpoint::VarDecl *> m_vars_to_move;
    Vec<const fixpoint::VarDecl *> m_vars_to_revive;

public:
    explicit TransferVisitor(MutRef<UAMSloverState> s, Ref<clang::ASTContext> ast_ctx) : m_state(s), m_ast_ctx(ast_ctx)
    {
    }

    void commit()
    {
      for (const auto *v : m_vars_to_move)
        m_state.vars[v] = VarStatus::Moved;

      for (const auto *v : m_vars_to_revive)
      {
        m_state.vars[v] = VarStatus::Alive;
        m_state.vars.erase(v);
      }
    }

    void visit(const fixpoint::Stmt *s)
    {
      using namespace fixpoint;

      if (!s)
        return;

      if (const auto *bin = dyn_cast<BinaryOperator>(s))
      {
        if (bin->isAssignmentOp())
        {
          visit(bin->getRHS());

          const Expr *lhs = bin->getLHS()->IgnoreParenImpCasts();
          if (const auto *dref = dyn_cast<DeclRefExpr>(lhs))
          {
            if (const auto *var = dyn_cast<VarDecl>(dref->getDecl()))
            {
              m_vars_to_revive.push_back(var);
            }
          }

          return;
        }
      }

      if (const auto *call = dyn_cast<CallExpr>(s))
      {
        if (fixpoint::utils::is_std_call(call, "move"))
        {
          if (call->getNumArgs() > 0)
          {
            const Expr *arg = call->getArg(0)->IgnoreParenImpCasts();
            if (const auto *dref = dyn_cast<DeclRefExpr>(arg))
            {
              if (const auto *var = dyn_cast<VarDecl>(dref->getDecl()))
              {
                if (!fixpoint::utils::is_cheap_to_copy(var))
                  m_vars_to_move.push_back(var);
              }
            }
          }
          return;
        }
      }

      if (const auto *dref = dyn_cast<DeclRefExpr>(s))
      {
        if (const auto *var = dyn_cast<VarDecl>(dref->getDecl()))
        {
          if (var->hasLocalStorage())
          {
            auto it = m_state.vars.find(var);
            if (it != m_state.vars.end() && it->second == VarStatus::Moved)
              ViolationReporter::report_ref_violation(m_ast_ctx.getFullLoc(dref->getLocation()), dref,
                                                      "Variable '{}' is being used after move.",
                                                      var->getNameAsString());
          }
        }
      }
    }
  };

  [[nodiscard]] auto UseAfterMoveSolver::get_initial_state() -> UAMSloverState
  {
    return {};
  }

  auto UseAfterMoveSolver::merge(Ref<UAMSloverState> current, Ref<UAMSloverState> incoming) -> UAMSloverState
  {
    UAMSloverState result = current;

    for (const auto &[decl, status] : incoming.vars)
    {
      if (status == VarStatus::Moved)
      {
        result.vars[decl] = VarStatus::Moved;
      }
    }
    return result;
  }

  auto UseAfterMoveSolver::transfer(const fixpoint::Stmt *stmt, MutRef<UAMSloverState> state) -> void
  {
    if (!stmt)
      return;
    TransferVisitor visitor(state, *get_match_result()->Context);
    visitor.visit(stmt);
    visitor.commit();
  }

  auto UseAfterMoveSolver::transfer_initializer(const fixpoint::CXXCtorInitializer *init, MutRef<UAMSloverState> state)
      -> void
  {
    if (const fixpoint::Expr *expr = init->getInit())
    {
      transfer(expr, state);
    }
  }

  auto UseAfterMoveSolver::transfer_implicit_dtor(const fixpoint::CFGImplicitDtor *dtor, MutRef<UAMSloverState> state)
      -> void
  {
    AU_UNUSED(dtor);
    AU_UNUSED(state);
  }

  [[nodiscard]] auto UseAfterMoveSolver::get_matcher() const -> fixpoint::DeclarationMatcher
  {
    using namespace fixpoint::ast;
    return functionDecl(isDefinition());
  }
} // namespace auxid