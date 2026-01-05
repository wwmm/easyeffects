/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <qhashfunctions.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "db_manager.hpp"
#include "pipeline_type.hpp"
#include "util.hpp"

class PluginPresetBase {
 public:
  PluginPresetBase(PipelineType pipeline_type, std::string instance_name);
  PluginPresetBase(const PluginPresetBase&) = delete;
  auto operator=(const PluginPresetBase&) -> PluginPresetBase& = delete;
  PluginPresetBase(const PluginPresetBase&&) = delete;
  auto operator=(const PluginPresetBase&&) -> PluginPresetBase& = delete;

  virtual ~PluginPresetBase() = default;

  template <typename dbType>
  auto get_db_instance(const PipelineType& pipeline_type) -> dbType* {
    auto instance = db::Manager::self().get_plugin_db<dbType>(pipeline_type, QString::fromStdString(instance_name));

    if (instance == nullptr) {
      util::warning(std::format("Failed to get the database instance for: {}", instance_name));
    }

    return instance;
  }

  void write(nlohmann::json& json) {
    try {
      save(json);
    } catch (const nlohmann::json::exception& e) {
      util::warning(e.what());
    }
  }

  void read(const nlohmann::json& json) {
    // Old presets do not have the filter instance id.

    if (!json.at(section).contains(instance_name)) {
      instance_name.erase(instance_name.find('#'));
    }

    // For simplicity, exceptions raised while reading presets parameters
    // should be handled outside this method.

    load(json);
  }

 protected:
  std::string section, instance_name;

  PipelineType pipeline_type;

  virtual void save(nlohmann::json& json) = 0;

  virtual void load(const nlohmann::json& json) = 0;

 private:
};
