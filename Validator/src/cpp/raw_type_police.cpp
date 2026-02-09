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

#include <raw_type_police.hpp>
#include <violation_reporter.hpp>

#include <fixpoint/utils.hpp>

namespace auxid
{
  using namespace fixpoint;

  static auto is_type_safe(MutRef<LLVM_StringRef> ty) -> bool
  {
    if (ty.starts_with("auxid::"))
    {
      ty.consume_front("auxid::");
    }
    else if (ty.starts_with("au::"))
    {
      ty.consume_front("au::");
    }

    ty = ty.trim();

    usize word_end = ty.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");

    if (word_end == LLVM_StringRef::npos || word_end == 0)
      return false;

    LLVM_StringRef name = ty.substr(0, word_end);
    LLVM_StringRef suffix = ty.substr(word_end);

    bool is_valid_keyword = (name == "Mut" || name == "Ref" || name == "MutRef" || name == "ForwardRef");

    if (!is_valid_keyword)
      return false;

    const usize bracket_pos = suffix.find_first_not_of(" \t\n\r");

    if (bracket_pos == LLVM_StringRef::npos || suffix[bracket_pos] != '<')
      return false;

    return true;
  }

  auto RawTypePolice::police(const fixpoint::Decl *decl, Ref<fixpoint::SourceLocation> loc) -> void
  {
    const auto &match_result = *get_match_result();

    const auto var = llvm_cast<const fixpoint::VarDecl>(decl);
    if (!var || var->getDeclContext()->isDependentContext() || var->getType()->isDependentType() ||
        var->isConstexpr() || var->getType().isConstQualified() || var->getType()->isReferenceType() ||
        (match_result.Nodes.getNodeAs<clang::Type>("is_template") != nullptr))
      return;

    if (const ParmVarDecl *parm = dyn_cast<ParmVarDecl>(var))
    {
      if (fixpoint::utils::is_cheap_to_copy(parm))
        return;
      if (const FunctionDecl *func = dyn_cast<FunctionDecl>(parm->getDeclContext()))
      {
        if (func->isDeleted())
          return;
      }
    }

    if ((match_result.Nodes.getNodeAs<clang::Type>("is_auto") != nullptr) &&
        (match_result.Nodes.getNodeAs<CallExpr>("mut_pattern") != nullptr))
      return;

    TypeSourceInfo *tsi = var->getTypeSourceInfo();
    if (!tsi)
      return;

    Mut<TypeLoc> tl = tsi->getTypeLoc();
    while (ArrayTypeLoc arr = tl.getAs<ArrayTypeLoc>())
    {
      tl = arr.getElementLoc();
    }

    SourceRange range = tl.getSourceRange();

    Mut<LLVM_StringRef> type_text = Lexer::getSourceText(
        CharSourceRange::getTokenRange(range), *match_result.SourceManager, match_result.Context->getLangOpts());

    if (type_text.empty())
    {
      return;
    }

    if (!is_type_safe(type_text))
    {
      clang::TypeSourceInfo *tsi = var->getTypeSourceInfo();
      if (!tsi)
        return;

      const auto type_name = type_text.str();
      if ((type_name == "auto") && !(tsi->getType().isConstQualified() || tsi->getType().isLocalConstQualified()))
        ViolationReporter::report_decl_violation(match_result.Context->getFullLoc(loc), var,
                                                 "Variable '{}' has unsafe type 'auto'. Must either be passed through "
                                                 "`mut()` or be marked "
                                                 "`const auto`.",
                                                 var->getNameAsString());
      else
        ViolationReporter::report_decl_violation(
            match_result.Context->getFullLoc(loc), var,
            "Variable '{}' has unsafe type '{}'. Must either be marked "
            "`const` or be wrapped in `Mut<T>`, `Ref<T>`, `MutRef<T>`, or `ForwardRef<T>`.",
            var->getNameAsString(), type_name);
    }
  }

  [[nodiscard]] auto RawTypePolice::get_matcher() const -> fixpoint::DeclarationMatcher
  {
    using namespace fixpoint::ast;
    return varDecl(unless(isImplicit()), hasType(qualType().bind("type_node")),
                   optionally(hasType(templateSpecializationType().bind("is_template"))),
                   optionally(hasType(autoType().bind("is_auto"))),
                   optionally(hasInitializer(callExpr(callee(functionDecl(hasName("mut")))).bind("mut_pattern"))));
  }
} // namespace auxid