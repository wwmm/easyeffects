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

#include "crossfeed_preset.hpp"

CrossfeedPreset::CrossfeedPreset()
    : input_settings(Gio::Settings::create("com.github.wwmm.easyeffects.crossfeed",
                                           "/com/github/wwmm/easyeffects/streaminputs/crossfeed/")),
      output_settings(Gio::Settings::create("com.github.wwmm.easyeffects.crossfeed",
                                            "/com/github/wwmm/easyeffects/streamoutputs/crossfeed/")) {}

void CrossfeedPreset::save(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".crossfeed.input-gain", settings->get_double("input-gain"));

  root.put(section + ".crossfeed.output-gain", settings->get_double("output-gain"));

  root.put(section + ".crossfeed.fcut", settings->get_int("fcut"));

  root.put(section + ".crossfeed.feed", settings->get_double("feed"));
}

void CrossfeedPreset::load(const nlohmann::json& json,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("crossfeed"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("crossfeed"), settings, "output-gain", "output-gain");

  update_key<int>(json.at(section).at("crossfeed"), settings, "fcut", "fcut");

  update_key<double>(json.at(section).at("crossfeed"), settings, "feed", "feed");
}

void CrossfeedPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  switch (preset_type) {
    case PresetType::output:
      save(root, "output", output_settings);
      break;
    case PresetType::input:
      save(root, "input", input_settings);
      break;
  }
}

void CrossfeedPreset::read(PresetType preset_type, const boost::property_tree::ptree& root) {}

void CrossfeedPreset::read(PresetType preset_type, const nlohmann::json& json) {
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
