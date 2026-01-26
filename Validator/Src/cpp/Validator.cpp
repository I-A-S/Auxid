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

extern au::Mut<llvm::cl::opt<bool>> raw_output;

namespace Auxid::Validator {

auto MutabilityMatchHandler::is_type_safe(MutRef<StringRef> ty) -> bool {
  if (ty.starts_with("Auxid::")) {
    ty.consume_front("Auxid::");
  } else if (ty.starts_with("ox::")) {
    ty.consume_front("ox::");
  }

  ty = ty.trim();

  usize word_end = ty.find_first_not_of(
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");

  if (word_end == StringRef::npos || word_end == 0) {
    return false;
  }

  StringRef name = ty.substr(0, word_end);
  StringRef suffix = ty.substr(word_end);

  bool is_valid_keyword = (name == "Mut" || name == "Const" || name == "Ref" ||
                           name == "MutRef" || name == "ForwardRef");

  if (!is_valid_keyword) {
    return false;
  }

  usize bracket_pos = suffix.find_first_not_of(" \t\n\r");

  if (bracket_pos == StringRef::npos || suffix[bracket_pos] != '<') {
    return false;
  }

  return true;
}

auto MutabilityMatchHandler::run(Ref<MatchFinder::MatchResult> result) -> void {
  const VarDecl *var_decl = result.Nodes.getNodeAs<clang::VarDecl>("var");
  if (!var_decl) {
    return;
  }

  if (var_decl->isConstexpr()) {
    return;
  }

  if (var_decl->getDeclContext()->isDependentContext() ||
      var_decl->getType()->isDependentType()) {
    return;
  }

  SourceLocation loc = var_decl->getLocation();
  if (result.SourceManager->isInSystemHeader(loc) ||
      !result.SourceManager->isInMainFile(loc) || !loc.isValid()) {
    return;
  }

  if (const ParmVarDecl *parm = dyn_cast<ParmVarDecl>(var_decl)) {
    if (const FunctionDecl *func =
            dyn_cast<FunctionDecl>(parm->getDeclContext())) {
      if (func->isDeleted()) {
        return;
      }
    }
  }

  TypeSourceInfo *tsi = var_decl->getTypeSourceInfo();
  if (!tsi) {
    return;
  }

  Mut<TypeLoc> tl = tsi->getTypeLoc();
  while (ArrayTypeLoc arr = tl.getAs<ArrayTypeLoc>()) {
    tl = arr.getElementLoc();
  }

  SourceRange range = tl.getSourceRange();

  Mut<StringRef> type_text = Lexer::getSourceText(
      CharSourceRange::getTokenRange(range), *result.SourceManager,
      result.Context->getLangOpts());

  if (type_text.empty()) {
    return;
  }

  if (!is_type_safe(type_text)) {
    FullSourceLoc full_loc = result.Context->getFullLoc(loc);

    const FileEntry *file_entry = full_loc.getFileEntry();
    String file_path =
        file_entry ? file_entry->tryGetRealPathName().str() : String();
    Mut<String> display_path = file_path;

    Mut<std::error_code> ec;
    std::filesystem::path rel_path = std::filesystem::relative(
        file_path, std::filesystem::current_path(), ec);
    if (!ec) {
      display_path = rel_path.string();
    }

    const char *ansi_yellow = "\033[33m";
    const char *ansi_reset = "\033[0m";

    if (raw_output) {
      llvm::outs() << file_path << ":" << full_loc.getSpellingLineNumber()
                   << ":" << full_loc.getSpellingColumnNumber()
                   << ": [Auxid] Violation: "
                   << "Variable '" << var_decl->getNameAsString()
                   << "' has unsafe type '" << type_text << "'. "
                   << "Must be wrapped in Mut<T>, T, Ref<T>, "
                      "MutRef<T>, or ForwardRef<T>.\n";
    } else {
      llvm::outs() << ansi_yellow << " \u26A0\ufe0f  [WARNING]  "
                   << display_path << ":" << full_loc.getSpellingLineNumber()
                   << ":" << full_loc.getSpellingColumnNumber() << ": "
                   << "Variable '" << var_decl->getNameAsString()
                   << "' has unsafe type '" << type_text << "'. "
                   << "Must be wrapped in Mut<T>, T, Ref<T>, "
                      "MutRef<T>, or ForwardRef<T>."
                   << ansi_reset << "\n";
    }
  }
}
} // namespace Auxid::Validator