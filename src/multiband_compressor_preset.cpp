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

#include "multiband_compressor_preset.hpp"
#include <qtypes.h>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_multiband_compressor.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"
#include "tags_multiband_compressor.hpp"
#include "util.hpp"

MultibandCompressorPreset::MultibandCompressorPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::MultibandCompressor>(pipeline_type);
}

void MultibandCompressorPreset::save(nlohmann::json& json) {
  using namespace tags::multiband_compressor;

  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["dry"] = settings->dry();

  json[section][instance_name]["wet"] = settings->wet();

  json[section][instance_name]["compressor-mode"] =
      settings->defaultCompressorModeLabelsValue()[settings->compressorMode()].toStdString();

  json[section][instance_name]["envelope-boost"] =
      settings->defaultEnvelopeBoostLabelsValue()[settings->envelopeBoost()].toStdString();

  json[section][instance_name]["stereo-split"] = settings->stereoSplit();

  json[section][instance_name]["input-to-sidechain"] = settings->inputToSidechain();

  json[section][instance_name]["input-to-link"] = settings->inputToLink();

  json[section][instance_name]["sidechain-to-input"] = settings->sidechainToInput();

  json[section][instance_name]["sidechain-to-link"] = settings->sidechainToLink();

  json[section][instance_name]["link-to-input"] = settings->linkToInput();

  json[section][instance_name]["link-to-sidechain"] = settings->linkToSidechain();

  for (uint n = 0U; n < tags::multiband_compressor::n_bands; n++) {
    const auto nstr = util::to_string(n);
    const auto bandn = "band" + nstr;

    if (n > 0U) {
      json[section][instance_name][bandn]["enable-band"] = settings->property(band_enable[n].data()).value<bool>();

      json[section][instance_name][bandn]["split-frequency"] =
          settings->property(band_split_frequency[n].data()).value<double>();
    }

    json[section][instance_name][bandn]["compressor-enable"] =
        settings->property(band_compressor_enable[n].data()).value<bool>();

    json[section][instance_name][bandn]["mute"] = settings->property(band_mute[n].data()).value<bool>();

    json[section][instance_name][bandn]["solo"] = settings->property(band_solo[n].data()).value<bool>();

    json[section][instance_name][bandn]["attack-threshold"] =
        settings->property(band_attack_threshold[n].data()).value<double>();

    json[section][instance_name][bandn]["attack-time"] = settings->property(band_attack_time[n].data()).value<double>();

    json[section][instance_name][bandn]["release-threshold"] =
        settings->property(band_release_threshold[n].data()).value<double>();

    json[section][instance_name][bandn]["release-time"] =
        settings->property(band_release_time[n].data()).value<double>();

    json[section][instance_name][bandn]["ratio"] = settings->property(band_ratio[n].data()).value<double>();

    json[section][instance_name][bandn]["knee"] = settings->property(band_knee[n].data()).value<double>();

    json[section][instance_name][bandn]["makeup"] = settings->property(band_makeup[n].data()).value<double>();

    json[section][instance_name][bandn]["compression-mode"] =
        settings->compressionModeLabels()[settings->property(band_compression_mode[n].data()).value<int>()]
            .toStdString();

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

    json[section][instance_name][bandn]["sidechain-lookahead"] =
        settings->property(band_sidechain_lookahead[n].data()).value<double>();

    json[section][instance_name][bandn]["sidechain-reactivity"] =
        settings->property(band_sidechain_reactivity[n].data()).value<double>();

    json[section][instance_name][bandn]["sidechain-preamp"] =
        settings->property(band_sidechain_preamp[n].data()).value<double>();

    json[section][instance_name][bandn]["sidechain-custom-lowcut-filter"] =
        settings->property(band_lowcut_filter[n].data()).value<bool>();

    json[section][instance_name][bandn]["sidechain-custom-highcut-filter"] =
        settings->property(band_highcut_filter[n].data()).value<bool>();

    json[section][instance_name][bandn]["sidechain-lowcut-frequency"] =
        settings->property(band_lowcut_filter_frequency[n].data()).value<double>();

    json[section][instance_name][bandn]["sidechain-highcut-frequency"] =
        settings->property(band_highcut_filter_frequency[n].data()).value<double>();

    json[section][instance_name][bandn]["boost-threshold"] =
        settings->property(band_boost_threshold[n].data()).value<double>();

    json[section][instance_name][bandn]["boost-amount"] =
        settings->property(band_boost_amount[n].data()).value<double>();
  }
}

