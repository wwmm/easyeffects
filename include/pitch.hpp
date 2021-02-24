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

#ifndef PITCH_HPP
#define PITCH_HPP

#include <glibmm.h>
#include "plugin_base.hpp"

class Pitch : public PluginBase {
 public:
  Pitch(const std::string& tag, const std::string& schema, const std::string& schema_path);
  Pitch(const Pitch&) = delete;
  auto operator=(const Pitch&) -> Pitch& = delete;
  Pitch(const Pitch&&) = delete;
  auto operator=(const Pitch&&) -> Pitch& = delete;
  ~Pitch() override;

  GstElement* pitch = nullptr;

 private:
  void bind_to_gsettings();
};

#endif
