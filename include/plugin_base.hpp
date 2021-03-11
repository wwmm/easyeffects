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

#include <giomm.h>
#include <pipewire/filter.h>
#include <sigc++/sigc++.h>
#include <span>
#include "pipe_manager.hpp"

class PluginBase {
 public:
  PluginBase(std::string tag,
             std::string plugin_name,
             const std::string& schema,
             const std::string& schema_path,
             PipeManager* pipe_manager);
  PluginBase(const PluginBase&) = delete;
  auto operator=(const PluginBase&) -> PluginBase& = delete;
  PluginBase(const PluginBase&&) = delete;
  auto operator=(const PluginBase&&) -> PluginBase& = delete;
  virtual ~PluginBase();

  struct data;

  struct port {
    struct data* data;
  };

  struct data {
    struct port *in_left = nullptr, *in_right = nullptr, *out_left = nullptr, *out_right = nullptr;

    PluginBase* pb = nullptr;
  };

  std::string log_tag, name;

  bool plugin_is_installed = false;

  pw_filter* filter = nullptr;

  [[nodiscard]] auto get_node_id() const -> uint;

  virtual void process(const std::vector<float>& left_in,
                       const std::vector<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out);

  void enable();
  void disable();

 protected:
  Glib::RefPtr<Gio::Settings> settings;

  PipeManager* pm = nullptr;

 private:
  spa_hook listener{};

  data pf_data = {};

  uint node_id = 0;
};

#endif
