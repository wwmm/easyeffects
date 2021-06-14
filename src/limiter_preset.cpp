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

#include "limiter_preset.hpp"

LimiterPreset::LimiterPreset() {
  input_settings = Gio::Settings::create("com.github.wwmm.easyeffects.limiter",
                                         "/com/github/wwmm/easyeffects/streaminputs/limiter/");

  output_settings = Gio::Settings::create("com.github.wwmm.easyeffects.limiter",
                                          "/com/github/wwmm/easyeffects/streamoutputs/limiter/");
}

void LimiterPreset::save(boost::property_tree::ptree& root,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".limiter.input-gain", settings->get_double("input-gain"));

  root.put(section + ".limiter.output-gain", settings->get_double("output-gain"));

  root.put(section + ".limiter.limit", settings->get_double("limit"));

  root.put(section + ".limiter.lookahead", settings->get_double("lookahead"));

  root.put(section + ".limiter.release", settings->get_double("release"));

  root.put(section + ".limiter.auto-level", settings->get_boolean("auto-level"));

  root.put(section + ".limiter.asc", settings->get_boolean("asc"));

  root.put(section + ".limiter.asc-level", settings->get_double("asc-level"));

  root.put(section + ".limiter.oversampling", settings->get_int("oversampling"));
}

void LimiterPreset::load(const nlohmann::json& json,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("limiter"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("limiter"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("limiter"), settings, "limit", "limit");

  update_key<double>(json.at(section).at("limiter"), settings, "lookahead", "lookahead");

  update_key<double>(json.at(section).at("limiter"), settings, "release", "release");

  update_key<bool>(json.at(section).at("limiter"), settings, "auto-level", "auto-level");

  update_key<bool>(json.at(section).at("limiter"), settings, "asc", "asc");

  update_key<double>(json.at(section).at("limiter"), settings, "asc-level", "asc-level");

  update_key<int>(json.at(section).at("limiter"), settings, "oversampling", "oversampling");
}

void LimiterPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  switch (preset_type) {
    case PresetType::output:
      save(root, "output", output_settings);
      break;
    case PresetType::input:
      save(root, "input", input_settings);
      break;
  }
}
