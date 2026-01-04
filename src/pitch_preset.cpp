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

#include "pitch_preset.hpp"
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_pitch.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

PitchPreset::PitchPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::Pitch>(pipeline_type);
}

void PitchPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["dry"] = settings->dry();

  json[section][instance_name]["wet"] = settings->wet();

  json[section][instance_name]["quick-seek"] = settings->quickSeek();

  json[section][instance_name]["anti-alias"] = settings->antiAlias();

  json[section][instance_name]["sequence-length"] = settings->sequenceLength();

  json[section][instance_name]["seek-window"] = settings->seekWindow();

  json[section][instance_name]["overlap-length"] = settings->overlapLength();

  json[section][instance_name]["tempo-difference"] = settings->tempoDifference();

  json[section][instance_name]["rate-difference"] = settings->rateDifference();

  json[section][instance_name]["octaves"] = settings->octaves();

  json[section][instance_name]["semitones"] = settings->semitones();

  json[section][instance_name]["cents"] = settings->cents();
}

void PitchPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("dry", Dry);
  UPDATE_PROPERTY("wet", Wet);
  UPDATE_PROPERTY("quick-seek", QuickSeek);
  UPDATE_PROPERTY("anti-alias", AntiAlias);
  UPDATE_PROPERTY("sequence-length", SequenceLength);
  UPDATE_PROPERTY("overlap-length", OverlapLength);
  UPDATE_PROPERTY("tempo-difference", TempoDifference);
  UPDATE_PROPERTY("rate-difference", RateDifference);
  UPDATE_PROPERTY("octaves", Octaves);
  UPDATE_PROPERTY("semitones", Semitones);
  UPDATE_PROPERTY("cents", Cents);
}
