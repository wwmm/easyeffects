/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * Copyright © 2026 Alessio Attilio
 *
 * This file is part of Easy Effects
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

#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_equalizer_channel.h"
#include "easyeffects_db_midside_equalizer.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"

class MidSideEqualizerPreset : public PluginPresetBase {
 public:
  explicit MidSideEqualizerPreset(PipelineType pipeline_type, const std::string& instance_name);

 private:
  DbMidSideEqualizer* settings = nullptr;
  DbEqualizerChannel *input_settings_mid = nullptr, *input_settings_side = nullptr;
  DbEqualizerChannel *output_settings_mid = nullptr, *output_settings_side = nullptr;

  void save(nlohmann::json& json) override;

  void load(const nlohmann::json& json) override;

  static void save_channel(nlohmann::json& json, DbEqualizerChannel* settings, const int& nbands);

  static void load_channel(const nlohmann::json& json, DbEqualizerChannel* settings, const int& nbands);
};
