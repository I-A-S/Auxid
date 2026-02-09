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

#include <use_after_free.hpp>
#include <violation_reporter.hpp>

#include <fixpoint/utils.hpp>

namespace auxid
{
  class UAFVisitor
  {
    MutRef<UAFState> m_state;
    Ref<clang::ASTContext> m_ast_ctx;

    Vec<const fixpoint::VarDecl *> m_vars_to_revive;
    Vec<const fixpoint::VarDecl *> m_vars_to_kill;

    void check_use(const fixpoint::Expr *expr, const char *context_msg)
    {
      if (!expr)
        return;

      expr = expr->IgnoreParenImpCasts();

      if (const auto dref = fixpoint::llvm_cast<const fixpoint::DeclRefExpr>(expr))
      {
        if (const auto var = fixpoint::llvm_cast<const fixpoint::VarDecl>(dref->getDecl()))
        {
          auto it = m_state.pointers.find(var);
          if (it != m_state.pointers.end() && it->second == PtrStatus::Dangling)
            ViolationReporter::report_ref_violation(m_ast_ctx.getFullLoc(dref->getLocation()), dref, "{} '{}'",
                                                    context_msg, var->getNameAsString());
        }
      }
    }

public:
    explicit UAFVisitor(MutRef<UAFState> s, Ref<clang::ASTContext> ast_ctx) : m_state(s), m_ast_ctx(ast_ctx)
    {
    }

    void commit()
    {
      for (const auto *v : m_vars_to_kill)
        m_state.pointers[v] = PtrStatus::Dangling;

      for (const auto *v : m_vars_to_revive)
        m_state.pointers.erase(v);
    }

    void visit(const fixpoint::Stmt *s)
    {
      using namespace fixpoint;
      if (!s)
        return;

      if (const auto *del = dyn_cast<CXXDeleteExpr>(s))
      {
        const Expr *arg = del->getArgument()->IgnoreParenImpCasts();
        if (const auto *dref = dyn_cast<DeclRefExpr>(arg))
        {
          if (const auto *var = dyn_cast<VarDecl>(dref->getDecl()))
          {
            auto it = m_state.pointers.find(var);
            if (it != m_state.pointers.end() && it->second == PtrStatus::Dangling)
              ViolationReporter::report_ref_violation(m_ast_ctx.getFullLoc(dref->getLocation()), dref,
                                                      "Detected double free of '{}", var->getNameAsString());
            m_vars_to_kill.push_back(var);
          }
        }
        return;
      }

      if (const auto *call = dyn_cast<CallExpr>(s))
      {
        if (fixpoint::utils::is_std_call(call, "free"))
        {
          if (call->getNumArgs() > 0)
          {
            const Expr *arg = call->getArg(0)->IgnoreParenImpCasts();
            if (const auto *dref = dyn_cast<DeclRefExpr>(arg))
            {
              if (const auto *var = dyn_cast<VarDecl>(dref->getDecl()))
              {
                auto it = m_state.pointers.find(var);
                if (it != m_state.pointers.end() && it->second == PtrStatus::Dangling)
                  ViolationReporter::report_ref_violation(m_ast_ctx.getFullLoc(dref->getLocation()), dref,
                                                          "Detected double free of '{}'", var->getNameAsString());
                m_vars_to_kill.push_back(var);
                return;
              }
            }
          }
        }

        for (const auto *arg : call->arguments())
        {
          check_use(arg, "Passed dangling pointer to function");
        }
      }

      if (const auto *bin = dyn_cast<BinaryOperator>(s))
      {
        if (bin->isAssignmentOp())
        {
          visit(bin->getRHS());

          check_use(bin->getRHS(), "Read of dangling pointer value during assignment");

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

      if (const auto *ret = dyn_cast<ReturnStmt>(s))
      {
        check_use(ret->getRetValue(), "Returned dangling pointer");
      }

      if (const auto *unary = dyn_cast<UnaryOperator>(s))
      {
        if (unary->getOpcode() == clang::UO_Deref)
        {
          check_use(unary->getSubExpr(), "Dereference of dangling pointer");
        }
      }
      else if (const auto *mem = dyn_cast<MemberExpr>(s))
      {
        if (mem->isArrow())
        {
          check_use(mem->getBase(), "Member access via dangling pointer");
        }
      }
      else if (const auto *arr = dyn_cast<ArraySubscriptExpr>(s))
      {
        check_use(arr->getBase(), "Array indexing of dangling pointer");
      }
    }
  };

  [[nodiscard]] auto UseAfterFreeSolver::get_initial_state() -> UAFState
  {
    return {};
  }

  auto UseAfterFreeSolver::merge(Ref<UAFState> current, Ref<UAFState> incoming) -> UAFState
  {
    UAFState result = current;
    for (const auto &[decl, status] : incoming.pointers)
    {
      if (status == PtrStatus::Dangling)
      {
        result.pointers[decl] = PtrStatus::Dangling;
      }
    }
    return result;
  }

  auto UseAfterFreeSolver::transfer(const fixpoint::Stmt *stmt, MutRef<UAFState> state) -> void
  {
    if (!stmt)
      return;
    UAFVisitor visitor(state, *get_match_result()->Context);
    visitor.visit(stmt);
    visitor.commit();
  }

  auto UseAfterFreeSolver::transfer_initializer(const fixpoint::CXXCtorInitializer *init, MutRef<UAFState> state)
      -> void
  {
    AU_UNUSED(init);
    AU_UNUSED(state);
  }

  auto UseAfterFreeSolver::transfer_implicit_dtor(const fixpoint::CFGImplicitDtor *dtor, MutRef<UAFState> state) -> void
  {
    AU_UNUSED(dtor);
    AU_UNUSED(state);
  }

  [[nodiscard]] auto UseAfterFreeSolver::get_matcher() const -> fixpoint::DeclarationMatcher
  {
    using namespace fixpoint::ast;
    return functionDecl(isDefinition());
  }
} // namespace auxid