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
#include <use_after_free.hpp>
#include <use_after_move.hpp>

namespace auxid
{
  static auto validator_main(i32 argc, const char **argv) -> Result<i32>
  {
    auto options = AU_TRY(fixpoint::Options::create("Auxid Validator", argc, argv));
    const auto compile_db = AU_TRY(fixpoint::CompileDB::create(options));

    const auto tool = AU_TRY(fixpoint::Tool::create(options, compile_db));

    fixpoint::Workload workload;
    workload.add_task<RawTypePolice>();
    workload.add_task<UseAfterMoveSolver>();
    workload.add_task<UseAfterFreeSolver>();
    AU_TRY_PURE(tool->run(workload));

    return 0;
  }
} // namespace auxid

int main(int argc, char *argv[])
{
  const auto result = auxid::validator_main(argc, (const char **) argv);

  if (!result)
  {
    std::cerr << "ERROR: " << result.error() << "\n";
    return -1;
  }

  return *result;
}