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

#ifndef MULTIBAND_COMPRESSOR_PRESET_HPP
#define MULTIBAND_COMPRESSOR_PRESET_HPP

#include "plugin_preset_base.hpp"

class MultibandCompressorPreset : public PluginPresetBase {
 public:
  MultibandCompressorPreset();

 private:
  static constexpr uint n_bands = 8U;

  void save(nlohmann::json& json, const std::string& section, const Glib::RefPtr<Gio::Settings>& settings) override;

  void load(const nlohmann::json& json,
            const std::string& section,
            const Glib::RefPtr<Gio::Settings>& settings) override;
};

#endif
