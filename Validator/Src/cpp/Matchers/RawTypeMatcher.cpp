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

#include <Validator.hpp>

namespace Auxid
{
  static auto is_type_safe(MutRef<StringRef> ty) -> bool
  {
    if (ty.starts_with("Auxid::"))
    {
      ty.consume_front("Auxid::");
    }
    else if (ty.starts_with("au::"))
    {
      ty.consume_front("au::");
    }

    ty = ty.trim();

    usize word_end = ty.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");

    if (word_end == StringRef::npos || word_end == 0)
      return false;

    StringRef name = ty.substr(0, word_end);
    StringRef suffix = ty.substr(word_end);

    bool is_valid_keyword = (name == "Mut" || name == "Ref" || name == "MutRef" || name == "ForwardRef");

    if (!is_valid_keyword)
      return false;

    const usize bracket_pos = suffix.find_first_not_of(" \t\n\r");

    if (bracket_pos == StringRef::npos || suffix[bracket_pos] != '<')
      return false;

    return true;
  }

  REGISTER_MATCHER(
      RawTypeMatcher,
      traverse(clang::TK_IgnoreUnlessSpelledInSource,
               varDecl(unless(isImplicit()), hasType(qualType().bind("type_node")),
                       optionally(hasType(templateSpecializationType().bind("is_template"))),
                       optionally(hasType(autoType().bind("is_auto"))),
                       optionally(hasInitializer(callExpr(callee(functionDecl(hasName("mut")))).bind("mut_pattern"))))
                   .bind("var")))
  {
    const auto *var = result.Nodes.getNodeAs<VarDecl>("var");
    if (!var)
      return;

    SourceLocation loc = var->getLocation();

    if (result.SourceManager->isInSystemHeader(loc) || !result.SourceManager->isInMainFile(loc) || !loc.isValid() ||
        var->getDeclContext()->isDependentContext() || var->getType()->isDependentType() || var->isConstexpr() ||
        var->getType().isConstQualified() || (result.Nodes.getNodeAs<clang::Type>("is_template") != nullptr))
      return;

    if (const ParmVarDecl *parm = dyn_cast<ParmVarDecl>(var))
    {
      if (const FunctionDecl *func = dyn_cast<FunctionDecl>(parm->getDeclContext()))
      {
        if (func->isDeleted())
          return;
      }
    }

    if ((result.Nodes.getNodeAs<clang::Type>("is_auto") != nullptr) &&
        (result.Nodes.getNodeAs<CallExpr>("mut_pattern") != nullptr))
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

    Mut<StringRef> type_text = Lexer::getSourceText(CharSourceRange::getTokenRange(range), *result.SourceManager,
                                                    result.Context->getLangOpts());

    if (type_text.empty())
    {
      return;
    }

    if (!is_type_safe(type_text))
    {
      FullSourceLoc full_loc = result.Context->getFullLoc(loc);

      const FileEntry *file_entry = full_loc.getFileEntry();
      String file_path = file_entry ? file_entry->tryGetRealPathName().str() : String();

      Mut<std::error_code> ec;
      std::filesystem::path rel_path = std::filesystem::relative(file_path, std::filesystem::current_path(), ec);
      const String display_path = ec ? file_path : rel_path.string();

      llvm::outs() << file_path << ":" << full_loc.getSpellingLineNumber() << ":" << full_loc.getSpellingColumnNumber()
                   << ": [Auxid] Violation: "
                   << "Variable '" << var->getNameAsString() << "' has unsafe type '" << type_text << "'. "
                   << "Must either be marked `const` or be wrapped in `Mut<T>`, `Ref<T>`, "
                      "`MutRef<T>`, or `ForwardRef<T>`.\n";
    }
  }
} // namespace Auxid