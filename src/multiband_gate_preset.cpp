/*
 *  Copyright © 2017-2025 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "multiband_gate_preset.hpp"
#include <qtypes.h>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_multiband_gate.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"
#include "tags_multiband_gate.hpp"
#include "util.hpp"

MultibandGatePreset::MultibandGatePreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::MultibandGate>(pipeline_type);
}

void MultibandGatePreset::save(nlohmann::json& json) {
  using namespace tags::multiband_gate;
  ;

  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["dry"] = settings->dry();

  json[section][instance_name]["wet"] = settings->wet();

  json[section][instance_name]["gate-mode"] =
      settings->defaultGateModeLabelsValue()[settings->gateMode()].toStdString();

  json[section][instance_name]["envelope-boost"] =
      settings->defaultEnvelopeBoostLabelsValue()[settings->envelopeBoost()].toStdString();

  json[section][instance_name]["stereo-split"] = settings->stereoSplit();

  for (uint n = 0U; n < tags::multiband_gate::n_bands; n++) {
    const auto nstr = util::to_string(n);
    const auto bandn = "band" + nstr;

    if (n > 0U) {
      json[section][instance_name][bandn]["enable-band"] = settings->property(band_enable[n].data()).value<bool>();

      json[section][instance_name][bandn]["split-frequency"] =
          settings->property(band_split_frequency[n].data()).value<double>();
    }

    json[section][instance_name][bandn]["gate-enable"] = settings->property(band_gate_enable[n].data()).value<bool>();

    json[section][instance_name][bandn]["mute"] = settings->property(band_mute[n].data()).value<bool>();

    json[section][instance_name][bandn]["solo"] = settings->property(band_solo[n].data()).value<bool>();

    json[section][instance_name][bandn]["attack-time"] = settings->property(band_attack_time[n].data()).value<double>();

    json[section][instance_name][bandn]["release-time"] =
        settings->property(band_release_time[n].data()).value<double>();

    json[section][instance_name][bandn]["hysteresis"] = settings->property(band_hysteresis[n].data()).value<bool>();

    json[section][instance_name][bandn]["hysteresis-threshold"] =
        settings->property(band_hysteresis_threshold[n].data()).value<double>();

    json[section][instance_name][bandn]["hysteresis-zone"] =
        settings->property(band_hysteresis_zone[n].data()).value<double>();

    json[section][instance_name][bandn]["curve-threshold"] =
        settings->property(band_curve_threshold[n].data()).value<double>();

    json[section][instance_name][bandn]["curve-zone"] = settings->property(band_curve_zone[n].data()).value<double>();

    json[section][instance_name][bandn]["reduction"] = settings->property(band_reduction[n].data()).value<double>();

    json[section][instance_name][bandn]["makeup"] = settings->property(band_makeup[n].data()).value<double>();

    json[section][instance_name][bandn]["sidechain-type"] =
        settings->sidechainTypeLabels()[settings->property(band_sidechain_type[n].data()).value<int>()].toStdString();

    json[section][instance_name][bandn]["sidechain-mode"] =
        settings->sidechainModeLabels()[settings->property(band_sidechain_mode[n].data()).value<int>()].toStdString();

    json[section][instance_name][bandn]["sidechain-source"] =
        settings->sidechainSourceLabels()[settings->property(band_sidechain_source[n].data()).value<int>()]
            .toStdString();

    json[section][instance_name][bandn]["stereo-split-source"] =
        settings->stereoSplitSourceLabels()[settings->property(band_stereo_split_source[n].data()).value<int>()]
            .toStdString();
  }
}

void MultibandGatePreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("dry", Dry);
  UPDATE_PROPERTY("wet", Wet);
  UPDATE_PROPERTY("stereo-split", StereoSplit);

  UPDATE_ENUM_LIKE_PROPERTY("gate-mode", GateMode);
  UPDATE_ENUM_LIKE_PROPERTY("envelope-boost", EnvelopeBoost);

  // TODO: Add band presets
  // ...
}
