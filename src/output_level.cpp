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

#include "output_level.hpp"

OutputLevel::OutputLevel(const std::string& tag,
                         const std::string& schema,
                         const std::string& schema_path,
                         PipeManager* pipe_manager)
    : PluginBase(tag, "output_level", schema, schema_path, pipe_manager) {}

OutputLevel::~OutputLevel() {}

void OutputLevel::process(const std::vector<float>& left_in,
                          const std::vector<float>& right_in,
                          std::span<float>& left_out,
                          std::span<float>& right_out) {
  //   util::warning(std::to_string(left_in.size()));

  float peak_l = 0.0F;
  float peak_r = 0.0F;

  for (const auto& v : left_in) {
    peak_l = (v > peak_l) ? v : peak_l;
  }

  for (const auto& v : right_in) {
    peak_r = (v > peak_r) ? v : peak_r;
  }

  float peak_l_db = util::linear_to_db(peak_l);
  float peak_r_db = util::linear_to_db(peak_r);

  Glib::signal_idle().connect_once([=] { level.emit(peak_l_db, peak_r_db); });

  std::copy(left_in.begin(), left_in.end(), left_out.begin());
  std::copy(right_in.begin(), right_in.end(), right_out.begin());
}