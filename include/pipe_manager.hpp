/*
 *  Copyright © 2017-2023 Wellington Wallace
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

#include <pipewire/extensions/metadata.h>
#include <pipewire/pipewire.h>
#include <sigc++/sigc++.h>
#include <spa/monitor/device.h>
#include <spa/param/audio/format-utils.h>
#include <spa/param/audio/type-info.h>
#include <spa/param/props.h>
#include <spa/utils/json.h>
#include <spa/utils/keys.h>
#include <spa/utils/result.h>
#include <algorithm>
#include <array>
#include <map>
#include <memory>
#include <span>
#include "tags_app.hpp"
#include "tags_pipewire.hpp"
#include "util.hpp"

struct NodeInfo {
  pw_proxy* proxy = nullptr;

  uint id = SPA_ID_INVALID;

  uint device_id = SPA_ID_INVALID;

  uint64_t serial = SPA_ID_INVALID;

  std::string name;

  std::string description;

  std::string media_class;

  std::string media_role;

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

class PipeManager {
 public:
  PipeManager();
  PipeManager(const PipeManager&) = delete;
  auto operator=(const PipeManager&) -> PipeManager& = delete;
  PipeManager(const PipeManager&&) = delete;
  auto operator=(const PipeManager&&) -> PipeManager& = delete;
  ~PipeManager();

  pw_thread_loop* thread_loop = nullptr;
  pw_core* core = nullptr;
  pw_registry* registry = nullptr;
  pw_metadata* metadata = nullptr;

  inline static bool exiting = false;

  inline static bool exclude_monitor_stream = true;

  spa_hook metadata_listener{};

  std::map<uint64_t, NodeInfo> node_map;

  std::vector<LinkInfo> list_links;

  std::vector<PortInfo> list_ports;

  std::vector<ModuleInfo> list_modules;

  std::vector<ClientInfo> list_clients;

  std::vector<DeviceInfo> list_devices;

  std::string default_output_device_name, default_input_device_name;

  NodeInfo ee_sink_node, ee_source_node;
  NodeInfo output_device, input_device;

  constexpr static auto blocklist_node_name =
      std::to_array({"Easy Effects", "EasyEffects", "easyeffects", "easyeffects_soe", "easyeffects_sie",
                     "EasyEffectsWebrtcProbe", "libcanberra", "gsd-media-keys", "GNOME Shell", "speech-dispatcher",
                     "speech-dispatcher-dummy", "speech-dispatcher-espeak-ng", "Mutter", "gameoverlayui"});

  std::array<std::string, 2U> blocklist_app_id = {"org.PulseAudio.pavucontrol", "org.gnome.VolumeControl"};

  std::array<std::string, 2U> blocklist_media_role = {"event", "Notification"};

  std::string header_version, library_version, core_name;
  std::string default_clock_rate = "0";
  std::string default_min_quantum = "0";
  std::string default_max_quantum = "0";
  std::string default_quantum = "0";

  auto node_map_at_id(const uint& id) -> NodeInfo&;

  auto stream_is_connected(const uint& id, const std::string& media_class) -> bool;

  void connect_stream_output(const uint& id) const;

  void connect_stream_input(const uint& id) const;

  void disconnect_stream(const uint& id) const;

  static void set_node_volume(pw_proxy* proxy, const uint& n_vol_ch, const float& value);

  static void set_node_mute(pw_proxy* proxy, const bool& state);

  auto count_node_ports(const uint& node_id) -> uint;

  /*
    Links the output ports of the node output_node_id to the input ports of the node input_node_id
  */

  auto link_nodes(const uint& output_node_id,
                  const uint& input_node_id,
                  const bool& probe_link = false,
                  const bool& link_passive = true) -> std::vector<pw_proxy*>;

  void destroy_object(const int& id) const;

  /*
    Destroy all the filters links
  */

  void destroy_links(const std::vector<pw_proxy*>& list) const;

  void lock() const;

  void unlock() const;

  void sync_wait_unlock() const;

  static auto json_object_find(const char* obj, const char* key, char* value, const size_t& len) -> int;

  sigc::signal<void(const NodeInfo)> stream_output_added;
  sigc::signal<void(const NodeInfo)> stream_input_added;
  sigc::signal<void(const NodeInfo)> stream_output_changed;
  sigc::signal<void(const NodeInfo)> stream_input_changed;
  sigc::signal<void(const uint64_t)> stream_output_removed;
  sigc::signal<void(const uint64_t)> stream_input_removed;

  /*
    Do not pass NodeInfo by reference. Sometimes it dies before we use it and a segmentation fault happens.
  */

  sigc::signal<void(const NodeInfo)> source_added;
  sigc::signal<void(const NodeInfo)> source_changed;
  sigc::signal<void(const NodeInfo)> source_removed;
  sigc::signal<void(const NodeInfo)> sink_added;
  sigc::signal<void(const NodeInfo)> sink_changed;
  sigc::signal<void(const NodeInfo)> sink_removed;
  sigc::signal<void(const std::string)> new_default_sink_name;
  sigc::signal<void(const std::string)> new_default_source_name;
  sigc::signal<void(const DeviceInfo)> device_input_route_changed;
  sigc::signal<void(const DeviceInfo)> device_output_route_changed;

  sigc::signal<void(const LinkInfo)> link_changed;

 private:
  pw_context* context = nullptr;
  pw_proxy *proxy_stream_output_sink = nullptr, *proxy_stream_input_source = nullptr;

  spa_hook core_listener{}, registry_listener{};

  void set_metadata_target_node(const uint& origin_id, const uint& target_id, const uint64_t& target_serial) const;
};
