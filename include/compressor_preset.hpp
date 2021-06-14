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

#ifndef COMPRESSOR_PRESET_HPP
#define COMPRESSOR_PRESET_HPP

#include "plugin_preset_base.hpp"

class CompressorPreset : public PluginPresetBase {
 public:
  CompressorPreset();

 private:
  void save(nlohmann::json& json, const std::string& section, const Glib::RefPtr<Gio::Settings>& settings) override;

  void load(const nlohmann::json& json,
            const std::string& section,
            const Glib::RefPtr<Gio::Settings>& settings) override;
};

#endif
