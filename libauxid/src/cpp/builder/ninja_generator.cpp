// Auxid: The Orthodox C++ Platform.
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

#include <builder/ninja_generator.hpp>

namespace au::builder
{
  auto NinjaGenerator::generate_manifest() const -> String
  {
    String out = "ninja_required_version = 1.5\n\n";

    for (const auto &var : m_global_variables)
    {
      out += var.key + " = " + var.value + "\n";
    }
    out += "\n";

    for (const auto &rule : m_rules)
    {
      out += "rule " + rule.name + "\n";
      out += "  command = " + rule.command + "\n";

      if (!rule.description.empty())
      {
        out += "  description = " + rule.description + "\n";
      }

      if (!rule.depfile.empty())
      {
        out += "  depfile = " + rule.depfile + "\n";
      }
      if (!rule.deps.empty())
      {
        out += "  deps = " + rule.deps + "\n";
      }
    }
    out += "\n";

    for (const auto &edge : m_edges)
    {
      out += "build ";
      for (const auto &output : edge.outputs)
        out += output + " ";
      out += ": " + edge.rule_name + " ";
      for (const auto &input : edge.inputs)
        out += input + " ";
      out += "\n";

      for (const auto &var : edge.edge_variables)
      {
        out += "  " + var.key + " = " + var.value + "\n";
      }
    }

    return out;
  }
} // namespace au::builder