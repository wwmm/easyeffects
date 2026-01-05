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

#include "deesser_preset.hpp"
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_deesser.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

DeesserPreset::DeesserPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::Deesser>(pipeline_type);
}

void DeesserPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["detection"] =
      settings->defaultDetectionLabelsValue()[settings->detection()].toStdString();

  json[section][instance_name]["mode"] = settings->defaultModeLabelsValue()[settings->mode()].toStdString();

  json[section][instance_name]["threshold"] = settings->threshold();

  json[section][instance_name]["ratio"] = settings->ratio();

  json[section][instance_name]["laxity"] = settings->laxity();

  json[section][instance_name]["makeup"] = settings->makeup();

  json[section][instance_name]["f1-freq"] = settings->f1Freq();

  json[section][instance_name]["f2-freq"] = settings->f2Freq();

  json[section][instance_name]["f1-level"] = settings->f1Level();

  json[section][instance_name]["f2-level"] = settings->f2Level();

  json[section][instance_name]["f2-q"] = settings->f2Q();

  json[section][instance_name]["sc-listen"] = settings->scListen();
}

void DeesserPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("threshold", Threshold);
  UPDATE_PROPERTY("ratio", Ratio);
  UPDATE_PROPERTY("laxity", Laxity);
  UPDATE_PROPERTY("makeup", Makeup);
  UPDATE_PROPERTY("f1-freq", F1Freq);
  UPDATE_PROPERTY("f2-freq", F2Freq);
  UPDATE_PROPERTY("f1-level", F1Level);
  UPDATE_PROPERTY("f2-level", F2Level);
  UPDATE_PROPERTY("f2-q", F2Q);
  UPDATE_PROPERTY("sc-listen", ScListen);

  UPDATE_ENUM_LIKE_PROPERTY("mode", Mode);
  UPDATE_ENUM_LIKE_PROPERTY("detection", Detection);
}
