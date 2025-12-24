/**
 * Copyright © 2017-2025 Wellington Wallace
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

#include "limiter_preset.hpp"
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_limiter.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

LimiterPreset::LimiterPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::Limiter>(pipeline_type);
}

void LimiterPreset::save(nlohmann::json& json) {
  json[section][instance_name]["mode"] = settings->defaultModeLabelsValue()[settings->mode()].toStdString();

  json[section][instance_name]["oversampling"] =
      settings->defaultOversamplingLabelsValue()[settings->oversampling()].toStdString();

  json[section][instance_name]["dithering"] =
      settings->defaultDitheringLabelsValue()[settings->dithering()].toStdString();

  json[section][instance_name]["sidechain-type"] =
      settings->defaultSidechainTypeLabelsValue()[settings->sidechainType()].toStdString();

  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["lookahead"] = settings->lookahead();

  json[section][instance_name]["attack"] = settings->attack();

  json[section][instance_name]["release"] = settings->release();

  json[section][instance_name]["threshold"] = settings->threshold();

  json[section][instance_name]["sidechain-preamp"] = settings->sidechainPreamp();

  json[section][instance_name]["stereo-link"] = settings->stereoLink();

  json[section][instance_name]["alr-attack"] = settings->alrAttack();

  json[section][instance_name]["alr-release"] = settings->alrRelease();

  json[section][instance_name]["alr-knee"] = settings->alrKnee();

  json[section][instance_name]["alr-knee-smooth"] = settings->alrKneeSmooth();

  json[section][instance_name]["alr"] = settings->alr();

  json[section][instance_name]["gain-boost"] = settings->gainBoost();

  json[section][instance_name]["input-to-sidechain"] = settings->inputToSidechain();

  json[section][instance_name]["input-to-link"] = settings->inputToLink();

  json[section][instance_name]["sidechain-to-input"] = settings->sidechainToInput();

  json[section][instance_name]["sidechain-to-link"] = settings->sidechainToLink();

  json[section][instance_name]["link-to-input"] = settings->linkToInput();

  json[section][instance_name]["link-to-sidechain"] = settings->linkToSidechain();
}

void LimiterPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("lookahead", Lookahead);
  UPDATE_PROPERTY("attack", Attack);
  UPDATE_PROPERTY("release", Release);
  UPDATE_PROPERTY("threshold", Threshold);
  UPDATE_PROPERTY("sidechain-preamp", SidechainPreamp);
  UPDATE_PROPERTY("stereo-link", StereoLink);
  UPDATE_PROPERTY("alr-attack", AlrAttack);
  UPDATE_PROPERTY("alr-release", AlrRelease);
  UPDATE_PROPERTY("alr-knee", AlrKnee);
  UPDATE_PROPERTY("alr-knee-smooth", AlrKneeSmooth);
  UPDATE_PROPERTY("alr", Alr);
  UPDATE_PROPERTY("gain-boost", GainBoost);
  UPDATE_PROPERTY("input-to-sidechain", InputToSidechain);
  UPDATE_PROPERTY("input-to-link", InputToLink);
  UPDATE_PROPERTY("sidechain-to-input", SidechainToInput);
  UPDATE_PROPERTY("sidechain-to-link", SidechainToLink);
  UPDATE_PROPERTY("link-to-input", LinkToInput);
  UPDATE_PROPERTY("link-to-sidechain", LinkToSidechain);

  UPDATE_ENUM_LIKE_PROPERTY("mode", Mode);
  UPDATE_ENUM_LIKE_PROPERTY("oversampling", Oversampling);
  UPDATE_ENUM_LIKE_PROPERTY("dithering", Dithering);
  UPDATE_ENUM_LIKE_PROPERTY("sidechain-type", SidechainType);
}
