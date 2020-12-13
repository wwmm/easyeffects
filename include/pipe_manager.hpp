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
#include <pipewire/pipewire.h>
#include <sigc++/sigc++.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>
#include "pipe_filter.hpp"

struct NodeInfo {
  uint id = 0;

  std::string type = "empty";

  std::string name = "empty";

  std::string description = "empty";

  std::string media_class = "empty";

  std::string icon_name = "empty";

  std::string media_name = "empty";

  int priority = -1;

  pw_node_state state;

  int n_input_ports = 0;

  int n_output_ports = 0;

  int rate = 0;

  float latency = 0;

  bool visible_to_user = false;

  bool connected = false;
};

struct PortInfo {
  std::string type = "empty";

  std::string path = "empty";

  std::string format_dsp = "empty";

  std::string audio_channel = "empty";

  std::string name = "empty";

  std::string direction = "empty";

  bool physical = false;

  bool terminal = false;

  bool monitor = false;

  uint id = 0;

  uint node_id = 0;
};

struct mySinkInfo {
  std::string name;
  uint index;
  std::string description;
};

struct mySourceInfo {
  std::string name;
  uint index;
  std::string description;
};

struct myModuleInfo {
  std::string name;
  uint index;
  std::string argument;
};

struct myClientInfo {
  std::string name;
  uint index;
  std::string binary;
};

struct AppInfo {
  std::string app_type;
  uint index;
  std::string name;
  std::string icon_name;
  std::string media_name;
  uint8_t channels;
  double volume;
  uint rate;
  std::string resampler;
  std::string format;
  int mute;
  bool connected;
  bool visible;
  uint buffer;
  uint latency;
  int corked;
  bool wants_to_play;
};

