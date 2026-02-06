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

enum class ApoFilter {
  OFF,
  PK,
  MODAL,
  PEQ,
  LP,
  LPQ,
  HP,
  HPQ,
  BP,
  LS,
  LSC,
  LS_6DB,
  LS_12DB,
  HS,
  HSC,
  HS_6DB,
  HS_12DB,
  NO,
  AP,
  UNKNOWN
};

struct APO_Band {
  ApoFilter type;
  float freq = 1000.0F;
  float gain = 0.0F;
  float quality = (1.0F / std::numbers::sqrt2_v<float>);  // default in LSP APO import
};

struct GraphicEQ_Band {
  float freq = 1000.0F;
  float gain = 0.0F;
};

std::map<ApoFilter, std::string> const ApoToEqualizerFilter = {
    {ApoFilter::OFF, "Off"},          {ApoFilter::PK, "Bell"},          {ApoFilter::MODAL, "Bell"},
    {ApoFilter::PEQ, "Bell"},         {ApoFilter::LP, "Lo-pass"},       {ApoFilter::LPQ, "Lo-pass"},
    {ApoFilter::HP, "Hi-pass"},       {ApoFilter::HPQ, "Hi-pass"},      {ApoFilter::BP, "Bandpass"},
    {ApoFilter::LS, "Lo-shelf"},      {ApoFilter::LSC, "Lo-shelf"},     {ApoFilter::LS_6DB, "Lo-shelf"},
    {ApoFilter::LS_12DB, "Lo-shelf"}, {ApoFilter::HS, "Hi-shelf"},      {ApoFilter::HSC, "Hi-shelf"},
    {ApoFilter::HS_6DB, "Hi-shelf"},  {ApoFilter::HS_12DB, "Hi-shelf"}, {ApoFilter::NO, "Notch"},
    {ApoFilter::AP, "Allpass"},       {ApoFilter::UNKNOWN, "Off"}};

std::map<std::string, ApoFilter> const EqualizerToApoFilter = {
    {"Bell", ApoFilter::PK},      {"Lo-pass", ApoFilter::LPQ},  {"Hi-pass", ApoFilter::HPQ},
    {"Lo-shelf", ApoFilter::LSC}, {"Hi-shelf", ApoFilter::HSC}, {"Notch", ApoFilter::NO},
    {"Allpass", ApoFilter::AP},   {"Bandpass", ApoFilter::BP}};

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
