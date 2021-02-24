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

#ifndef LIMITER_HPP
#define LIMITER_HPP

#include <glibmm.h>
#include <array>
#include "plugin_base.hpp"

class Limiter : public PluginBase {
 public:
  Limiter(const std::string& tag, const std::string& schema, const std::string& schema_path);
  Limiter(const Limiter&) = delete;
  auto operator=(const Limiter&) -> Limiter& = delete;
  Limiter(const Limiter&&) = delete;
  auto operator=(const Limiter&&) -> Limiter& = delete;
  ~Limiter() override;

  GstElement* limiter = nullptr;

  sigc::connection input_level_connection, output_level_connection, attenuation_connection;

  sigc::signal<void(std::array<double, 2>)> input_level, output_level;
  sigc::signal<void(double)> attenuation;

 private:
  void bind_to_gsettings();
};

#endif
