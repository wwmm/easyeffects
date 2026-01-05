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

#include "stereo_tools_preset.hpp"
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_stereo_tools.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

StereoToolsPreset::StereoToolsPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::StereoTools>(pipeline_type);
}

void StereoToolsPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["balance-in"] = settings->balanceIn();

  json[section][instance_name]["balance-out"] = settings->balanceOut();

  json[section][instance_name]["softclip"] = settings->softclip();

  json[section][instance_name]["mutel"] = settings->mutel();

  json[section][instance_name]["muter"] = settings->muter();

  json[section][instance_name]["phasel"] = settings->phasel();

  json[section][instance_name]["phaser"] = settings->phaser();

  json[section][instance_name]["mode"] = settings->defaultModeLabelsValue()[settings->mode()].toStdString();

  json[section][instance_name]["side-level"] = settings->slev();

  json[section][instance_name]["side-balance"] = settings->sbal();

  json[section][instance_name]["middle-level"] = settings->mlev();

  json[section][instance_name]["middle-panorama"] = settings->mpan();

  json[section][instance_name]["stereo-base"] = settings->stereoBase();

  json[section][instance_name]["delay"] = settings->delay();

  json[section][instance_name]["sc-level"] = settings->scLevel();

  json[section][instance_name]["stereo-phase"] = settings->stereoPhase();

  json[section][instance_name]["dry"] = settings->dry();

  json[section][instance_name]["wet"] = settings->wet();
}

void StereoToolsPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("balance-in", BalanceIn);
  UPDATE_PROPERTY("balance-out", BalanceOut);
  UPDATE_PROPERTY("softclip", Softclip);
  UPDATE_PROPERTY("mutel", Mutel);
  UPDATE_PROPERTY("muter", Muter);
  UPDATE_PROPERTY("phasel", Phasel);
  UPDATE_PROPERTY("phaser", Phaser);
  UPDATE_PROPERTY("side-level", Slev);
  UPDATE_PROPERTY("side-balance", Sbal);
  UPDATE_PROPERTY("middle-level", Mlev);
  UPDATE_PROPERTY("middle-panorama", Mpan);
  UPDATE_PROPERTY("stereo-base", StereoBase);
  UPDATE_PROPERTY("delay", Delay);
  UPDATE_PROPERTY("sc-level", ScLevel);
  UPDATE_PROPERTY("phaser", Phaser);
  UPDATE_PROPERTY("stereo-phase", StereoPhase);
  UPDATE_PROPERTY("dry", Dry);
  UPDATE_PROPERTY("wet", Wet);

  UPDATE_ENUM_LIKE_PROPERTY("mode", Mode);
}
