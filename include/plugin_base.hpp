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

#ifndef PLUGIN_BASE_HPP
#define PLUGIN_BASE_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include "util.hpp"

class PluginBase {
 public:
  PluginBase(std::string tag, std::string plugin_name, const std::string& schema, const std::string& schema_path);
  PluginBase(const PluginBase&) = delete;
  auto operator=(const PluginBase&) -> PluginBase& = delete;
  PluginBase(const PluginBase&&) = delete;
  auto operator=(const PluginBase&&) -> PluginBase& = delete;
  virtual ~PluginBase();

  std::string log_tag, name;
  GstElement *plugin = nullptr, *bin = nullptr, *identity_in = nullptr, *identity_out = nullptr;

  bool plugin_is_installed = false;

  void enable();
  void disable();

 protected:
  GSettings* settings = nullptr;

  auto is_installed(GstElement* e) -> bool;
};

#endif
