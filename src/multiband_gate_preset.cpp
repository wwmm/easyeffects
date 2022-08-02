/*
 *  Copyright © 2017-2022 Wellington Wallace
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

#include "multiband_gate_preset.hpp"

MultibandGatePreset::MultibandGatePreset(PresetType preset_type, const int& index)
    : PluginPresetBase(preset_type, index) {
  switch (preset_type) {
    case PresetType::input:
      settings = g_settings_new_with_path(tags::schema::multiband_gate::id, tags::schema::multiband_gate::input_path);
      break;
    case PresetType::output:
      settings = g_settings_new_with_path(tags::schema::multiband_gate::id, tags::schema::multiband_gate::output_path);
      break;
  }
}

void MultibandGatePreset::save(nlohmann::json& json) {
  json[section]["multiband_gate"]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section]["multiband_gate"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["multiband_gate"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["multiband_gate"]["dry"] = g_settings_get_double(settings, "dry");

  json[section]["multiband_gate"]["wet"] = g_settings_get_double(settings, "wet");

  json[section]["multiband_gate"]["gate-mode"] = util::gsettings_get_string(settings, "gate-mode");

  json[section]["multiband_gate"]["envelope-boost"] = util::gsettings_get_string(settings, "envelope-boost");

  for (uint n = 0U; n < n_bands; n++) {
    const auto nstr = util::to_string(n);
    const auto bandn = "band" + nstr;

    if (n > 0U) {
      json[section]["multiband_gate"][bandn]["enable-band"] =
          g_settings_get_boolean(settings, ("enable-band" + nstr).c_str()) != 0;

      json[section]["multiband_gate"][bandn]["split-frequency"] =
          g_settings_get_double(settings, ("split-frequency" + nstr).c_str());
    }

    json[section]["multiband_gate"][bandn]["gate-enable"] =
        g_settings_get_boolean(settings, ("gate-enable" + nstr).c_str()) != 0;

    json[section]["multiband_gate"][bandn]["solo"] = g_settings_get_boolean(settings, ("solo" + nstr).c_str()) != 0;

    json[section]["multiband_gate"][bandn]["mute"] = g_settings_get_boolean(settings, ("mute" + nstr).c_str()) != 0;

    json[section]["multiband_gate"][bandn]["attack-time"] =
        g_settings_get_double(settings, ("attack-time" + nstr).c_str());

    json[section]["multiband_gate"][bandn]["release-time"] =
        g_settings_get_double(settings, ("release-time" + nstr).c_str());

    json[section]["multiband_gate"][bandn]["hysteresis"] =
        g_settings_get_boolean(settings, ("hysteresis" + nstr).c_str()) != 0;

    json[section]["multiband_gate"][bandn]["hysteresis-threshold"] =
        g_settings_get_double(settings, ("hysteresis-threshold" + nstr).c_str());

    json[section]["multiband_gate"][bandn]["hysteresis-zone"] =
        g_settings_get_double(settings, ("hysteresis-zone" + nstr).c_str());

    json[section]["multiband_gate"][bandn]["curve-threshold"] =
        g_settings_get_double(settings, ("curve-threshold" + nstr).c_str());

    json[section]["multiband_gate"][bandn]["curve-zone"] =
        g_settings_get_double(settings, ("curve-zone" + nstr).c_str());

    json[section]["multiband_gate"][bandn]["reduction"] = g_settings_get_double(settings, ("reduction" + nstr).c_str());

    json[section]["multiband_gate"][bandn]["makeup"] = g_settings_get_double(settings, ("makeup" + nstr).c_str());

    json[section]["multiband_gate"][bandn]["external-sidechain"] =
        g_settings_get_boolean(settings, ("external-sidechain" + nstr).c_str()) != 0;

    json[section]["multiband_gate"][bandn]["sidechain-mode"] =
        util::gsettings_get_string(settings, ("sidechain-mode" + nstr).c_str());

    json[section]["multiband_gate"][bandn]["sidechain-source"] =
        util::gsettings_get_string(settings, ("sidechain-source" + nstr).c_str());

    json[section]["multiband_gate"][bandn]["sidechain-lookahead"] =
        g_settings_get_double(settings, ("sidechain-lookahead" + nstr).c_str());

    json[section]["multiband_gate"][bandn]["sidechain-reactivity"] =
        g_settings_get_double(settings, ("sidechain-reactivity" + nstr).c_str());

    json[section]["multiband_gate"][bandn]["sidechain-preamp"] =
        g_settings_get_double(settings, ("sidechain-preamp" + nstr).c_str());

    json[section]["multiband_gate"][bandn]["sidechain-custom-lowcut-filter"] =
        g_settings_get_boolean(settings, ("sidechain-custom-lowcut-filter" + nstr).c_str()) != 0;

    json[section]["multiband_gate"][bandn]["sidechain-custom-highcut-filter"] =
        g_settings_get_boolean(settings, ("sidechain-custom-highcut-filter" + nstr).c_str()) != 0;

    json[section]["multiband_gate"][bandn]["sidechain-lowcut-frequency"] =
        g_settings_get_double(settings, ("sidechain-lowcut-frequency" + nstr).c_str());

    json[section]["multiband_gate"][bandn]["sidechain-highcut-frequency"] =
        g_settings_get_double(settings, ("sidechain-highcut-frequency" + nstr).c_str());
  }
}

void MultibandGatePreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at("multiband_gate"), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at("multiband_gate"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("multiband_gate"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("multiband_gate"), settings, "dry", "dry");

  update_key<double>(json.at(section).at("multiband_gate"), settings, "wet", "wet");

  update_key<gchar*>(json.at(section).at("multiband_gate"), settings, "gate-mode", "gate-mode");

  update_key<gchar*>(json.at(section).at("multiband_gate"), settings, "envelope-boost", "envelope-boost");

  for (uint n = 0U; n < n_bands; n++) {
    const auto nstr = util::to_string(n);
    const auto bandn = "band" + nstr;

    if (n > 0U) {
      update_key<bool>(json.at(section).at("multiband_gate").at(bandn), settings, "enable-band" + nstr, "enable-band");

      update_key<double>(json.at(section).at("multiband_gate").at(bandn), settings, "split-frequency" + nstr,
                         "split-frequency");
    }

    update_key<bool>(json.at(section).at("multiband_gate").at(bandn), settings, "gate-enable" + nstr, "gate-enable");

    update_key<bool>(json.at(section).at("multiband_gate").at(bandn), settings, "solo" + nstr, "solo");

    update_key<bool>(json.at(section).at("multiband_gate").at(bandn), settings, "mute" + nstr, "mute");

    update_key<double>(json.at(section).at("multiband_gate").at(bandn), settings, "attack-time" + nstr, "attack-time");

    update_key<double>(json.at(section).at("multiband_gate").at(bandn), settings, "release-time" + nstr,
                       "release-time");

    update_key<bool>(json.at(section).at("multiband_gate").at(bandn), settings, "hysteresis" + nstr, "hysteresis");

    update_key<double>(json.at(section).at("multiband_gate").at(bandn), settings, "hysteresis-threshold" + nstr,
                       "hysteresis-threshold");

    update_key<double>(json.at(section).at("multiband_gate").at(bandn), settings, "hysteresis-zone" + nstr,
                       "hysteresis-zone");

    update_key<double>(json.at(section).at("multiband_gate").at(bandn), settings, "curve-threshold" + nstr,
                       "curve-threshold");

    update_key<double>(json.at(section).at("multiband_gate").at(bandn), settings, "curve-zone" + nstr, "curve-zone");

    update_key<double>(json.at(section).at("multiband_gate").at(bandn), settings, "reduction" + nstr, "reduction");

    update_key<double>(json.at(section).at("multiband_gate").at(bandn), settings, "makeup" + nstr, "makeup");

    update_key<bool>(json.at(section).at("multiband_gate").at(bandn), settings, "external-sidechain" + nstr,
                     "external-sidechain");

    update_key<gchar*>(json.at(section).at("multiband_gate").at(bandn), settings, "sidechain-mode" + nstr,
                       "sidechain-mode");

    update_key<gchar*>(json.at(section).at("multiband_gate").at(bandn), settings, "sidechain-source" + nstr,
                       "sidechain-source");

    update_key<double>(json.at(section).at("multiband_gate").at(bandn), settings, "sidechain-lookahead" + nstr,
                       "sidechain-lookahead");

    update_key<double>(json.at(section).at("multiband_gate").at(bandn), settings, "sidechain-reactivity" + nstr,
                       "sidechain-reactivity");

    update_key<double>(json.at(section).at("multiband_gate").at(bandn), settings, "sidechain-preamp" + nstr,
                       "sidechain-preamp");

    update_key<bool>(json.at(section).at("multiband_gate").at(bandn), settings, "sidechain-custom-lowcut-filter" + nstr,
                     "sidechain-custom-lowcut-filter");

    update_key<bool>(json.at(section).at("multiband_gate").at(bandn), settings,
                     "sidechain-custom-highcut-filter" + nstr, "sidechain-custom-highcut-filter");

    update_key<double>(json.at(section).at("multiband_gate").at(bandn), settings, "sidechain-lowcut-frequency" + nstr,
                       "sidechain-lowcut-frequency");

    update_key<double>(json.at(section).at("multiband_gate").at(bandn), settings, "sidechain-highcut-frequency" + nstr,
                       "sidechain-highcut-frequency");
  }
}
