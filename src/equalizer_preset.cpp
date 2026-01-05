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

#include "equalizer_preset.hpp"
#include <strings.h>
#include <QString>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_equalizer.h"
#include "easyeffects_db_equalizer_channel.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"
#include "tags_equalizer.hpp"
#include "util.hpp"

using namespace tags::equalizer;

EqualizerPreset::EqualizerPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::Equalizer>(pipeline_type);

  if (settings != nullptr) {
    input_settings_left = db::Manager::self().get_plugin_db<db::EqualizerChannel>(
        pipeline_type, QString::fromStdString(instance_name + "#left"));
    input_settings_right = db::Manager::self().get_plugin_db<db::EqualizerChannel>(
        pipeline_type, QString::fromStdString(instance_name + "#right"));

    output_settings_left = db::Manager::self().get_plugin_db<db::EqualizerChannel>(
        pipeline_type, QString::fromStdString(instance_name + "#left"));
    output_settings_right = db::Manager::self().get_plugin_db<db::EqualizerChannel>(
        pipeline_type, QString::fromStdString(instance_name + "#right"));
  }
}

void EqualizerPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["mode"] = settings->defaultModeLabelsValue()[settings->mode()].toStdString();

  json[section][instance_name]["split-channels"] = settings->splitChannels();

  json[section][instance_name]["balance"] = settings->balance();

  json[section][instance_name]["pitch-left"] = settings->pitchLeft();

  json[section][instance_name]["pitch-right"] = settings->pitchRight();

  const auto nbands = settings->numBands();

  json[section][instance_name]["num-bands"] = nbands;

  if (section == "input") {
    save_channel(json[section][instance_name]["left"], input_settings_left, nbands);
    save_channel(json[section][instance_name]["right"], input_settings_right, nbands);
  } else if (section == "output") {
    save_channel(json[section][instance_name]["left"], output_settings_left, nbands);
    save_channel(json[section][instance_name]["right"], output_settings_right, nbands);
  }
}

void EqualizerPreset::save_channel(nlohmann::json& json, db::EqualizerChannel* settings, const int& nbands) {
  for (int n = 0; n < nbands; n++) {
    const auto* const bandn = band_id[n];

    json[bandn]["type"] =
        settings->bandTypeLabels()[settings->property(band_type[n].data()).value<int>()].toStdString();

    json[bandn]["mode"] =
        settings->bandModeLabels()[settings->property(band_mode[n].data()).value<int>()].toStdString();

    json[bandn]["slope"] =
        settings->bandSlopeLabels()[settings->property(band_slope[n].data()).value<int>()].toStdString();

    json[bandn]["solo"] = settings->property(band_solo[n].data()).value<bool>();

    json[bandn]["mute"] = settings->property(band_mute[n].data()).value<bool>();

    json[bandn]["gain"] = settings->property(band_gain[n].data()).value<double>();

    json[bandn]["frequency"] = settings->property(band_frequency[n].data()).value<double>();

    json[bandn]["q"] = settings->property(band_q[n].data()).value<double>();

    json[bandn]["width"] = settings->property(band_width[n].data()).value<double>();
  }
}

void EqualizerPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("num-bands", NumBands);
  UPDATE_PROPERTY("split-channels", SplitChannels);
  UPDATE_PROPERTY("balance", Balance);
  UPDATE_PROPERTY("pitch-left", PitchLeft);
  UPDATE_PROPERTY("pitch-right", PitchRight);

  UPDATE_ENUM_LIKE_PROPERTY("mode", Mode);

  const auto nbands = settings->numBands();

  if (section == "input") {
    load_channel(json.at(section).at(instance_name).at("left"), input_settings_left, nbands);
    load_channel(json.at(section).at(instance_name).at("right"), input_settings_right, nbands);
  } else if (section == "output") {
    load_channel(json.at(section).at(instance_name).at("left"), output_settings_left, nbands);
    load_channel(json.at(section).at(instance_name).at("right"), output_settings_right, nbands);
  }
}

void EqualizerPreset::load_channel(const nlohmann::json& json, db::EqualizerChannel* settings, const int& nbands) {
  for (int n = 0; n < nbands; n++) {
    const auto bandn = "band" + util::to_string(n);

    settings->setProperty(
        band_type[n].data(),
        settings->bandTypeLabels().indexOf(json.at(bandn).value(
            "type",
            settings->bandTypeLabels()[settings->getDefaultValue(band_type[n].data()).value<int>()].toStdString())));

    settings->setProperty(
        band_mode[n].data(),
        settings->bandModeLabels().indexOf(json.at(bandn).value(
            "mode",
            settings->bandModeLabels()[settings->getDefaultValue(band_mode[n].data()).value<int>()].toStdString())));

    settings->setProperty(
        band_slope[n].data(),
        settings->bandSlopeLabels().indexOf(json.at(bandn).value(
            "slope",
            settings->bandSlopeLabels()[settings->getDefaultValue(band_slope[n].data()).value<int>()].toStdString())));

    settings->setProperty(band_solo[n].data(),
                          json.at(bandn).value("solo", settings->getDefaultValue(band_solo[n].data()).value<bool>()));

    settings->setProperty(band_mute[n].data(),
                          json.at(bandn).value("mute", settings->getDefaultValue(band_mute[n].data()).value<bool>()));

    settings->setProperty(band_gain[n].data(),
                          json.at(bandn).value("gain", settings->getDefaultValue(band_gain[n].data()).value<double>()));

    settings->setProperty(
        band_frequency[n].data(),
        json.at(bandn).value("frequency", settings->getDefaultValue(band_frequency[n].data()).value<double>()));

    settings->setProperty(band_q[n].data(),
                          json.at(bandn).value("q", settings->getDefaultValue(band_q[n].data()).value<double>()));

    settings->setProperty(
        band_width[n].data(),
        json.at(bandn).value("width", settings->getDefaultValue(band_width[n].data()).value<double>()));
  }
}
