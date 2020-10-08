/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "limiter_preset.hpp"

LimiterPreset::LimiterPreset()
    : input_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.limiter",
                                           "/com/github/wwmm/pulseeffects/sourceoutputs/limiter/")),
      output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.limiter",
                                            "/com/github/wwmm/pulseeffects/sinkinputs/limiter/")) {}

void LimiterPreset::save(boost::property_tree::ptree& root,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".limiter.state", settings->get_boolean("state"));

  root.put(section + ".limiter.input-gain", settings->get_double("input-gain"));

  root.put(section + ".limiter.limit", settings->get_double("limit"));

  root.put(section + ".limiter.lookahead", settings->get_double("lookahead"));

  root.put(section + ".limiter.release", settings->get_double("release"));

  root.put(section + ".limiter.auto-level", settings->get_boolean("auto-level"));

  root.put(section + ".limiter.asc", settings->get_boolean("asc"));

  root.put(section + ".limiter.asc-level", settings->get_double("asc-level"));

  root.put(section + ".limiter.oversampling", settings->get_int("oversampling"));

  root.put(section + ".limiter.output-gain", settings->get_double("output-gain"));
}

void LimiterPreset::load(const boost::property_tree::ptree& root,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".limiter.state");

  update_key<double>(root, settings, "input-gain", section + ".limiter.input-gain");

  update_key<double>(root, settings, "limit", section + ".limiter.limit");

  update_key<double>(root, settings, "lookahead", section + ".limiter.lookahead");

  update_key<double>(root, settings, "release", section + ".limiter.release");

  update_key<bool>(root, settings, "auto-level", section + ".limiter.auto-level");

  update_key<bool>(root, settings, "asc", section + ".limiter.asc");

  update_key<double>(root, settings, "asc-level", section + ".limiter.asc-level");

  update_key<int>(root, settings, "oversampling", section + ".limiter.oversampling");

  update_key<double>(root, settings, "output-gain", section + ".limiter.output-gain");
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

void LimiterPreset::read(PresetType preset_type, const boost::property_tree::ptree& root) {
  switch (preset_type) {
    case PresetType::output:
      load(root, "output", output_settings);
      break;
    case PresetType::input:
      load(root, "input", input_settings);
      break;
  }
}
