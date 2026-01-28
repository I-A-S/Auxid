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

static llvm::cl::OptionCategory auxid_category("auxid-validator options");
static cl::extrahelp common_help(CommonOptionsParser::HelpMessage);

namespace auxid
{
  class SingleCommandDatabase : public CompilationDatabase
  {
    const CompilationDatabase &m_base;

public:
    SingleCommandDatabase(Ref<CompilationDatabase> base) : m_base(base)
    {
    }

    [[nodiscard]] Vec<CompileCommand> getCompileCommands(StringRef file_path) const override
    {
      auto cmds = m_base.getCompileCommands(file_path);
      if (cmds.size() > 1)
        cmds.resize(1);
      return cmds;
    }

    [[nodiscard]] Vec<String> getAllFiles() const override
    {
      return m_base.getAllFiles();
    }

    [[nodiscard]] Vec<CompileCommand> getAllCompileCommands() const override
    {
      return m_base.getAllCompileCommands();
    }
  };

  ValidationMatcher::ValidationMatcher(ForwardRef<DeclarationMatcher> pattern,
                                       ForwardRef<Box<MatchFinder::MatchCallback>> callback)
      : pattern(std::move(pattern)), callback(std::move(callback))
  {
    Validator::instance().add_matcher(this);
  }

  auto Validator::add_matcher(ValidationMatcher *matcher) -> void
  {
    m_matchers.push_back(matcher);
  }

  auto Validator::run(i32 argc, const char *argv[]) -> Result<i32>
  {
    llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);

    auto expected_parser = CommonOptionsParser::create(argc, argv, auxid_category);
    if (!expected_parser)
    {
      llvm::errs() << expected_parser.takeError();
      return 1;
    }
    CommonOptionsParser &options_parser = expected_parser.get();

    SingleCommandDatabase single_cmd_db(options_parser.getCompilations());

    ClangTool tool(single_cmd_db, options_parser.getSourcePathList());

    const auto resource_dir = get_clang_resource_dir();

    tool.appendArgumentsAdjuster([&](const CommandLineArguments &args, StringRef) {
      CommandLineArguments new_args;

      if (!args.empty())
        new_args.push_back(args[0]);

      if (!resource_dir.empty())
      {
        new_args.push_back("-resource-dir");
        new_args.push_back(resource_dir);
      }

      for (size_t i = 1; i < args.size(); ++i)
      {
        StringRef arg = args[i];

        if (arg == "-Xclang" && i + 1 < args.size())
        {
          StringRef next_arg = args[i + 1];
          bool should_strip_next = false;

          if (next_arg == "-include-pch" || next_arg == "-pch-is-pch")
          {
            should_strip_next = true;
          }
          else if (next_arg == "-include")
          {
            size_t file_idx = i + 2;
            if (file_idx < args.size() && args[file_idx] == "-Xclang")
              file_idx++;

            if (file_idx < args.size() && (args[file_idx].find("cmake_pch") != String::npos))
            {
              should_strip_next = true;
            }
          }

          if (should_strip_next)
          {
            continue;
          }
        }

        if (arg == "-include-pch" || arg == "-pch-is-pch")
        {
          if (arg == "-include-pch" && i + 1 < args.size())
            i++;
          continue;
        }

        if (arg == "-include")
        {
          size_t next_idx = i + 1;
          if (next_idx < args.size() && args[next_idx] == "-Xclang")
            next_idx++;

          if (next_idx < args.size() && (args[next_idx].find("cmake_pch") != String::npos))
          {
            i = next_idx;
            continue;
          }
        }

        if (arg.contains("cmake_pch") && (arg.ends_with(".hxx") || arg.ends_with(".pch")))
        {
          continue;
        }

        new_args.push_back(std::string(arg));
      }
      return new_args;
    });

    MatchFinder finder;

    for (const auto &m : m_matchers)
    {
      finder.addMatcher(m->pattern, m->callback.get());
    }

    return tool.run(newFrontendActionFactory(&finder).get());
  }

  auto Validator::get_clang_resource_dir() -> String
  {
    Mut<String> result;

#if defined(_WIN32)
    FILE *const pipe = _popen("clang -print-resource-dir", "r");
#else
    FILE *const pipe = popen("clang -print-resource-dir", "r");
#endif
    if (!pipe)
      return "";

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
      result += buffer;

#if defined(_WIN32)
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
      result.pop_back();

    return result;
  }
} // namespace auxid