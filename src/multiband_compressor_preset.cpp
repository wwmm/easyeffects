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

#include "multiband_compressor_preset.hpp"

MultibandCompressorPreset::MultibandCompressorPreset() {
  input_settings = Gio::Settings::create("com.github.wwmm.easyeffects.multibandcompressor",
                                         "/com/github/wwmm/easyeffects/streaminputs/multibandcompressor/");

  output_settings = Gio::Settings::create("com.github.wwmm.easyeffects.multibandcompressor",
                                          "/com/github/wwmm/easyeffects/streamoutputs/multibandcompressor/");
}

void MultibandCompressorPreset::save(nlohmann::json& json,
                                     const std::string& section,
                                     const Glib::RefPtr<Gio::Settings>& settings) {
  // json[section]["multiband_compressor"]["input-gain"] = settings->get_double("input-gain");

  // json[section]["multiband_compressor"]["output-gain"] = settings->get_double("output-gain");

  // json[section]["multiband_compressor"]["freq0"] = settings->get_double("freq0");

  // json[section]["multiband_compressor"]["freq1"] = settings->get_double("freq1");

  // json[section]["multiband_compressor"]["freq2"] = settings->get_double("freq2");

  // json[section]["multiband_compressor"]["mode"] = settings->get_string("mode").c_str();

  // // sub band

  // json[section]["multiband_compressor"]["subband"]["threshold"] = settings->get_double("threshold0");

  // json[section]["multiband_compressor"]["subband"]["ratio"] = settings->get_double("ratio0");

  // json[section]["multiband_compressor"]["subband"]["attack"] = settings->get_double("attack0");

  // json[section]["multiband_compressor"]["subband"]["release"] = settings->get_double("release0");

  // json[section]["multiband_compressor"]["subband"]["makeup"] = settings->get_double("makeup0");

  // json[section]["multiband_compressor"]["subband"]["knee"] = settings->get_double("knee0");

  // json[section]["multiband_compressor"]["subband"]["detection"] = settings->get_string("detection0").c_str();

  // json[section]["multiband_compressor"]["subband"]["bypass"] = settings->get_boolean("bypass0");

  // json[section]["multiband_compressor"]["subband"]["solo"] = settings->get_boolean("solo0");

  // // low band

  // json[section]["multiband_compressor"]["lowband"]["threshold"] = settings->get_double("threshold1");

  // json[section]["multiband_compressor"]["lowband"]["ratio"] = settings->get_double("ratio1");

  // json[section]["multiband_compressor"]["lowband"]["attack"] = settings->get_double("attack1");

  // json[section]["multiband_compressor"]["lowband"]["release"] = settings->get_double("release1");

  // json[section]["multiband_compressor"]["lowband"]["makeup"] = settings->get_double("makeup1");

  // json[section]["multiband_compressor"]["lowband"]["knee"] = settings->get_double("knee1");

  // json[section]["multiband_compressor"]["lowband"]["detection"] = settings->get_string("detection1").c_str();

  // json[section]["multiband_compressor"]["lowband"]["bypass"] = settings->get_boolean("bypass1");

  // json[section]["multiband_compressor"]["lowband"]["solo"] = settings->get_boolean("solo1");

  // // mid band

  // json[section]["multiband_compressor"]["midband"]["threshold"] = settings->get_double("threshold2");

  // json[section]["multiband_compressor"]["midband"]["ratio"] = settings->get_double("ratio2");

  // json[section]["multiband_compressor"]["midband"]["attack"] = settings->get_double("attack2");

  // json[section]["multiband_compressor"]["midband"]["release"] = settings->get_double("release2");

  // json[section]["multiband_compressor"]["midband"]["makeup"] = settings->get_double("makeup2");

  // json[section]["multiband_compressor"]["midband"]["knee"] = settings->get_double("knee2");

  // json[section]["multiband_compressor"]["midband"]["detection"] = settings->get_string("detection2").c_str();

  // json[section]["multiband_compressor"]["midband"]["bypass"] = settings->get_boolean("bypass2");

  // json[section]["multiband_compressor"]["midband"]["solo"] = settings->get_boolean("solo2");

  // // high band

  // json[section]["multiband_compressor"]["highband"]["threshold"] = settings->get_double("threshold3");

  // json[section]["multiband_compressor"]["highband"]["ratio"] = settings->get_double("ratio3");

  // json[section]["multiband_compressor"]["highband"]["attack"] = settings->get_double("attack3");

  // json[section]["multiband_compressor"]["highband"]["release"] = settings->get_double("release3");

  // json[section]["multiband_compressor"]["highband"]["makeup"] = settings->get_double("makeup3");

  // json[section]["multiband_compressor"]["highband"]["knee"] = settings->get_double("knee3");

  // json[section]["multiband_compressor"]["highband"]["detection"] = settings->get_string("detection3").c_str();

  // json[section]["multiband_compressor"]["highband"]["bypass"] = settings->get_boolean("bypass3");

  // json[section]["multiband_compressor"]["highband"]["solo"] = settings->get_boolean("solo3");
}

