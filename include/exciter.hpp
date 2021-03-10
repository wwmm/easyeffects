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

#ifndef EXCITER_HPP
#define EXCITER_HPP

#include <glibmm/main.h>
#include "plugin_base.hpp"

class Exciter : public PluginBase {
 public:
  Exciter(const std::string& tag, const std::string& schema, const std::string& schema_path, PipeManager* pipe_manager);
  Exciter(const Exciter&) = delete;
  auto operator=(const Exciter&) -> Exciter& = delete;
  Exciter(const Exciter&&) = delete;
  auto operator=(const Exciter&&) -> Exciter& = delete;
  ~Exciter() override;

  GstElement* exciter = nullptr;

  sigc::connection harmonics_connection;

  sigc::signal<void(double)> harmonics;

 private:
  void bind_to_gsettings();
};

#endif
