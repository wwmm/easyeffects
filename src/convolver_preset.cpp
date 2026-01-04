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

#include "convolver_preset.hpp"
#include <QString>
#include <filesystem>
#include <format>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_convolver.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"
#include "util.hpp"

ConvolverPreset::ConvolverPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::Convolver>(pipeline_type);
}

void ConvolverPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["kernel-name"] = settings->kernelName().toStdString();

  json[section][instance_name]["ir-width"] = settings->irWidth();

  json[section][instance_name]["autogain"] = settings->autogain();

  json[section][instance_name]["dry"] = settings->dry();

  json[section][instance_name]["wet"] = settings->wet();

  json[section][instance_name]["sofa"]["azimuth"] = settings->targetSofaAzimuth();

  json[section][instance_name]["sofa"]["elevation"] = settings->targetSofaElevation();

  json[section][instance_name]["sofa"]["radius"] = settings->targetSofaRadius();
}

void ConvolverPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("ir-width", IrWidth);
  UPDATE_PROPERTY("autogain", Autogain);
  UPDATE_PROPERTY("dry", Dry);
  UPDATE_PROPERTY("wet", Wet);

  UPDATE_PROPERTY_INSIDE_SUBSECTION("sofa", "azimuth", TargetSofaAzimuth);
  UPDATE_PROPERTY_INSIDE_SUBSECTION("sofa", "elevation", TargetSofaElevation);
  UPDATE_PROPERTY_INSIDE_SUBSECTION("sofa", "radius", TargetSofaRadius);

  // kernel-path deprecation
  const auto* kernel_name_key = "kernel-name";

  std::string new_kernel_name =
      json.at(section).at(instance_name).value(kernel_name_key, settings->defaultKernelNameValue().toStdString());

  if (new_kernel_name.empty()) {
    const std::string kernel_path = json.at(section).at(instance_name).value("kernel-path", "");

    if (!kernel_path.empty()) {
      new_kernel_name = std::filesystem::path{kernel_path}.stem().string();

      util::warning(std::format(
          "Using Convolver kernel-path is deprecated, please update your preset; fallback to kernel-name: {}",
          new_kernel_name));
    }
  }

  const auto current_kernel_name = settings->kernelName();

  if (new_kernel_name != current_kernel_name) {
    settings->setKernelName(QString::fromStdString(new_kernel_name));
  }
}