void MultibandCompressorPreset::load(const nlohmann::json& json,
                                     const std::string& section,
                                     const Glib::RefPtr<Gio::Settings>& settings) {
  // update_key<double>(json.at(section).at("multiband_compressor"), settings, "input-gain", "input-gain");

  // update_key<double>(json.at(section).at("multiband_compressor"), settings, "output-gain", "output-gain");

  // update_key<double>(json.at(section).at("multiband_compressor"), settings, "freq0", "freq0");

  // update_key<double>(json.at(section).at("multiband_compressor"), settings, "freq1", "freq1");

  // update_key<double>(json.at(section).at("multiband_compressor"), settings, "freq2", "freq2");

  // update_string_key(json.at(section).at("multiband_compressor"), settings, "mode", "mode");

  // // sub band

  // update_key<double>(json.at(section).at("multiband_compressor").at("subband"), settings, "threshold0", "threshold");

  // update_key<double>(json.at(section).at("multiband_compressor").at("subband"), settings, "ratio0", "ratio");

  // update_key<double>(json.at(section).at("multiband_compressor").at("subband"), settings, "attack0", "attack");

  // update_key<double>(json.at(section).at("multiband_compressor").at("subband"), settings, "release0", "release");

  // update_key<double>(json.at(section).at("multiband_compressor").at("subband"), settings, "makeup0", "makeup");

  // update_key<double>(json.at(section).at("multiband_compressor").at("subband"), settings, "knee0", "knee");

  // update_string_key(json.at(section).at("multiband_compressor").at("subband"), settings, "detection0", "detection");

  // update_key<bool>(json.at(section).at("multiband_compressor").at("subband"), settings, "bypass0", "bypass");

  // update_key<bool>(json.at(section).at("multiband_compressor").at("subband"), settings, "solo0", "solo");

  // // low band

  // update_key<double>(json.at(section).at("multiband_compressor").at("lowband"), settings, "threshold1", "threshold");

  // update_key<double>(json.at(section).at("multiband_compressor").at("lowband"), settings, "ratio1", "ratio");

  // update_key<double>(json.at(section).at("multiband_compressor").at("lowband"), settings, "attack1", "attack");

  // update_key<double>(json.at(section).at("multiband_compressor").at("lowband"), settings, "release1", "release");

  // update_key<double>(json.at(section).at("multiband_compressor").at("lowband"), settings, "makeup1", "makeup");

  // update_key<double>(json.at(section).at("multiband_compressor").at("lowband"), settings, "knee1", "knee");

  // update_string_key(json.at(section).at("multiband_compressor").at("lowband"), settings, "detection1", "detection");

  // update_key<bool>(json.at(section).at("multiband_compressor").at("lowband"), settings, "bypass1", "bypass");

  // update_key<bool>(json.at(section).at("multiband_compressor").at("lowband"), settings, "solo1", "solo");

  // // mid band

  // update_key<double>(json.at(section).at("multiband_compressor").at("midband"), settings, "threshold2", "threshold");

  // update_key<double>(json.at(section).at("multiband_compressor").at("midband"), settings, "ratio2", "ratio");

  // update_key<double>(json.at(section).at("multiband_compressor").at("midband"), settings, "attack2", "attack");

  // update_key<double>(json.at(section).at("multiband_compressor").at("midband"), settings, "release2", "release");

  // update_key<double>(json.at(section).at("multiband_compressor").at("midband"), settings, "makeup2", "makeup");

  // update_key<double>(json.at(section).at("multiband_compressor").at("midband"), settings, "knee2", "knee");

  // update_string_key(json.at(section).at("multiband_compressor").at("midband"), settings, "detection2", "detection");

  // update_key<bool>(json.at(section).at("multiband_compressor").at("midband"), settings, "bypass2", "bypass");

  // update_key<bool>(json.at(section).at("multiband_compressor").at("midband"), settings, "solo2", "solo");

  // // high band

  // update_key<double>(json.at(section).at("multiband_compressor").at("highband"), settings, "threshold3",
  // "threshold");

  // update_key<double>(json.at(section).at("multiband_compressor").at("highband"), settings, "ratio3", "ratio");

  // update_key<double>(json.at(section).at("multiband_compressor").at("highband"), settings, "attack3", "attack");

  // update_key<double>(json.at(section).at("multiband_compressor").at("highband"), settings, "release3", "release");

  // update_key<double>(json.at(section).at("multiband_compressor").at("highband"), settings, "makeup3", "makeup");

  // update_key<double>(json.at(section).at("multiband_compressor").at("highband"), settings, "knee3", "knee");

  // update_string_key(json.at(section).at("multiband_compressor").at("highband"), settings, "detection3", "detection");

  // update_key<bool>(json.at(section).at("multiband_compressor").at("highband"), settings, "bypass3", "bypass");

  // update_key<bool>(json.at(section).at("multiband_compressor").at("highband"), settings, "solo3", "solo");
}
