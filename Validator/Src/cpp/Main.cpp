// Oxide: Rust like safety and syntax for C++
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

using namespace Oxide;
using namespace Oxide::Validator;

static Mut<cl::OptionCategory> oxide_category("oxide-validator options");
static Const<cl::extrahelp> COMMON_HELP(CommonOptionsParser::HelpMessage);
static Const<cl::extrahelp>
    MORE_HELP("\nEnforces Oxide explicit mutability rules.\n");

Mut<cl::opt<bool>> raw_output("raw", cl::desc("Disable pretty printing"),
                              cl::cat(oxide_category));

auto get_clang_resource_dir() -> Result<String> {
  Mut<Array<char, 128>> buffer;
  Mut<String> result;

#ifdef _WIN32
  Const<FILE *> pipe = _popen("clang -print-resource-dir 2>NUL", "r");
#else
  Const<FILE *> pipe = popen("clang -print-resource-dir 2>/dev/null", "r");
#endif

  if (!pipe) {
    return fail("Error: 'clang' executable not found in PATH.");
  }

  while (fgets(buffer.data(), static_cast<i32>(buffer.size()), pipe) !=
         nullptr) {
    result += buffer.data();
  }

#ifdef _WIN32
  _pclose(pipe);
#else
  pclose(pipe);
#endif

  while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
    result.pop_back();
  }

  return result + "/include";
}

auto main(Const<int> argc, Const<const char **> argv) -> int {
  if (argc < 2) {
    llvm::outs() << "Usage: " << argv[0] << " <file.cpp> [options]\n";
    return 1;
  }

  Mut<bool> has_resource_arg = false;
  for (Mut<i32> i = 0; i < argc; ++i) {
    Const<StringRef> arg(argv[i]);

    if (arg.contains("-I") && arg.contains("clang") &&
        arg.contains("include")) {
      has_resource_arg = true;
      break;
    }
  }

  Mut<Vec<const char *>> new_argv;
  new_argv.reserve(static_cast<usize>(argc) + 2);
  for (Mut<i32> i = 0; i < argc; ++i) {
    new_argv.push_back(argv[i]);
  }

  Mut<String> resource_path_storage;
  Mut<String> extra_arg_storage;

  if (!has_resource_arg) {
    Mut<Result<String>> res = get_clang_resource_dir();
    if (!res) {
      llvm::errs() << res.error() << "\n";
      return 1;
    }
    resource_path_storage = *res;
    extra_arg_storage = "--extra-arg=-I" + resource_path_storage;
    new_argv.push_back(extra_arg_storage.c_str());
  }

  Mut<int> new_argc = static_cast<int>(new_argv.size());

  Mut<llvm::Expected<CommonOptionsParser>> expected_parser =
      CommonOptionsParser::create(new_argc, new_argv.data(), oxide_category);
  if (!expected_parser) {
    llvm::errs() << expected_parser.takeError();
    return 1;
  }

  Mut<ClangTool> tool(expected_parser.get().getCompilations(),
                      expected_parser.get().getSourcePathList());

  tool.appendArgumentsAdjuster(
      [](Ref<clang::tooling::CommandLineArguments> args,
         Mut<llvm::StringRef> filename) {
        Mut<clang::tooling::CommandLineArguments> new_args;

        if (!args.empty()) {
          new_args.push_back(args[0]);
        }

        new_args.push_back("-Wno-ignored-gch");
        new_args.push_back("-Wno-unused-command-line-argument");

        for (Mut<size_t> i = 1; i < args.size(); ++i) {
          if (args[i] == "-include-pch" || args[i] == "-pch-is-pch") {
            i++;
            continue;
          }

          if (args[i].starts_with("@")) {
            continue;
          }

          new_args.push_back(args[i]);
        }
        return new_args;
      });

  Mut<MutabilityMatchHandler> handler;
  Mut<MatchFinder> finder;

  finder.addMatcher(
      varDecl(unless(isImplicit()), unless(isExpansionInSystemHeader()))
          .bind("var"),
      &handler);

  return tool.run(newFrontendActionFactory(&finder).get());
}