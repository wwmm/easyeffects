/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects
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

#pragma once

#include <map>
#include <numbers>
#include <string>
#include "easyeffects_db_equalizer.h"
#include "easyeffects_db_equalizer_channel.h"

namespace apo {

enum class Channel { left, right };

struct APO_Band {
  std::string type;
  float freq = 1000.0F;
  float gain = 0.0F;
  float quality = (1.0F / std::numbers::sqrt2_v<float>);  // default in LSP APO import
};

struct GraphicEQ_Band {
  float freq = 1000.0F;
  float gain = 0.0F;
};

std::map<std::string, std::string> const ApoToEasyEffectsFilter = {
    {"OFF", "Off"},         {"PK", "Bell"},          {"MODAL", "Bell"},       {"PEQ", "Bell"},    {"LP", "Lo-pass"},
    {"LPQ", "Lo-pass"},     {"HP", "Hi-pass"},       {"HPQ", "Hi-pass"},      {"BP", "Bandpass"}, {"LS", "Lo-shelf"},
    {"LSC", "Lo-shelf"},    {"LS 6DB", "Lo-shelf"},  {"LS 12DB", "Lo-shelf"}, {"HS", "Hi-shelf"}, {"HSC", "Hi-shelf"},
    {"HS 6DB", "Hi-shelf"}, {"HS 12DB", "Hi-shelf"}, {"NO", "Notch"},         {"AP", "Allpass"}};

std::map<std::string, std::string> const EasyEffectsToApoFilter = {
    {"Bell", "PK"},      {"Lo-pass", "LPQ"}, {"Hi-pass", "HPQ"}, {"Lo-shelf", "LSC"},
    {"Hi-shelf", "HSC"}, {"Notch", "NO"},    {"Allpass", "AP"},  {"Bandpass", "BP"}};

auto import_apo_preset(db::Equalizer* settings,
                       db::EqualizerChannel* settings_left,
                       db::EqualizerChannel* settings_right,
                       const std::string& file_path) -> bool;

auto import_graphiceq_preset(db::Equalizer* settings,
                             db::EqualizerChannel* settings_left,
                             db::EqualizerChannel* settings_right,
                             const std::string& file_path) -> bool;

auto export_apo_preset(db::Equalizer* settings,
                       db::EqualizerChannel* settings_left,
                       db::EqualizerChannel* settings_right,
                       const std::string& file_path) -> bool;

}  // namespace apo
