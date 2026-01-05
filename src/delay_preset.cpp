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

#include "delay_preset.hpp"
#include <strings.h>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_delay.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

DelayPreset::DelayPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::Delay>(pipeline_type);
}

void DelayPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["mode-l"] = settings->defaultModeLLabelsValue()[settings->modeL()].toStdString();

  json[section][instance_name]["mode-r"] = settings->defaultModeRLabelsValue()[settings->modeR()].toStdString();

  json[section][instance_name]["time-l"] = settings->timeL();

  json[section][instance_name]["time-r"] = settings->timeR();

  json[section][instance_name]["sample-l"] = settings->sampleL();

  json[section][instance_name]["sample-r"] = settings->sampleR();

  json[section][instance_name]["meters-l"] = settings->metersL();

  json[section][instance_name]["meters-r"] = settings->metersR();

  json[section][instance_name]["centimeters-l"] = settings->centimetersL();

  json[section][instance_name]["centimeters-r"] = settings->centimetersR();

  json[section][instance_name]["temperature-l"] = settings->temperatureL();

  json[section][instance_name]["temperature-r"] = settings->temperatureR();

  json[section][instance_name]["dry-l"] = settings->dryL();

  json[section][instance_name]["dry-r"] = settings->dryR();

  json[section][instance_name]["wet-l"] = settings->wetL();

  json[section][instance_name]["wet-r"] = settings->wetR();

  json[section][instance_name]["invert-phase-l"] = settings->invertPhaseL();

  json[section][instance_name]["invert-phase-r"] = settings->invertPhaseR();
}

void DelayPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("time-l", TimeL);
  UPDATE_PROPERTY("time-r", TimeR);
  UPDATE_PROPERTY("sample-l", SampleL);
  UPDATE_PROPERTY("sample-r", SampleR);
  UPDATE_PROPERTY("meters-l", MetersL);
  UPDATE_PROPERTY("meters-r", MetersR);
  UPDATE_PROPERTY("centimeters-l", CentimetersL);
  UPDATE_PROPERTY("centimeters-r", CentimetersR);
  UPDATE_PROPERTY("temperature-l", TemperatureL);
  UPDATE_PROPERTY("temperature-r", TemperatureR);
  UPDATE_PROPERTY("dry-l", DryL);
  UPDATE_PROPERTY("dry-r", DryR);
  UPDATE_PROPERTY("wet-l", WetL);
  UPDATE_PROPERTY("wet-r", WetR);
  UPDATE_PROPERTY("invert-phase-l", InvertPhaseL);
  UPDATE_PROPERTY("invert-phase-r", InvertPhaseR);

  UPDATE_ENUM_LIKE_PROPERTY("mode-l", ModeL);
  UPDATE_ENUM_LIKE_PROPERTY("mode-r", ModeR);
}