void MultibandCompressorPreset::load(const nlohmann::json& json) {
  using namespace tags::multiband_compressor;

  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("dry", Dry);
  UPDATE_PROPERTY("wet", Wet);
  UPDATE_PROPERTY("stereo-split", StereoSplit);
  UPDATE_PROPERTY("input-to-sidechain", InputToSidechain);
  UPDATE_PROPERTY("input-to-link", InputToLink);
  UPDATE_PROPERTY("sidechain-to-input", SidechainToInput);
  UPDATE_PROPERTY("sidechain-to-link", SidechainToLink);
  UPDATE_PROPERTY("link-to-input", LinkToInput);
  UPDATE_PROPERTY("link-to-sidechain", LinkToSidechain);

  UPDATE_ENUM_LIKE_PROPERTY("compressor-mode", CompressorMode);
  UPDATE_ENUM_LIKE_PROPERTY("envelope-boost", EnvelopeBoost);

  for (uint n = 0U; n < tags::multiband_compressor::n_bands; n++) {
    const auto nstr = util::to_string(n);
    auto jbandn = json.at(section).at(instance_name).at("band" + nstr);

    if (n > 0U) {
      settings->setProperty(
          band_enable[n].data(),
          jbandn.value("enable-band", settings->getDefaultValue(band_enable[n].data()).value<bool>()));

      settings->setProperty(
          band_split_frequency[n].data(),
          jbandn.value("split-frequency", settings->getDefaultValue(band_split_frequency[n].data()).value<double>()));
    }

    settings->setProperty(
        band_compressor_enable[n].data(),
        jbandn.value("compressor-enable", settings->getDefaultValue(band_compressor_enable[n].data()).value<bool>()));

    settings->setProperty(band_solo[n].data(),
                          jbandn.value("solo", settings->getDefaultValue(band_solo[n].data()).value<bool>()));

    settings->setProperty(band_mute[n].data(),
                          jbandn.value("mute", settings->getDefaultValue(band_mute[n].data()).value<bool>()));

    settings->setProperty(
        band_attack_time[n].data(),
        jbandn.value("attack-time", settings->getDefaultValue(band_attack_time[n].data()).value<double>()));

    settings->setProperty(
        band_attack_threshold[n].data(),
        jbandn.value("attack-threshold", settings->getDefaultValue(band_attack_threshold[n].data()).value<double>()));

    settings->setProperty(
        band_release_time[n].data(),
        jbandn.value("release-time", settings->getDefaultValue(band_release_time[n].data()).value<double>()));

    settings->setProperty(
        band_release_threshold[n].data(),
        jbandn.value("release-threshold", settings->getDefaultValue(band_release_threshold[n].data()).value<double>()));

    settings->setProperty(band_ratio[n].data(),
                          jbandn.value("ratio", settings->getDefaultValue(band_ratio[n].data()).value<double>()));

    settings->setProperty(band_knee[n].data(),
                          jbandn.value("knee", settings->getDefaultValue(band_knee[n].data()).value<double>()));

    settings->setProperty(band_makeup[n].data(),
                          jbandn.value("makeup", settings->getDefaultValue(band_makeup[n].data()).value<double>()));

    settings->setProperty(
        band_compression_mode[n].data(),
        settings->compressionModeLabels().indexOf(jbandn.value(
            "compression-mode",
            settings->compressionModeLabels()[settings->getDefaultValue(band_compression_mode[n].data()).value<int>()]
                .toStdString())));

    settings->setProperty(
        band_sidechain_type[n].data(),
        settings->sidechainTypeLabels().indexOf(jbandn.value(
            "sidechain-type",
            settings->sidechainTypeLabels()[settings->getDefaultValue(band_sidechain_type[n].data()).value<int>()]
                .toStdString())));

    settings->setProperty(
        band_sidechain_mode[n].data(),
        settings->sidechainModeLabels().indexOf(jbandn.value(
            "sidechain-mode",
            settings->sidechainModeLabels()[settings->getDefaultValue(band_sidechain_mode[n].data()).value<int>()]
                .toStdString())));

    settings->setProperty(
        band_sidechain_source[n].data(),
        settings->sidechainSourceLabels().indexOf(jbandn.value(
            "sidechain-source",
            settings->sidechainSourceLabels()[settings->getDefaultValue(band_sidechain_source[n].data()).value<int>()]
                .toStdString())));

    settings->setProperty(
        band_stereo_split_source[n].data(),
        settings->stereoSplitSourceLabels().indexOf(jbandn.value(
            "stereo-split-source",
            settings
                ->stereoSplitSourceLabels()[settings->getDefaultValue(band_stereo_split_source[n].data()).value<int>()]
                .toStdString())));

    settings->setProperty(band_sidechain_lookahead[n].data(),
                          jbandn.value("sidechain-lookahead",
                                       settings->getDefaultValue(band_sidechain_lookahead[n].data()).value<double>()));

    settings->setProperty(band_sidechain_reactivity[n].data(),
                          jbandn.value("sidechain-reactivity",
                                       settings->getDefaultValue(band_sidechain_reactivity[n].data()).value<double>()));

    settings->setProperty(
        band_sidechain_preamp[n].data(),
        jbandn.value("sidechain-preamp", settings->getDefaultValue(band_sidechain_preamp[n].data()).value<double>()));

    settings->setProperty(band_lowcut_filter[n].data(),
                          jbandn.value("sidechain-custom-lowcut-filter",
                                       settings->getDefaultValue(band_lowcut_filter[n].data()).value<bool>()));

    settings->setProperty(band_highcut_filter[n].data(),
                          jbandn.value("sidechain-custom-highcut-filter",
                                       settings->getDefaultValue(band_highcut_filter[n].data()).value<bool>()));

    settings->setProperty(
        band_lowcut_filter_frequency[n].data(),
        jbandn.value("sidechain-lowcut-frequency",
                     settings->getDefaultValue(band_lowcut_filter_frequency[n].data()).value<double>()));

    settings->setProperty(
        band_highcut_filter_frequency[n].data(),
        jbandn.value("sidechain-highcut-frequency",
                     settings->getDefaultValue(band_highcut_filter_frequency[n].data()).value<double>()));

    settings->setProperty(
        band_boost_threshold[n].data(),
        jbandn.value("boost-threshold", settings->getDefaultValue(band_boost_threshold[n].data()).value<double>()));

    settings->setProperty(
        band_boost_amount[n].data(),
        jbandn.value("boost-amount", settings->getDefaultValue(band_boost_amount[n].data()).value<double>()));
  }
}