class ParseAppInfo;

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

  std::vector<NodeInfo> list_nodes;

  std::vector<PortInfo> list_ports;

  auto get_default_source() -> NodeInfo;

  auto get_default_sink() -> NodeInfo;

  std::shared_ptr<mySinkInfo> apps_sink_info;
  std::shared_ptr<mySinkInfo> mic_sink_info;

  std::vector<std::string> blocklist_in;   // for input effects
  std::vector<std::string> blocklist_out;  // for output effects

  std::array<std::string, 7> blocklist_node_name = {
      "PulseEffectsWebrtcProbe", "PulseAudio Volume Control", "libcanberra", "gsd-media-keys", "GNOME Shell",
      "speech-dispatcher",
  };

  auto move_sink_input_to_pulseeffects(const std::string& name, uint idx) -> bool;
  auto remove_sink_input_from_pulseeffects(const std::string& name, uint idx) -> bool;
  auto move_source_output_to_pulseeffects(const std::string& name, uint idx) -> bool;
  auto remove_source_output_from_pulseeffects(const std::string& name, uint idx) -> bool;
  void set_sink_input_volume(const std::string& name, uint idx, uint8_t channels, uint value);
  void set_sink_input_mute(const std::string& name, uint idx, bool state);
  void set_source_output_volume(const std::string& name, uint idx, uint8_t channels, uint value);
  void set_source_output_mute(const std::string& name, uint idx, bool state);
  void set_sink_volume_by_name(const std::string& name, uint8_t channels, uint value);

  sigc::signal<void, NodeInfo> source_added;
  sigc::signal<void, std::shared_ptr<mySourceInfo>> source_changed;
  sigc::signal<void, NodeInfo> source_removed;
  sigc::signal<void, NodeInfo> sink_added;
  sigc::signal<void, std::shared_ptr<mySinkInfo>> sink_changed;
  sigc::signal<void, NodeInfo> sink_removed;
  sigc::signal<void, std::string> new_default_sink;
  sigc::signal<void, std::string> new_default_source;
  sigc::signal<void, NodeInfo> stream_output_added;
  sigc::signal<void, NodeInfo> stream_output_changed;
  sigc::signal<void, NodeInfo> stream_output_removed;
  sigc::signal<void, NodeInfo> stream_input_added;
  sigc::signal<void, NodeInfo> stream_input_changed;
  sigc::signal<void, NodeInfo> stream_input_removed;
  sigc::signal<void> server_changed;
  sigc::signal<void, std::shared_ptr<myModuleInfo>> module_info;
  sigc::signal<void, std::shared_ptr<myClientInfo>> client_info;

 private:
  bool context_ready = false;

  pw_context* context = nullptr;
  pw_proxy* proxy_stream_output_sink = nullptr;

  spa_hook core_listener{}, registry_listener{};

  PipeFilter* filter = nullptr;

  std::array<std::string, 4> blocklist_media_name = {"pulsesink probe", "bell-window-system", "audio-volume-change",
                                                     "screen-capture"};

  std::array<std::string, 1> blocklist_media_role = {"event"};

  std::array<std::string, 4> blocklist_app_id = {"com.github.wwmm.pulseeffects.sinkinputs",
                                                 "com.github.wwmm.pulseeffects.sourceoutputs",
                                                 "org.PulseAudio.pavucontrol", "org.gnome.VolumeControl"};

  // void new_app(const pa_sink_input_info* info);

  // void new_app(const pa_source_output_info* info);

  // void changed_app(const pa_sink_input_info* info);

  // void changed_app(const pa_source_output_info* info);

  static void print_app_info(const std::shared_ptr<AppInfo>& info);

  // auto app_is_connected(const pa_sink_input_info* info) -> bool;

  // auto app_is_connected(const pa_source_output_info* info) -> bool;

  // static auto get_latency(const pa_sink_input_info* info) -> uint { return info->sink_usec; }

  // static auto get_latency(const pa_source_output_info* info) -> uint { return info->source_usec; }

  template <typename T>
  auto parse_app_info(const T& info) -> std::shared_ptr<AppInfo> {
    std::string app_name;
    std::string media_name;
    std::string media_role;
    std::string app_id;
    auto ai = std::make_shared<AppInfo>();
    bool forbidden_app = false;
    /*
        auto prop = pa_proplist_gets(info->proplist, "application.name");

        if (prop != nullptr) {
          app_name = prop;

          forbidden_app =
              std::find(std::begin(blocklist_apps), std::end(blocklist_apps), app_name) != std::end(blocklist_apps);

          if (forbidden_app) {
            return nullptr;
          }
        }

        prop = pa_proplist_gets(info->proplist, "media.name");

        if (prop != nullptr) {
          media_name = prop;

          if (app_name.empty()) {
            app_name = media_name;
          }

          forbidden_app = std::find(std::begin(blocklist_media_name), std::end(blocklist_media_name), media_name) !=
                          std::end(blocklist_media_name);

          if (forbidden_app) {
            return nullptr;
          }
        }

        prop = pa_proplist_gets(info->proplist, "media.role");

        if (prop != nullptr) {
          media_role = prop;

          forbidden_app = std::find(std::begin(blocklist_media_role), std::end(blocklist_media_role), media_role) !=
                          std::end(blocklist_media_role);

          if (forbidden_app) {
            return nullptr;
          }
        }

        prop = pa_proplist_gets(info->proplist, "application.id");

        if (prop != nullptr) {
          app_id = prop;

          forbidden_app =
              std::find(std::begin(blocklist_app_id), std::end(blocklist_app_id), app_id) != std::end(blocklist_app_id);

          if (forbidden_app) {
            return nullptr;
          }
        }

        prop = pa_proplist_gets(info->proplist, "application.icon_name");

        std::string icon_name;

        if (prop != nullptr) {
          icon_name = prop;
        } else {
          prop = pa_proplist_gets(info->proplist, "media.icon_name");

          if (prop != nullptr) {
            if (std::strcmp(prop, "audio-card-bluetooth") ==
                0) {  // there is no GTK icon with this name given by Pulseaudio =/
            } else {
              icon_name = "bluetooth-symbolic";
            }
          } else {
            icon_name = "audio-x-generic-symbolic";
          }
        }

        // Connection flag: it specifies only the primary state that can be enabled/disabled by the user
        ai->connected = app_is_connected(info);

        // Initialize visibility to true, it will be properly updated forward
        ai->visible = true;

        // linear volume
        ai->volume = 100.0 * (static_cast<double>(pa_cvolume_max(&info->volume)) / PA_VOLUME_NORM);

        if (info->resample_method) {
          ai->resampler = info->resample_method;
        } else {
          ai->resampler = "none";
        }

        ai->format = pa_sample_format_to_string(info->sample_spec.format);

        ai->index = info->index;
        ai->name = app_name;
        ai->media_name = media_name;
        ai->icon_name = icon_name;
        ai->channels = info->volume.channels;
        ai->rate = info->sample_spec.rate;
        ai->mute = info->mute;
        ai->buffer = info->buffer_usec;
        ai->latency = get_latency(info);
        ai->corked = info->corked;
        ai->wants_to_play = ai->connected && !ai->corked;

        */

    return ai;
  }
};

#endif
