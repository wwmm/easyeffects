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

#ifndef GATE_HPP
#define GATE_HPP

#include <glibmm.h>
#include "plugin_base.hpp"
#include "util.hpp"

class Gate : public PluginBase {
 public:
  Gate(const std::string& tag, const std::string& schema, const std::string& schema_path);
  Gate(const Gate&) = delete;
  auto operator=(const Gate&) -> Gate& = delete;
  Gate(const Gate&&) = delete;
  auto operator=(const Gate&&) -> Gate& = delete;
  ~Gate() override;

  GstElement* gate = nullptr;

  sigc::connection gating_connection;

  sigc::signal<void(double)> gating;

 private:
  void bind_to_gsettings();
};

#endif
