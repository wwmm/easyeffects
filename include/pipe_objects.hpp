/*
 *  Copyright © 2017-2025 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <pipewire/link.h>
#include <pipewire/node.h>
#include <pipewire/proxy.h>
#include <spa/param/param.h>
#include <spa/utils/defs.h>
#include <sys/types.h>
#include <cstdint>
#include <string>

struct NodeInfo {
  pw_proxy* proxy = nullptr;

  uint id = SPA_ID_INVALID;

  uint device_id = SPA_ID_INVALID;

  uint64_t serial = SPA_ID_INVALID;

  std::string name;

  std::string description;

  std::string media_class;

  std::string media_role;

  std::string app_name;

  std::string app_process_id;

  std::string app_process_binary;

  std::string app_icon_name;

  std::string media_icon_name;

  std::string device_icon_name;

  std::string media_name;

  std::string format;

  std::string application_id;

  int priority = -1;

  pw_node_state state = PW_NODE_STATE_IDLE;

  bool mute = false;

  bool connected = false;

  int n_input_ports = 0;

  int n_output_ports = 0;

  int rate = 1;  // used as divisor to calculate latency, so do not initialize it as 0

  uint n_volume_channels = 0U;

  float latency = 0.0F;

  float volume = 0.0F;
};

struct LinkInfo {
  std::string path;

  uint id = SPA_ID_INVALID;

  uint input_node_id = 0U;

  uint input_port_id = 0U;

  uint output_node_id = 0U;

  uint output_port_id = 0U;

  uint64_t serial = SPA_ID_INVALID;

  bool passive = false;  // does not cause the graph to be runnable

  pw_link_state state = PW_LINK_STATE_UNLINKED;
};

struct PortInfo {
  std::string path;

  std::string format_dsp;

  std::string audio_channel;

  std::string name;

  std::string direction;

  bool physical = false;

  bool terminal = false;

  bool monitor = false;

  uint id = SPA_ID_INVALID;

  uint node_id = 0U;

  uint port_id = 0U;

  uint64_t serial = SPA_ID_INVALID;
};

struct ModuleInfo {
  uint id;

  uint64_t serial = SPA_ID_INVALID;

  std::string name;

  std::string description;

  std::string filename;
};

struct ClientInfo {
  uint id;

  uint64_t serial = SPA_ID_INVALID;

  std::string name;

  std::string access;

  std::string api;
};

struct DeviceInfo {
  uint id;

  uint64_t serial = SPA_ID_INVALID;

  std::string name;

  std::string description;

  std::string nick;

  std::string media_class;

  std::string api;

  std::string input_route_name;

  std::string output_route_name;

  std::string bus_id;

  std::string bus_path;

  spa_param_availability input_route_available;

  spa_param_availability output_route_available;
};
