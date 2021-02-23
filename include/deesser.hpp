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

#ifndef DEESSER_HPP
#define DEESSER_HPP

#include "plugin_base.hpp"

class Deesser : public PluginBase {
 public:
  Deesser(const std::string& tag, const std::string& schema, const std::string& schema_path);
  Deesser(const Deesser&) = delete;
  auto operator=(const Deesser&) -> Deesser& = delete;
  Deesser(const Deesser&&) = delete;
  auto operator=(const Deesser&&) -> Deesser& = delete;
  ~Deesser() override;

  GstElement* deesser = nullptr;

  sigc::connection compression_connection, detected_connection;

  sigc::signal<void(double)> compression, detected;

 private:
  void bind_to_gsettings();
};

#endif
