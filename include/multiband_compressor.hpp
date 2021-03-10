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

#ifndef MULTIBAND_COMPRESSOR_HPP
#define MULTIBAND_COMPRESSOR_HPP

#include <glibmm.h>
#include <array>
#include "plugin_base.hpp"

class MultibandCompressor : public PluginBase {
 public:
  MultibandCompressor(const std::string& tag,
                      const std::string& schema,
                      const std::string& schema_path,
                      PipeManager* pipe_manager);
  MultibandCompressor(const MultibandCompressor&) = delete;
  auto operator=(const MultibandCompressor&) -> MultibandCompressor& = delete;
  MultibandCompressor(const MultibandCompressor&&) = delete;
  auto operator=(const MultibandCompressor&&) -> MultibandCompressor& = delete;
  ~MultibandCompressor() override;

  GstElement* multiband_compressor = nullptr;

  sigc::connection input_level_connection, output_level_connection;

  sigc::connection output0_connection, output1_connection, output2_connection, output3_connection,
      compression0_connection, compression1_connection, compression2_connection, compression3_connection;

  sigc::signal<void(std::array<double, 2>)> input_level, output_level;

  sigc::signal<void(double)> output0, output1, output2, output3, compression0, compression1, compression2, compression3;

 private:
  void bind_to_gsettings();
};

#endif
