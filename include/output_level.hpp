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

#ifndef OUTPUT_LEVEL_HPP
#define OUTPUT_LEVEL_HPP

#include "plugin_base.hpp"

class OutputLevel : public PluginBase {
 public:
  OutputLevel(const std::string& tag,
              const std::string& schema,
              const std::string& schema_path,
              PipeManager* pipe_manager);
  OutputLevel(const OutputLevel&) = delete;
  auto operator=(const OutputLevel&) -> OutputLevel& = delete;
  OutputLevel(const OutputLevel&&) = delete;
  auto operator=(const OutputLevel&&) -> OutputLevel& = delete;
  ~OutputLevel() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;
};

#endif
