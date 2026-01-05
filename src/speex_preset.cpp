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

#include "speex_preset.hpp"
#include <strings.h>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_speex.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

SpeexPreset::SpeexPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::Speex>(pipeline_type);
}

void SpeexPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["enable-denoise"] = settings->enableDenoise();

  json[section][instance_name]["noise-suppression"] = settings->noiseSuppression();

  json[section][instance_name]["enable-agc"] = settings->enableAgc();

  json[section][instance_name]["vad"]["enable"] = settings->enableVad();

  json[section][instance_name]["vad"]["probability-start"] = settings->vadProbabilityStart();

  json[section][instance_name]["vad"]["probability-continue"] = settings->vadProbabilityContinue();

  json[section][instance_name]["enable-dereverb"] = settings->enableDereverb();
}

void SpeexPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("enable-denoise", EnableDenoise);
  UPDATE_PROPERTY("noise-suppression", NoiseSuppression);
  UPDATE_PROPERTY("enable-agc", EnableAgc);
  UPDATE_PROPERTY("enable-dereverb", EnableDereverb);

  UPDATE_PROPERTY_INSIDE_SUBSECTION("vad", "enable", EnableVad);
  UPDATE_PROPERTY_INSIDE_SUBSECTION("vad", "probability-start", VadProbabilityStart);
  UPDATE_PROPERTY_INSIDE_SUBSECTION("vad", "probability-continue", VadProbabilityContinue);
}
