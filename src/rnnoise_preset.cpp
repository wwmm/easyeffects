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

#include "rnnoise_preset.hpp"
#include <strings.h>
#include <QString>
#include <filesystem>
#include <format>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_rnnoise.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"
#include "util.hpp"

RNNoisePreset::RNNoisePreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::RNNoise>(pipeline_type);
}

void RNNoisePreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["model-name"] = settings->modelName().toStdString();

  json[section][instance_name]["enable-vad"] = settings->enableVad();

  json[section][instance_name]["vad-thres"] = settings->vadThres();

  json[section][instance_name]["wet"] = settings->wet();

  json[section][instance_name]["release"] = settings->release();

  json[section][instance_name]["use-standard-model"] = settings->useStandardModel();
}

void RNNoisePreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("enable-vad", EnableVad);
  UPDATE_PROPERTY("vad-thres", VadThres);
  UPDATE_PROPERTY("wet", Wet);
  UPDATE_PROPERTY("release", Release);
  UPDATE_PROPERTY("use-standard-model", UseStandardModel);

  // model-path deprecation

  std::string new_model_name =
      json.at(section).at(instance_name).value("model-name", settings->modelName().toStdString());

  if (new_model_name.empty()) {
    const std::string model_path = json.at(section).at(instance_name).value("model-path", "");

    if (!model_path.empty()) {
      new_model_name = std::filesystem::path{model_path}.stem().string();

      util::warning(
          std::format("Using RNNoise model-path is deprecated, please update your preset; fallback to model-name: {}",
                      new_model_name));
    }
  }

  if (new_model_name != settings->modelName()) {
    settings->setModelName(QString::fromStdString(new_model_name));
  }
}
