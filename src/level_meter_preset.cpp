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

#include "level_meter_preset.hpp"
#include <strings.h>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_level_meter.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

LevelMeterPreset::LevelMeterPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::LevelMeter>(pipeline_type);
}

void LevelMeterPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();
}

void LevelMeterPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
}
