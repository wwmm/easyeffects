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

#ifndef COMPRESSOR_HPP
#define COMPRESSOR_HPP

#include <glibmm.h>
#include <array>
#include "plugin_base.hpp"

class Compressor : public PluginBase {
 public:
  Compressor(const std::string& tag, const std::string& schema, const std::string& schema_path);
  Compressor(const Compressor&) = delete;
  auto operator=(const Compressor&) -> Compressor& = delete;
  Compressor(const Compressor&&) = delete;
  auto operator=(const Compressor&&) -> Compressor& = delete;
  ~Compressor() override;

  GstElement* compressor = nullptr;

  sigc::connection input_level_connection, output_level_connection, reduction_connection, sidechain_connection,
      curve_connection;

  sigc::signal<void(std::array<double, 2>)> input_level, output_level;
  sigc::signal<void(double)> reduction, sidechain, curve;

 private:
  void bind_to_gsettings();
};

#endif
