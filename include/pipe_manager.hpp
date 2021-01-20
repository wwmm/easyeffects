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

#ifndef PIPE_MANAGER_HPP
#define PIPE_MANAGER_HPP

#include <glibmm.h>
#include <pipewire/extensions/metadata.h>
#include <pipewire/pipewire.h>
#include <sigc++/sigc++.h>
#include <spa/param/audio/format-utils.h>
#include <spa/param/props.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include "spa/utils/defs.h"
#include "util.hpp"

struct NodeInfo {
  pw_proxy* proxy = nullptr;

  uint id = 0;

  std::string name;

  std::string description;

  std::string media_class;

  std::string icon_name;

  std::string media_name;

  std::string format;

  int priority = -1;

  pw_node_state state;

  bool mute = false;

  bool visible_to_user = false;

  int n_input_ports = 0;

  int n_output_ports = 0;

  uint rate = 0;

  int n_volume_channels = 0;

  float latency = 0.0F;

  float volume = 0.0F;
};

struct LinkInfo {
  std::string path;

  uint id = 0;

  uint input_node_id = 0;

  uint input_port_id = 0;

  uint output_node_id = 0;

  uint output_port_id = 0;

  bool passive = false;  // does not cause the graph to be runnable
};

struct ModuleInfo {
  uint id;

  std::string name;

  std::string description;

  std::string filename;
};

struct ClientInfo {
  uint id;

  std::string name;

  std::string access;

  std::string api;
};

class PipeManager {
 public:
  PipeManager();
  PipeManager(const PipeManager&) = delete;
  auto operator=(const PipeManager&) -> PipeManager& = delete;
  PipeManager(const PipeManager&&) = delete;
  auto operator=(const PipeManager&&) -> PipeManager& = delete;
  ~PipeManager();

  std::string log_tag = "pipe_manager: ";

  pw_thread_loop* thread_loop = nullptr;
  pw_core* core = nullptr;
  pw_registry* registry = nullptr;
  pw_metadata* metadata = nullptr;

  spa_hook metadata_listener{};

  std::vector<NodeInfo> list_nodes;

  std::vector<LinkInfo> list_links;

  std::vector<ModuleInfo> list_modules;

  std::vector<ClientInfo> list_clients;

  NodeInfo pe_sink_node, pe_source_node;

  NodeInfo default_sink, default_source;

  std::vector<std::string> blocklist_in;   // for input effects
  std::vector<std::string> blocklist_out;  // for output effects

  std::array<std::string, 13> blocklist_node_name = {"PulseEffects",
                                                     "pulseeffects",
                                                     "pulseeffects_soe",
                                                     "pulseeffects_sie",
                                                     "PulseEffectsWebrtcProbe",
                                                     "pavucontrol",
                                                     "PulseAudio Volume Control",
                                                     "libcanberra",
                                                     "gsd-media-keys",
                                                     "GNOME Shell",
                                                     "speech-dispatcher",
                                                     "Mutter",
                                                     "gameoverlayui"};

  std::array<std::string, 2> blocklist_media_role = {"event", "Notification"};

  std::string header_version, library_version, core_name, default_clock_rate, default_min_quantum, default_max_quantum,
      default_quantum;

  void connect_stream_output(const NodeInfo& nd_info) const;

  void disconnect_stream_output(const NodeInfo& nd_info) const;

  void connect_stream_input(const NodeInfo& nd_info) const;

  void disconnect_stream_input(const NodeInfo& nd_info) const;

  static void set_node_volume(const NodeInfo& nd_info, const float& value);

  static void set_node_mute(const NodeInfo& nd_info, const bool& state);

  sigc::signal<void, NodeInfo> source_added;
  sigc::signal<void, NodeInfo> source_changed;
  sigc::signal<void, NodeInfo> source_removed;
  sigc::signal<void, NodeInfo> sink_added;
  sigc::signal<void, NodeInfo> sink_changed;
  sigc::signal<void, NodeInfo> sink_removed;
  sigc::signal<void, NodeInfo> new_default_sink;
  sigc::signal<void, NodeInfo> new_default_source;
  sigc::signal<void, NodeInfo> stream_output_added;
  sigc::signal<void, NodeInfo> stream_output_changed;
  sigc::signal<void, NodeInfo> stream_output_removed;
  sigc::signal<void, NodeInfo> stream_input_added;
  sigc::signal<void, NodeInfo> stream_input_changed;
  sigc::signal<void, NodeInfo> stream_input_removed;

 private:
  bool context_ready = false;

  pw_context* context = nullptr;
  pw_proxy *proxy_stream_output_sink = nullptr, *proxy_stream_input_source = nullptr;

  spa_hook core_listener{}, registry_listener{};
};

#endif
