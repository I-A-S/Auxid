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

#pragma once

#include <auxid/builder/target.hpp>

namespace au::builder
{
  struct NinjaVariable
  {
    String key;
    String value;
  };

  struct NinjaRule
  {
    String name;
    String command;
    String description;
    String depfile;
    String deps;
  };

  struct NinjaBuildEdge
  {
    String rule_name;
    Vec<String> outputs;
    Vec<String> inputs;
    Vec<String> implicit_inputs;
    Vec<NinjaVariable> edge_variables;
  };

  class NinjaGenerator
  {
private:
    Vec<NinjaVariable> m_global_variables;
    Vec<NinjaRule> m_rules;
    Vec<NinjaBuildEdge> m_edges;

public:
    auto add_global_variable(String key, String value) -> void
    {
      m_global_variables.push_back({key, value});
    }

    auto add_rule(NinjaRule rule) -> void
    {
      m_rules.push_back(rule);
    }

    auto add_edge(NinjaBuildEdge edge) -> void
    {
      m_edges.push_back(edge);
    }

    [[nodiscard]] auto generate_manifest() const -> String;
  };
} // namespace au::builder