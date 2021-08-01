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
  json[section]["multiband_compressor"]["input-gain"] = settings->get_double("input-gain");

  json[section]["multiband_compressor"]["output-gain"] = settings->get_double("output-gain");

  json[section]["multiband_compressor"]["compressor-mode"] = settings->get_string("compressor-mode").c_str();

  json[section]["multiband_compressor"]["envelope-boost"] = settings->get_string("envelope-boost").c_str();

  for (uint n = 0; n < n_bands; n++) {
    auto nstr = std::to_string(n);

    if (n > 0) {
      json[section]["multiband_compressor"]["band" + nstr]["enable-band"] = settings->get_boolean("enable-band" + nstr);

      json[section]["multiband_compressor"]["band" + nstr]["split-frequency"] =
          settings->get_double("split-frequency" + nstr);
    }

    json[section]["multiband_compressor"]["band" + nstr]["compressor-enable"] =
        settings->get_boolean("compressor-enable" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["solo"] = settings->get_boolean("solo" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["mute"] = settings->get_boolean("mute" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["attack-threshold"] =
        settings->get_double("attack-threshold" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["attack-time"] = settings->get_double("attack-time" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["release-threshold"] =
        settings->get_double("release-threshold" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["release-time"] = settings->get_double("release-time" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["ratio"] = settings->get_double("ratio" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["knee"] = settings->get_double("knee" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["makeup"] = settings->get_double("makeup" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["compression-mode"] =
        settings->get_string("compression-mode" + nstr).c_str();

    json[section]["multiband_compressor"]["band" + nstr]["sidechain-mode"] =
        settings->get_string("sidechain-mode" + nstr).c_str();

    json[section]["multiband_compressor"]["band" + nstr]["sidechain-source"] =
        settings->get_string("sidechain-source" + nstr).c_str();

    json[section]["multiband_compressor"]["band" + nstr]["sidechain-lookahead"] =
        settings->get_double("sidechain-lookahead" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["sidechain-reactivity"] =
        settings->get_double("sidechain-reactivity" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["sidechain-preamp"] =
        settings->get_double("sidechain-preamp" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["sidechain-custom-lowcut-filter"] =
        settings->get_boolean("sidechain-custom-lowcut-filter" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["sidechain-custom-highcut-filter"] =
        settings->get_boolean("sidechain-custom-highcut-filter" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["sidechain-lowcut-frequency"] =
        settings->get_double("sidechain-lowcut-frequency" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["sidechain-highcut-frequency"] =
        settings->get_double("sidechain-highcut-frequency" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["boost-threshold"] =
        settings->get_double("boost-threshold" + nstr);

    json[section]["multiband_compressor"]["band" + nstr]["boost-amount"] = settings->get_double("boost-amount" + nstr);
  }
}

void MultibandCompressorPreset::load(const nlohmann::json& json,
                                     const std::string& section,
                                     const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("multiband_compressor"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("multiband_compressor"), settings, "output-gain", "output-gain");

  update_string_key(json.at(section).at("multiband_compressor"), settings, "compressor-mode", "compressor-mode");

  update_string_key(json.at(section).at("multiband_compressor"), settings, "envelope-boost", "envelope-boost");

  for (uint n = 0; n < n_bands; n++) {
    auto nstr = std::to_string(n);

    if (n > 0) {
      update_key<bool>(json.at(section).at("multiband_compressor").at("band" + nstr), settings, "enable-band" + nstr,
                       "enable-band");

      update_key<double>(json.at(section).at("multiband_compressor").at("band" + nstr), settings,
                         "split-frequency" + nstr, "split-frequency");
    }

    update_key<bool>(json.at(section).at("multiband_compressor").at("band" + nstr), settings,
                     "compressor-enable" + nstr, "compressor-enable");

    update_key<bool>(json.at(section).at("multiband_compressor").at("band" + nstr), settings, "solo" + nstr, "solo");

    update_key<bool>(json.at(section).at("multiband_compressor").at("band" + nstr), settings, "mute" + nstr, "mute");

    update_key<double>(json.at(section).at("multiband_compressor").at("band" + nstr), settings,
                       "attack-threshold" + nstr, "attack-threshold");

    update_key<double>(json.at(section).at("multiband_compressor").at("band" + nstr), settings, "attack-time" + nstr,
                       "attack-time");

    update_key<double>(json.at(section).at("multiband_compressor").at("band" + nstr), settings,
                       "release-threshold" + nstr, "release-threshold");

    update_key<double>(json.at(section).at("multiband_compressor").at("band" + nstr), settings, "release-time" + nstr,
                       "release-time");

    update_key<double>(json.at(section).at("multiband_compressor").at("band" + nstr), settings, "ratio" + nstr,
                       "ratio");

    update_key<double>(json.at(section).at("multiband_compressor").at("band" + nstr), settings, "knee" + nstr, "knee");

    update_key<double>(json.at(section).at("multiband_compressor").at("band" + nstr), settings, "makeup" + nstr,
                       "makeup");

    update_string_key(json.at(section).at("multiband_compressor").at("band" + nstr), settings,
                      "compression-mode" + nstr, "compression-mode");

    update_string_key(json.at(section).at("multiband_compressor").at("band" + nstr), settings, "sidechain-mode" + nstr,
                      "sidechain-mode");

    update_string_key(json.at(section).at("multiband_compressor").at("band" + nstr), settings,
                      "sidechain-source" + nstr, "sidechain-source");

    update_key<double>(json.at(section).at("multiband_compressor").at("band" + nstr), settings,
                       "sidechain-lookahead" + nstr, "sidechain-lookahead");

    update_key<double>(json.at(section).at("multiband_compressor").at("band" + nstr), settings,
                       "sidechain-reactivity" + nstr, "sidechain-reactivity");

    update_key<double>(json.at(section).at("multiband_compressor").at("band" + nstr), settings,
                       "sidechain-preamp" + nstr, "sidechain-preamp");

    update_key<bool>(json.at(section).at("multiband_compressor").at("band" + nstr), settings,
                     "sidechain-custom-lowcut-filter" + nstr, "sidechain-custom-lowcut-filter");

    update_key<bool>(json.at(section).at("multiband_compressor").at("band" + nstr), settings,
                     "sidechain-custom-highcut-filter" + nstr, "sidechain-custom-highcut-filter");

    update_key<double>(json.at(section).at("multiband_compressor").at("band" + nstr), settings,
                       "sidechain-lowcut-frequency" + nstr, "sidechain-lowcut-frequency");

    update_key<double>(json.at(section).at("multiband_compressor").at("band" + nstr), settings,
                       "sidechain-highcut-frequency" + nstr, "sidechain-highcut-frequency");

    update_key<double>(json.at(section).at("multiband_compressor").at("band" + nstr), settings,
                       "boost-threshold" + nstr, "boost-threshold");

    update_key<double>(json.at(section).at("multiband_compressor").at("band" + nstr), settings, "boost-amount" + nstr,
                       "boost-amount");
  }
}
