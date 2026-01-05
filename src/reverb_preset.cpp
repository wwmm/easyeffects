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

#include "reverb_preset.hpp"
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_reverb.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

ReverbPreset::ReverbPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::Reverb>(pipeline_type);
}

void ReverbPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["room-size"] =
      settings->defaultRoomSizeLabelsValue()[settings->roomSize()].toStdString();

  json[section][instance_name]["decay-time"] = settings->decayTime();

  json[section][instance_name]["hf-damp"] = settings->hfDamp();

  json[section][instance_name]["diffusion"] = settings->diffusion();

  json[section][instance_name]["amount"] = settings->amount();

  json[section][instance_name]["dry"] = settings->dry();

  json[section][instance_name]["predelay"] = settings->predelay();

  json[section][instance_name]["bass-cut"] = settings->bassCut();

  json[section][instance_name]["treble-cut"] = settings->trebleCut();
}

void ReverbPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("decay-time", DecayTime);
  UPDATE_PROPERTY("hf-damp", HfDamp);
  UPDATE_PROPERTY("diffusion", Diffusion);
  UPDATE_PROPERTY("amount", Amount);
  UPDATE_PROPERTY("dry", Dry);
  UPDATE_PROPERTY("predelay", Predelay);
  UPDATE_PROPERTY("bass-cut", BassCut);
  UPDATE_PROPERTY("treble-cut", TrebleCut);

  UPDATE_ENUM_LIKE_PROPERTY("room-size", RoomSize);
}
