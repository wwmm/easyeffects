/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "compressor_preset.hpp"

CompressorPreset::CompressorPreset()
    : input_settings(Gio::Settings::create("com.github.wwmm.easyeffects.compressor",
                                           "/com/github/wwmm/easyeffects/streaminputs/compressor/")),
      output_settings(Gio::Settings::create("com.github.wwmm.easyeffects.compressor",
                                            "/com/github/wwmm/easyeffects/streamoutputs/compressor/")) {}

void CompressorPreset::save(boost::property_tree::ptree& root,
                            const std::string& section,
                            const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".compressor.input-gain", settings->get_double("input-gain"));

  root.put(section + ".compressor.output-gain", settings->get_double("output-gain"));

  root.put(section + ".compressor.mode", settings->get_string("mode"));

  root.put(section + ".compressor.attack", settings->get_double("attack"));

  root.put(section + ".compressor.release", settings->get_double("release"));

  root.put(section + ".compressor.release-threshold", settings->get_double("release-threshold"));

  root.put(section + ".compressor.threshold", settings->get_double("threshold"));

  root.put(section + ".compressor.ratio", settings->get_double("ratio"));

  root.put(section + ".compressor.knee", settings->get_double("knee"));

  root.put(section + ".compressor.makeup", settings->get_double("makeup"));

  root.put(section + ".compressor.boost-threshold", settings->get_double("boost-threshold"));

  root.put(section + ".compressor.sidechain.listen", settings->get_boolean("sidechain-listen"));

  root.put(section + ".compressor.sidechain.type", settings->get_string("sidechain-type"));

  root.put(section + ".compressor.sidechain.mode", settings->get_string("sidechain-mode"));

  root.put(section + ".compressor.sidechain.source", settings->get_string("sidechain-source"));

  root.put(section + ".compressor.sidechain.preamp", settings->get_double("sidechain-preamp"));

  root.put(section + ".compressor.sidechain.reactivity", settings->get_double("sidechain-reactivity"));

  root.put(section + ".compressor.sidechain.lookahead", settings->get_double("sidechain-lookahead"));

  root.put(section + ".compressor.hpf-mode", settings->get_string("hpf-mode"));

  root.put(section + ".compressor.hpf-frequency", settings->get_double("hpf-frequency"));

  root.put(section + ".compressor.lpf-mode", settings->get_string("lpf-mode"));

  root.put(section + ".compressor.lpf-frequency", settings->get_double("lpf-frequency"));
}

void CompressorPreset::load(const nlohmann::json& json,
                            const std::string& section,
                            const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("compressor"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("compressor"), settings, "output-gain", "output-gain");

  update_string_key(json.at(section).at("compressor"), settings, "mode", "mode");

  update_key<double>(json.at(section).at("compressor"), settings, "attack", "attack");

  update_key<double>(json.at(section).at("compressor"), settings, "release", "release");

  update_key<double>(json.at(section).at("compressor"), settings, "release-threshold", "release-threshold");

  update_key<double>(json.at(section).at("compressor"), settings, "threshold", "threshold");

  update_key<double>(json.at(section).at("compressor"), settings, "ratio", "ratio");

  update_key<double>(json.at(section).at("compressor"), settings, "knee", "knee");

  update_key<double>(json.at(section).at("compressor"), settings, "makeup", "makeup");

  update_key<double>(json.at(section).at("compressor"), settings, "boost-threshold", "boost-threshold");

  update_key<bool>(json.at(section).at("compressor").at("sidechain"), settings, "sidechain-listen", "listen");

  update_string_key(json.at(section).at("compressor").at("sidechain"), settings, "sidechain-type", "type");

  update_string_key(json.at(section).at("compressor").at("sidechain"), settings, "sidechain-mode", "mode");

  update_string_key(json.at(section).at("compressor").at("sidechain"), settings, "sidechain-source", "source");

  update_key<double>(json.at(section).at("compressor").at("sidechain"), settings, "sidechain-preamp", "preamp");

  update_key<double>(json.at(section).at("compressor").at("sidechain"), settings, "sidechain-reactivity", "reactivity");

  update_key<double>(json.at(section).at("compressor").at("sidechain"), settings, "sidechain-lookahead", "lookahead");

  update_string_key(json.at(section).at("compressor"), settings, "hpf-mode", "hpf-mode");

  update_key<double>(json.at(section).at("compressor"), settings, "hpf-frequency", "hpf-frequency");

  update_string_key(json.at(section).at("compressor"), settings, "lpf-mode", "lpf-mode");

  update_key<double>(json.at(section).at("compressor"), settings, "lpf-frequency", "lpf-frequency");
}

void CompressorPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  switch (preset_type) {
    case PresetType::output:
      save(root, "output", output_settings);
      break;
    case PresetType::input:
      save(root, "input", input_settings);
      break;
  }
}

void CompressorPreset::read(PresetType preset_type, const nlohmann::json& json) {
  try {
    switch (preset_type) {
      case PresetType::output:
        load(json, "output", output_settings);
        break;
      case PresetType::input:
        load(json, "input", input_settings);
        break;
    }
  } catch (const nlohmann::json::exception& e) {
    util::warning(e.what());
  }
}