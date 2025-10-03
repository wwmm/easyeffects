/**
 * Copyright © 2017-2025 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "pw_manager.hpp"
#include <pipewire/client.h>
#include <pipewire/context.h>
#include <pipewire/core.h>
#include <pipewire/device.h>
#include <pipewire/extensions/metadata.h>
#include <pipewire/keys.h>
#include <pipewire/link.h>
#include <pipewire/module.h>
#include <pipewire/node.h>
#include <pipewire/pipewire.h>
#include <pipewire/port.h>
#include <pipewire/properties.h>
#include <pipewire/proxy.h>
#include <pipewire/thread-loop.h>
#include <pipewire/version.h>
#include <qmap.h>
#include <qobjectdefs.h>
#include <qqml.h>
#include <qtmetamacros.h>
#include <qvariant.h>
#include <spa/monitor/device.h>
#include <spa/param/audio/raw-types.h>
#include <spa/param/audio/raw.h>
#include <spa/param/format.h>
#include <spa/param/param.h>
#include <spa/param/props.h>
#include <spa/param/route.h>
#include <spa/pod/builder.h>  // NOLINT
#include <spa/pod/iter.h>
#include <spa/pod/parser.h>
#include <spa/pod/pod.h>
#include <spa/pod/vararg.h>
#include <spa/utils/defs.h>
#include <spa/utils/dict.h>
#include <spa/utils/hook.h>
#include <spa/utils/keys.h>
#include <spa/utils/result.h>
#include <spa/utils/type.h>
#include <sys/types.h>
#include <QString>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <format>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <span>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include "config.h"
#include "db_manager.hpp"
#include "pw_model_clients.hpp"
#include "pw_model_modules.hpp"
#include "pw_model_nodes.hpp"
#include "pw_objects.hpp"
#include "tags_app.hpp"
#include "tags_pipewire.hpp"
#include "util.hpp"

namespace {

struct node_data {
  pw_proxy* proxy = nullptr;

  spa_hook proxy_listener{};

  spa_hook object_listener{};

  pw::Manager* pm = nullptr;

  pw::NodeInfo* nd_info{};
};

struct proxy_data {
  pw_proxy* proxy = nullptr;

  spa_hook proxy_listener{};

  spa_hook object_listener{};

  pw::Manager* pm = nullptr;

  uint id = SPA_ID_INVALID;

  uint64_t serial = SPA_ID_INVALID;
};

template <typename T>
auto spa_dict_get_string(const spa_dict* props, const char* key, T& str) -> bool {
  // If we will use string views in the future, this template could be useful.
  if (const auto* s = spa_dict_lookup(props, key)) {
    str = s;

    return true;
  }

  return false;
}

template <typename T>
auto spa_dict_get_num(const spa_dict* props, const char* key, T& num) -> bool {
  if (const auto* n = spa_dict_lookup(props, key)) {
    return util::str_to_num(std::string(n), num);
  }

  return false;
}

auto spa_dict_get_bool(const spa_dict* props, const char* key, bool& b) -> bool {
  // Returning bool is for conversion success state.
  // The bool value is assigned to reference parameter.
  if (const auto* v = spa_dict_lookup(props, key)) {
    b = (std::strcmp(v, "true") == 0);

    return true;
  }

  return false;
}

void on_removed_proxy(void* data) {
  auto* const pd = static_cast<proxy_data*>(data);

  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  if (pd->object_listener.link.next != nullptr || pd->object_listener.link.prev != nullptr) {
    spa_hook_remove(&pd->object_listener);
  }

  pw_proxy_destroy(pd->proxy);
}

auto link_info_from_props(const spa_dict* props) -> pw::LinkInfo {
  pw::LinkInfo info;

  spa_dict_get_num(props, PW_KEY_LINK_ID, info.id);

  spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, info.serial);

  spa_dict_get_string(props, PW_KEY_OBJECT_PATH, info.path);

  spa_dict_get_num(props, PW_KEY_LINK_INPUT_NODE, info.input_node_id);

  spa_dict_get_num(props, PW_KEY_LINK_INPUT_PORT, info.input_port_id);

  spa_dict_get_num(props, PW_KEY_LINK_OUTPUT_NODE, info.output_node_id);

  spa_dict_get_num(props, PW_KEY_LINK_OUTPUT_PORT, info.output_port_id);

  spa_dict_get_bool(props, PW_KEY_LINK_PASSIVE, info.passive);

  return info;
}

auto port_info_from_props(const spa_dict* props) -> pw::PortInfo {
  pw::PortInfo info;

  spa_dict_get_num(props, PW_KEY_PORT_ID, info.port_id);

  spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, info.serial);

  spa_dict_get_string(props, PW_KEY_PORT_NAME, info.name);

  spa_dict_get_num(props, PW_KEY_NODE_ID, info.node_id);

  spa_dict_get_string(props, PW_KEY_PORT_DIRECTION, info.direction);

  spa_dict_get_string(props, PW_KEY_AUDIO_CHANNEL, info.audio_channel);

  spa_dict_get_string(props, PW_KEY_AUDIO_FORMAT, info.format_dsp);

  spa_dict_get_bool(props, PW_KEY_PORT_PHYSICAL, info.physical);

  spa_dict_get_bool(props, PW_KEY_PORT_TERMINAL, info.terminal);

  spa_dict_get_bool(props, PW_KEY_PORT_MONITOR, info.monitor);

  return info;
}

void on_removed_node_proxy(void* data) {
  auto* const nd = static_cast<node_data*>(data);

  spa_hook_remove(&nd->object_listener);

  pw_proxy_destroy(nd->proxy);
}

void on_destroy_node_proxy(void* data) {
  auto* const nd = static_cast<node_data*>(data);

  auto* const pm = nd->pm;

  spa_hook_remove(&nd->proxy_listener);

  nd->nd_info->proxy = nullptr;

  pm->model_nodes.remove_by_serial(nd->nd_info->serial);

  util::debug(std::format("{} {} {} has been removed", nd->nd_info->media_class.toStdString(), nd->nd_info->id,
                          nd->nd_info->name.toStdString()));

  delete nd->nd_info;
}

void on_node_info(void* object, const struct pw_node_info* info) {
  if (pw::Manager::exiting) {
    return;
  }

  auto* const nd = static_cast<node_data*>(object);

  auto* const pm = nd->pm;

  // Check if the node has to be removed

  bool ignore_node = false;

  // Exclude blocklisted App id.
  // To be checked here because PW_KEY_APP_ID is not set in on_registry_global.

  if (const auto* app_id = spa_dict_lookup(info->props, PW_KEY_APP_ID)) {
    if (std::ranges::find(pm->blocklist_app_id, app_id) != pm->blocklist_app_id.end()) {
      ignore_node = true;
    }
  }

  /**
   * Exclude capture streams.
   * Even PW_KEY_STREAM_CAPTURE_SINK is not set in on_registry_global.
   * Useful to exclude OBS recording streams.
   */

  if (const auto* is_capture_sink = spa_dict_lookup(info->props, PW_KEY_STREAM_CAPTURE_SINK)) {
    if (std::strcmp(is_capture_sink, "true") == 0 && pw::Manager::exclude_monitor_stream) {
      ignore_node = true;
    }
  }

  if (nd->nd_info->media_class == tags::pipewire::media_class::input_stream) {
    if (const auto* target_object = spa_dict_lookup(info->props, PW_KEY_TARGET_OBJECT)) {
      // target.object can a name or serial number:
      // https://gitlab.freedesktop.org/pipewire/pipewire/-/blob/master/src/pipewire/keys.h#L334

      uint64_t serial = SPA_ID_INVALID;

      auto input_device = pm->model_nodes.get_node_by_name(db::StreamInputs::inputDevice());

      if (util::str_to_num(target_object, serial)) {
        if (serial != SPA_ID_INVALID && (serial != input_device.serial && serial != pm->ee_source_node.serial)) {
          ignore_node = true;
        }
      } else if (target_object != input_device.name && target_object != pm->ee_source_node.name) {
        ignore_node = true;
      }

      if (ignore_node) {
        util::debug(
            std::format("The input stream {} does not have as target the same mic used as EE input: {}\n"
                        "The user wants it to record from device {}. We will ignore this stream.",
                        nd->nd_info->name.toStdString(), input_device.name.toStdString(), target_object));
      }
    }
  }

  if (nd->nd_info->media_class == tags::pipewire::media_class::output_stream) {
    if (const auto* target_object = spa_dict_lookup(info->props, PW_KEY_TARGET_OBJECT)) {
      // target.object can a name or serial number:
      // https://gitlab.freedesktop.org/pipewire/pipewire/-/blob/master/src/pipewire/keys.h#L334

      uint64_t serial = SPA_ID_INVALID;

      util::str_to_num(target_object, serial);

      auto output_device = pm->model_nodes.get_node_by_name(db::StreamOutputs::outputDevice());

      if (util::str_to_num(target_object, serial)) {
        if (serial != SPA_ID_INVALID && (serial != output_device.serial && serial != pm->ee_sink_node.serial)) {
          ignore_node = true;
        }
      } else if (target_object != output_device.name && target_object != pm->ee_sink_node.name) {
        ignore_node = true;
      }

      if (ignore_node) {
        util::debug(
            std::format("The output stream {} does not have as target the same output device used as EE: {}\n"
                        "The user wants it to play to device {}. We will ignore this stream.",
                        nd->nd_info->name.toStdString(), output_device.name.toStdString(), target_object));
      }
    }
  }

  if (ignore_node) {
    return;
  }

  // Chech for node info updates

  nd->nd_info->state = info->state;
  nd->nd_info->n_input_ports = static_cast<int>(info->n_input_ports);
  nd->nd_info->n_output_ports = static_cast<int>(info->n_output_ports);

  spa_dict_get_num(info->props, PW_KEY_PRIORITY_SESSION, nd->nd_info->priority);

  if (const auto* app_id = spa_dict_lookup(info->props, PW_KEY_APP_ID)) {
    nd->nd_info->application_id = app_id;
  }

  // spa_dict_get_string(props, PW_KEY_APP_PROCESS_BINARY, app_process_binary);

  if (const auto* app_name = spa_dict_lookup(info->props, PW_KEY_APP_NAME)) {
    nd->nd_info->app_name = app_name;
  }

  if (const auto* app_process_id = spa_dict_lookup(info->props, PW_KEY_APP_PROCESS_ID)) {
    nd->nd_info->app_process_id = app_process_id;
  }

  if (const auto* app_process_binary = spa_dict_lookup(info->props, PW_KEY_APP_PROCESS_BINARY)) {
    nd->nd_info->app_process_binary = app_process_binary;
  }

  if (const auto* app_icon_name = spa_dict_lookup(info->props, PW_KEY_APP_ICON_NAME)) {
    nd->nd_info->app_icon_name = app_icon_name;
  }

  if (const auto* media_icon_name = spa_dict_lookup(info->props, PW_KEY_MEDIA_ICON_NAME)) {
    nd->nd_info->media_icon_name = media_icon_name;
  }

  spa_dict_get_string(info->props, PW_KEY_DEVICE_ICON_NAME, nd->nd_info->device_icon_name);

  if (const auto* media_name = spa_dict_lookup(info->props, PW_KEY_MEDIA_NAME)) {
    nd->nd_info->media_name = media_name;
  }

  if (const auto* node_latency = spa_dict_lookup(info->props, PW_KEY_NODE_LATENCY)) {
    const auto str = std::string(node_latency);

    const auto delimiter_pos = str.find('/');

    int rate = 1;

    if (util::str_to_num(str.substr(delimiter_pos + 1U), rate)) {
      nd->nd_info->rate = rate;
    }

    float pw_lat = 0.0F;

    if (util::str_to_num(str.substr(0U, delimiter_pos), pw_lat)) {
      nd->nd_info->latency = pw_lat / static_cast<float>(nd->nd_info->rate);
    }
  }

  spa_dict_get_num(info->props, PW_KEY_DEVICE_ID, nd->nd_info->device_id);

  bool deviceProfileChanged = false;

  if (const auto* device_profile_name = spa_dict_lookup(info->props, "device.profile.name")) {
    if (nd->nd_info->device_profile_name != device_profile_name) {
      nd->nd_info->device_profile_name = device_profile_name;
      deviceProfileChanged = true;
    }
  }

  if (const auto* device_profile_description = spa_dict_lookup(info->props, "device.profile.description")) {
    nd->nd_info->device_profile_description = device_profile_description;
  }

  // sometimes PipeWire destroys the pointer before signal_idle is called,
  // therefore we make a copy

  if (nd->nd_info->connected != pm->stream_is_connected(info->id, nd->nd_info->media_class)) {
    nd->nd_info->connected = !nd->nd_info->connected;
  }

  if (nd->nd_info->media_class == tags::pipewire::media_class::source) {
    const auto nd_info_copy = *nd->nd_info;

    if (nd_info_copy.serial == pm->ee_source_node.serial) {
      pm->ee_source_node = nd_info_copy;
    }

  } else if (nd->nd_info->media_class == tags::pipewire::media_class::sink) {
    const auto nd_info_copy = *nd->nd_info;

    if (nd_info_copy.serial == pm->ee_sink_node.serial) {
      pm->ee_sink_node = nd_info_copy;
    }
  }

  auto connect_to_ee_sink = [&]() {
    if (db::Main::processAllOutputs() && !nd->nd_info->connected && !nd->nd_info->is_blocklisted) {
      // target.node for backward compatibility with old PW session managers
      // NOLINTNEXTLINE
      pw_metadata_set_property(pm->metadata, nd->nd_info->id, "target.node", "Spa:Id",
                               util::to_string(pm->ee_sink_node.id).c_str());

      // NOLINTNEXTLINE
      pw_metadata_set_property(pm->metadata, nd->nd_info->id, "target.object", "Spa:Id",
                               util::to_string(pm->ee_sink_node.serial).c_str());
    }
  };

  auto connect_to_ee_source = [&]() {
    if (db::Main::processAllInputs() && !nd->nd_info->connected && !nd->nd_info->is_blocklisted) {
      // target.node for backward compatibility with old PW session managers
      // NOLINTNEXTLINE
      pw_metadata_set_property(pm->metadata, nd->nd_info->id, "target.node", "Spa:Id",
                               util::to_string(pm->ee_source_node.id).c_str());

      // NOLINTNEXTLINE
      pw_metadata_set_property(pm->metadata, nd->nd_info->id, "target.object", "Spa:Id",
                               util::to_string(pm->ee_source_node.serial).c_str());
    }
  };

  // update NodeInfo or add it if it is not in the model yet

  if (!pm->model_nodes.has_serial(nd->nd_info->serial)) {
    pm->model_nodes.append(*nd->nd_info);

    auto nd_info_copy = *nd->nd_info;

    if (nd_info_copy.media_class == tags::pipewire::media_class::source &&
        nd_info_copy.name != tags::pipewire::ee_source_name) {
      Q_EMIT pm->sourceAdded(nd_info_copy);
    } else if (nd_info_copy.media_class == tags::pipewire::media_class::sink &&
               nd_info_copy.name != tags::pipewire::ee_sink_name) {
      Q_EMIT pm->sinkAdded(nd_info_copy);
    } else if (nd_info_copy.media_class == tags::pipewire::media_class::output_stream) {
      connect_to_ee_sink();
    } else if (nd_info_copy.media_class == tags::pipewire::media_class::input_stream) {
      connect_to_ee_source();
    }

  } else {
    nd->nd_info->is_blocklisted = pm->model_nodes.get_node_by_id(nd->nd_info->id).is_blocklisted;

    pm->model_nodes.update_info(*nd->nd_info);

    if (nd->nd_info->media_class == tags::pipewire::media_class::output_stream) {
      connect_to_ee_sink();
    } else if (nd->nd_info->media_class == tags::pipewire::media_class::input_stream) {
      connect_to_ee_source();
    }
  }

  if (deviceProfileChanged) {
    if (nd->nd_info->media_class == tags::pipewire::media_class::source &&
        nd->nd_info->name != tags::pipewire::ee_source_name) {
      Q_EMIT pm->sourceProfileNameChanged(*nd->nd_info);
    } else if (nd->nd_info->media_class == tags::pipewire::media_class::sink &&
               nd->nd_info->name != tags::pipewire::ee_sink_name) {
      Q_EMIT pm->sinkProfileNameChanged(*nd->nd_info);
    }
  }

  if ((info->change_mask & PW_NODE_CHANGE_MASK_PARAMS) != 0U) {
    auto params = std::span(info->params, info->n_params);

    for (auto param : params) {
      if ((param.flags & SPA_PARAM_INFO_READ) == 0U) {
        continue;
      }

      if (const auto id = param.id; id == SPA_PARAM_Props || id == SPA_PARAM_EnumFormat || id == SPA_PARAM_Format) {
        pw_node_enum_params((struct pw_node*)nd->proxy, 0, id, 0, -1, nullptr);  // NOLINT
      }
    }
  }

  // const struct spa_dict_item* item = nullptr;
  // spa_dict_for_each(item, info->props) printf("\t\t%s: \"%s\"\n", item->key, item->value);
}

void on_node_event_param(void* object,
                         [[maybe_unused]] int seq,
                         [[maybe_unused]] uint32_t id,
                         [[maybe_unused]] uint32_t index,
                         [[maybe_unused]] uint32_t next,
                         const struct spa_pod* param) {
  if (pw::Manager::exiting || param == nullptr) {
    return;
  }

  auto* const nd = static_cast<node_data*>(object);

  auto* const pm = nd->pm;

  const auto serial = nd->nd_info->serial;

  const auto rowIndex = pm->model_nodes.get_row_by_serial(serial);

  if (rowIndex == -1) {
    return;
  }

  spa_pod_prop* pod_prop = nullptr;
  auto* obj = (spa_pod_object*)param;

  SPA_POD_OBJECT_FOREACH(obj, pod_prop) {
    switch (pod_prop->key) {
      case SPA_FORMAT_AUDIO_format: {
        uint format = 0U;

        if (spa_pod_get_id(&pod_prop->value, &format) != 0) {
          break;
        }

        QString format_str = "unknown";

        for (const auto type_info : std::to_array(spa_type_audio_format)) {
          if (format == type_info.type) {
            if (type_info.name != nullptr) {
              QString long_name = type_info.name;

              format_str = long_name.sliced(long_name.lastIndexOf(":") + 1);
            }
          }
        }

        if (format_str != nd->nd_info->format) {
          nd->nd_info->format = format_str;

          pm->model_nodes.update_field(rowIndex, pw::models::Nodes::Roles::Format, format_str);
        }

        break;
      }
      case SPA_FORMAT_AUDIO_rate: {
        int rate = 1;

        if (spa_pod_get_int(&pod_prop->value, &rate) != 0) {
          break;
        }

        if (rate == nd->nd_info->rate) {
          break;
        }

        nd->nd_info->rate = rate;

        pm->model_nodes.update_field(rowIndex, pw::models::Nodes::Roles::Rate, rate);

        break;
      }
      case SPA_PROP_mute: {
        auto v = false;

        if (spa_pod_get_bool(&pod_prop->value, &v) != 0) {
          break;
        }

        if (v == nd->nd_info->mute) {
          break;
        }

        nd->nd_info->mute = v;

        pm->model_nodes.update_field(rowIndex, pw::models::Nodes::Roles::Mute, v);

        break;
      }
      case SPA_PROP_channelVolumes: {
        std::array<float, SPA_AUDIO_MAX_CHANNELS> volumes{};

        const auto n_volumes =
            spa_pod_copy_array(&pod_prop->value, SPA_TYPE_Float, volumes.data(), SPA_AUDIO_MAX_CHANNELS);

        float max = 0.0F;

        for (uint i = 0U; i < n_volumes; i++) {
          max = std::max(volumes.at(i), max);
        }

        if (n_volumes != nd->nd_info->n_volume_channels || max != nd->nd_info->volume) {
          nd->nd_info->n_volume_channels = n_volumes;
          nd->nd_info->volume = max;

          pm->model_nodes.update_field(rowIndex, pw::models::Nodes::NvolumeChannels, n_volumes);
          pm->model_nodes.update_field(rowIndex, pw::models::Nodes::Volume, max);
        }

        break;
      }
      default:
        break;
    }
  }

  if (nd->nd_info->serial == pm->ee_source_node.serial) {
    pm->ee_source_node = *nd->nd_info;
  }

  if (nd->nd_info->serial == pm->ee_sink_node.serial) {
    pm->ee_sink_node = *nd->nd_info;
  }
}

void on_link_info(void* object, const struct pw_link_info* info) {
  auto* const ld = static_cast<proxy_data*>(object);
  auto* const pm = ld->pm;

  pw::LinkInfo link_copy;

  for (auto& l : ld->pm->list_links) {
    if (l.serial == ld->serial) {
      l.state = info->state;

      link_copy = l;

      Q_EMIT pm->linkChanged(link_copy);

      // util::warning(pw_link_state_as_string(l.state));

      break;
    }
  }

  // const struct spa_dict_item* item = nullptr;
  // spa_dict_for_each(item, info->props) printf("\t\t%s: \"%s\"\n", item->key, item->value);
}

void on_destroy_link_proxy(void* data) {
  auto* const ld = static_cast<proxy_data*>(data);

  spa_hook_remove(&ld->proxy_listener);

  auto it = std::ranges::remove_if(ld->pm->list_links, [=](const auto& n) { return n.serial == ld->serial; });

  ld->pm->list_links.erase(it.begin(), it.end());
}

void on_destroy_port_proxy(void* data) {
  auto* const pd = static_cast<proxy_data*>(data);

  spa_hook_remove(&pd->proxy_listener);

  auto it = std::ranges::remove_if(pd->pm->list_ports, [=](const auto& n) { return n.serial == pd->serial; });

  pd->pm->list_ports.erase(it.begin(), it.end());
}

void on_module_info(void* object, const struct pw_module_info* info) {
  auto* const md = static_cast<proxy_data*>(object);

  auto list = md->pm->model_modules.get_list();

  for (int n = 0; n < list.size(); n++) {
    if (list[n].id == info->id) {
      if (info->props != nullptr) {
        QString description;

        spa_dict_get_string(info->props, PW_KEY_MODULE_DESCRIPTION, description);

        md->pm->model_modules.update_field(n, pw::models::Modules::Roles::Description, description);
      }

      if (info->filename != nullptr) {
        md->pm->model_modules.update_field(n, pw::models::Modules::Roles::Filename, info->filename);
      }

      break;
    }
  }
}

void on_destroy_module_proxy(void* data) {
  auto* const md = static_cast<proxy_data*>(data);

  spa_hook_remove(&md->proxy_listener);

  md->pm->model_modules.remove_by_id(md->id);
}

void on_client_info(void* object, const struct pw_client_info* info) {
  auto* const cd = static_cast<proxy_data*>(object);

  auto list = cd->pm->model_clients.get_list();

  for (int n = 0; n < list.size(); n++) {
    if (list[n].id == info->id && info->props != nullptr) {
      QString name;
      QString access;
      QString api;

      spa_dict_get_string(info->props, PW_KEY_APP_NAME, name);

      spa_dict_get_string(info->props, PW_KEY_ACCESS, access);

      spa_dict_get_string(info->props, PW_KEY_CLIENT_API, api);

      cd->pm->model_clients.update_field(n, pw::models::Clients::Roles::Name, name);
      cd->pm->model_clients.update_field(n, pw::models::Clients::Roles::Access, access);
      cd->pm->model_clients.update_field(n, pw::models::Clients::Roles::Api, api);

      break;
    }
  }
}

void on_destroy_client_proxy(void* data) {
  auto* const cd = static_cast<proxy_data*>(data);

  spa_hook_remove(&cd->proxy_listener);

  cd->pm->model_clients.remove_by_id(cd->id);
}

auto on_metadata_property(void* data, uint32_t id, const char* key, const char* type, const char* value) -> int {
  auto* const pm = static_cast<pw::Manager*>(data);

  const std::string str_key = (key != nullptr) ? key : "";
  const std::string str_value = (value != nullptr) ? value : "";

  util::debug(std::format("new metadata property: {}, {}, {}, {}", id, str_key, type ? type : "", str_value));

  if (str_value.empty()) {
    return 0;
  }

  if (str_key == "default.audio.sink") {
    auto v = nlohmann::json::parse(str_value).value("name", "");

    if (v == tags::pipewire::ee_sink_name) {
      return 0;
    }

    util::debug(std::format("new default output device: {}", v));

    pm->defaultOutputDeviceName = QString::fromStdString(v);

    Q_EMIT pm->newDefaultSinkName(pm->defaultOutputDeviceName);
  }

  if (str_key == "default.audio.source") {
    auto v = nlohmann::json::parse(str_value).value("name", "");

    if (v == tags::pipewire::ee_source_name) {
      return 0;
    }

    util::debug(std::format("new default input device: {}", v));

    pm->defaultInputDeviceName = QString::fromStdString(v);

    Q_EMIT pm->newDefaultSourceName(pm->defaultInputDeviceName);
  }

  return 0;
}

const struct pw_metadata_events metadata_events = {.version = PW_VERSION_METADATA_EVENTS,
                                                   .property = on_metadata_property};

const struct pw_proxy_events link_proxy_events = {.version = 0,
                                                  .destroy = on_destroy_link_proxy,
                                                  .bound = nullptr,
                                                  .removed = on_removed_proxy,
                                                  .done = nullptr,
                                                  .error = nullptr,
                                                  .bound_props = nullptr};

const struct pw_proxy_events port_proxy_events = {.version = 0,
                                                  .destroy = on_destroy_port_proxy,
                                                  .bound = nullptr,
                                                  .removed = on_removed_proxy,
                                                  .done = nullptr,
                                                  .error = nullptr,
                                                  .bound_props = nullptr};

const struct pw_proxy_events module_proxy_events = {.version = 0,
                                                    .destroy = on_destroy_module_proxy,
                                                    .bound = nullptr,
                                                    .removed = on_removed_proxy,
                                                    .done = nullptr,
                                                    .error = nullptr,
                                                    .bound_props = nullptr};

const struct pw_proxy_events client_proxy_events = {.version = 0,
                                                    .destroy = on_destroy_client_proxy,
                                                    .bound = nullptr,
                                                    .removed = on_removed_proxy,
                                                    .done = nullptr,
                                                    .error = nullptr,
                                                    .bound_props = nullptr};

const struct pw_proxy_events node_proxy_events = {.version = 0,
                                                  .destroy = on_destroy_node_proxy,
                                                  .bound = nullptr,
                                                  .removed = on_removed_node_proxy,
                                                  .done = nullptr,
                                                  .error = nullptr,
                                                  .bound_props = nullptr};

const struct pw_node_events node_events = {.version = 0, .info = on_node_info, .param = on_node_event_param};

const struct pw_link_events link_events = {
    .version = 0,
    .info = on_link_info,
};

const struct pw_module_events module_events = {
    .version = 0,
    .info = on_module_info,
};

const struct pw_client_events client_events = {.version = 0, .info = on_client_info, .permissions = nullptr};

void on_registry_global(void* data,
                        uint32_t id,
                        [[maybe_unused]] uint32_t permissions,
                        const char* type,
                        [[maybe_unused]] uint32_t version,
                        const struct spa_dict* props) {
  if (id == SPA_ID_INVALID) {
    // If PipeWire send us a wrong id, we don't have issues
    return;
  }

  auto* const pm = static_cast<pw::Manager*>(data);

  if (std::strcmp(type, PW_TYPE_INTERFACE_Node) == 0) {
    bool is_ee_filter = false;

    if (const auto* key_media_role = spa_dict_lookup(props, PW_KEY_MEDIA_ROLE)) {
      if (std::strcmp(key_media_role, "DSP") == 0) {
        if (const auto* key_media_category = spa_dict_lookup(props, PW_KEY_MEDIA_CATEGORY)) {
          if (std::strcmp(key_media_category, "Filter") == 0) {
            if (const auto* key_node_name = spa_dict_lookup(props, PW_KEY_NODE_NAME)) {
              if (std::string node_name(key_node_name); node_name.size() > 3U) {
                if (node_name.starts_with("ee_")) {
                  is_ee_filter = true;
                }
              }
            }
          }
        }
      }
    }

    QString media_class;
    QString media_role;

    if (const auto* key_media_class = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS)) {
      media_class = key_media_class;
    }

    if (const auto* key_media_role = spa_dict_lookup(props, PW_KEY_MEDIA_ROLE)) {
      media_role = key_media_role;
    }

    constexpr auto class_array =
        std::to_array({tags::pipewire::media_class::output_stream, tags::pipewire::media_class::input_stream,
                       tags::pipewire::media_class::sink, tags::pipewire::media_class::source,
                       tags::pipewire::media_class::virtual_source});

    if (!is_ee_filter && !std::ranges::any_of(class_array, [&](const auto& str) { return str == media_class; })) {
      return;
    }

    QString node_name;

    spa_dict_get_string(props, PW_KEY_NODE_NAME, node_name);

    // At least for now I do not think there is a point in showing
    // the spectrum adn the output level filters in menus

    if (node_name.contains("output_level") || node_name.contains("spectrum")) {
      return;
    }

    // Exclude blocklisted node names

    if (std::ranges::find(pw::Manager::blocklist_node_name, node_name) != pw::Manager::blocklist_node_name.end()) {
      return;
    }

    uint64_t serial = 0U;

    if (!spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, serial)) {
      util::warning(
          "An error occurred while retrieving the object serial. The node cannot be handled by Easy Effects.");
      return;
    }

    // New node can be added into the node map

    auto* proxy = static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_NODE, sizeof(node_data)));

    auto* nd = static_cast<node_data*>(pw_proxy_get_user_data(proxy));

    nd->proxy = proxy;
    nd->pm = pm;

    nd->nd_info = new pw::NodeInfo();

    nd->nd_info->proxy = proxy;
    nd->nd_info->serial = serial;
    nd->nd_info->id = id;
    nd->nd_info->media_role = media_role;
    nd->nd_info->media_class = media_class;
    nd->nd_info->name = node_name;

    if (node_name == tags::pipewire::ee_source_name) {
      nd->nd_info->media_class = tags::pipewire::media_class::ee_source;
    } else if (node_name == tags::pipewire::ee_sink_name) {
      nd->nd_info->media_class = tags::pipewire::media_class::ee_sink;
    }

    spa_dict_get_string(props, PW_KEY_NODE_DESCRIPTION, nd->nd_info->description);

    spa_dict_get_num(props, PW_KEY_PRIORITY_SESSION, nd->nd_info->priority);

    spa_dict_get_num(props, PW_KEY_DEVICE_ID, nd->nd_info->device_id);

    const auto user_blocklist = (media_class == tags::pipewire::media_class::output_stream)
                                    ? db::StreamOutputs::blocklist()
                                    : db::StreamInputs::blocklist();

    nd->nd_info->is_blocklisted =
        std::ranges::find(user_blocklist, nd->nd_info->application_id) != user_blocklist.end();

    nd->nd_info->is_blocklisted =
        std::ranges::find(user_blocklist, nd->nd_info->app_process_binary) != user_blocklist.end();

    nd->nd_info->is_blocklisted =
        nd->nd_info->is_blocklisted || std::ranges::find(user_blocklist, nd->nd_info->name) != user_blocklist.end();

    pw_proxy_add_object_listener(proxy, &nd->object_listener, &node_events, nd);  // NOLINT
    pw_proxy_add_listener(proxy, &nd->proxy_listener, &node_proxy_events, nd);

    // We will have debug info about our filters later

    if (!is_ee_filter) {
      util::debug(std::format("{} {} {} with serial {} has been added", nd->nd_info->media_class.toStdString(), id,
                              nd->nd_info->name.toStdString(), serial));
    }

    return;
  }

  if (std::strcmp(type, PW_TYPE_INTERFACE_Link) == 0) {
    uint64_t serial = 0U;

    if (!spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, serial)) {
      util::warning(
          "An error occurred while retrieving the object serial. This link cannot be handled by Easy Effects.");
      return;
    }

    auto* proxy = static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_LINK, sizeof(proxy_data)));

    auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

    pd->proxy = proxy;
    pd->pm = pm;
    pd->id = id;
    pd->serial = serial;

    pw_proxy_add_object_listener(proxy, &pd->object_listener, &link_events, pd);  // NOLINT
    pw_proxy_add_listener(proxy, &pd->proxy_listener, &link_proxy_events, pd);

    auto link_info = link_info_from_props(props);

    link_info.id = id;
    link_info.serial = serial;

    pm->list_links.push_back(link_info);

    try {
      const auto input_node = pm->model_nodes.get_node_by_id(link_info.input_node_id);

      const auto output_node = pm->model_nodes.get_node_by_id(link_info.output_node_id);

      util::debug(std::format("{} port {} is connected to {} port {}", output_node.name.toStdString(),
                              link_info.output_port_id, input_node.name.toStdString(), link_info.input_port_id));
    } catch (std::out_of_range& e) {
      util::debug(e.what());
    }

    return;
  }

  if (std::strcmp(type, PW_TYPE_INTERFACE_Port) == 0) {
    uint64_t serial = 0U;

    if (!spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, serial)) {
      util::warning(
          "An error occurred while retrieving the object serial. This port cannot be handled by Easy Effects.");
      return;
    }

    auto* proxy = static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_PORT, sizeof(proxy_data)));

    auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

    pd->proxy = proxy;
    pd->pm = pm;
    pd->id = id;
    pd->serial = serial;

    pw_proxy_add_listener(proxy, &pd->proxy_listener, &port_proxy_events, pd);

    auto port_info = port_info_from_props(props);

    port_info.id = id;
    port_info.serial = serial;

    // std::cout << port_info.name << "\t" << port_info.audio_channel << "\t" << port_info.direction << "\t"
    //           << port_info.format_dsp << "\t" << port_info.port_id << "\t" << port_info.node_id << std::endl;

    pm->list_ports.push_back(port_info);

    return;
  }

  if (std::strcmp(type, PW_TYPE_INTERFACE_Module) == 0) {
    uint64_t serial = 0U;

    if (!spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, serial)) {
      util::warning(
          "An error occurred while retrieving the object serial. This module cannot be handled by Easy Effects.");
      return;
    }

    auto* proxy =
        static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_MODULE, sizeof(proxy_data)));

    auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

    pd->proxy = proxy;
    pd->pm = pm;
    pd->id = id;
    pd->serial = serial;

    pw_proxy_add_object_listener(proxy, &pd->object_listener, &module_events, pd);  // NOLINT
    pw_proxy_add_listener(proxy, &pd->proxy_listener, &module_proxy_events, pd);

    pw::ModuleInfo m_info{.id = id, .serial = serial, .name = "", .description = "", .filename = ""};

    spa_dict_get_string(props, PW_KEY_MODULE_NAME, m_info.name);

    pm->model_modules.append(m_info);

    return;
  }

  if (std::strcmp(type, PW_TYPE_INTERFACE_Client) == 0) {
    uint64_t serial = 0U;

    if (!spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, serial)) {
      util::warning(
          "An error occurred while retrieving the object serial. This client cannot be handled by Easy Effects.");
      return;
    }

    auto* proxy =
        static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_CLIENT, sizeof(proxy_data)));

    auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

    pd->proxy = proxy;
    pd->pm = pm;
    pd->id = id;
    pd->serial = serial;

    pw_proxy_add_object_listener(proxy, &pd->object_listener, &client_events, pd);  // NOLINT
    pw_proxy_add_listener(proxy, &pd->proxy_listener, &client_proxy_events, pd);

    pw::ClientInfo c_info{.id = id, .serial = serial, .name = "", .access = "", .api = ""};

    pm->model_clients.append(c_info);

    return;
  }

  if (std::strcmp(type, PW_TYPE_INTERFACE_Metadata) == 0) {
    if (const auto* name = spa_dict_lookup(props, PW_KEY_METADATA_NAME)) {
      util::debug(std::format("found metadata: {}", name));

      if (std::strcmp(name, "default") == 0) {
        if (pm->metadata != nullptr) {
          util::debug("A new default metadata is available. We will use it");

          spa_hook_remove(&pm->metadata_listener);
        }

        pm->metadata = static_cast<pw_metadata*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_METADATA, 0));

        if (pm->metadata != nullptr) {
          pw_metadata_add_listener(pm->metadata, &pm->metadata_listener, &metadata_events, pm);  // NOLINT
        } else {
          util::warning("pw_registry_bind returned a null metadata object");
        }
      }
    }

    return;
  }
}

void on_core_error(void* data, uint32_t id, [[maybe_unused]] int seq, int res, const char* message) {
  auto* const pm = static_cast<pw::Manager*>(data);

  if (id == PW_ID_CORE) {
    util::warning(std::format("Remote error res: {}", spa_strerror(res)));
    util::warning(std::format("Remote error message: {}", message));

    if (std::string(message) == "connection error") {
      util::fatal("No connection to PipeWire. Aborting!");
    }

    pw_thread_loop_signal(pm->thread_loop, false);
  }
}

void on_core_info(void* data, const struct pw_core_info* info) {
  auto* const pm = static_cast<pw::Manager*>(data);

  pm->runtimeVersion = info->version;

  spa_dict_get_string(info->props, "default.clock.rate", pm->defaultClockRate);

  spa_dict_get_string(info->props, "default.clock.min-quantum", pm->defaultMinQuantum);

  spa_dict_get_string(info->props, "default.clock.max-quantum", pm->defaultMaxQuantum);

  spa_dict_get_string(info->props, "default.clock.quantum", pm->defaultQuantum);

  util::warning(std::format("core version: {}", info->version));
  util::warning(std::format("core name: {}", info->name));
}

void on_core_done(void* data, uint32_t id, [[maybe_unused]] int seq) {
  auto* const pm = static_cast<pw::Manager*>(data);

  if (id == PW_ID_CORE) {
    pw_thread_loop_signal(pm->thread_loop, false);
  }
}

const struct pw_core_events core_events = {.version = PW_VERSION_CORE_EVENTS,
                                           .info = on_core_info,
                                           .done = on_core_done,
                                           .ping = nullptr,
                                           .error = on_core_error,
                                           .remove_id = nullptr,
                                           .bound_id = nullptr,
                                           .add_mem = nullptr,
                                           .remove_mem = nullptr,
                                           .bound_props = nullptr};

const struct pw_registry_events registry_events = {.version = 0,
                                                   .global = on_registry_global,
                                                   .global_remove = nullptr};

}  // namespace

namespace pw {

Manager::Manager() : headerVersion(pw_get_headers_version()), libraryVersion(pw_get_library_version()) {
  register_models();

  pw_init(nullptr, nullptr);

  spa_zero(core_listener);
  spa_zero(registry_listener);

  util::debug(std::format("compiled with PipeWire: {}", headerVersion.toStdString()));
  util::debug(std::format("linked to PipeWire: {}", libraryVersion.toStdString()));

  // this needs to occur after pw_init(), so putting it before pw_init() in the initializer breaks this
  // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
  thread_loop = pw_thread_loop_new("ee-pipewire-thread", nullptr);

  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  if (thread_loop == nullptr) {
    util::fatal("could not create PipeWire loop");
  }

  if (pw_thread_loop_start(thread_loop) != 0) {
    util::fatal("could not start the loop");
  }

  lock();

  pw_properties* props_context = pw_properties_new(nullptr, nullptr);

  const auto comparison_result = util::compare_versions(libraryVersion.toStdString(), "1.4.0");

  if (comparison_result == -1) {
    pw_properties_set(props_context, PW_KEY_CONFIG_NAME, "client-rt.conf");
  }

  pw_properties_set(props_context, PW_KEY_MEDIA_TYPE, "Audio");
  pw_properties_set(props_context, PW_KEY_MEDIA_CATEGORY, "Manager");
  pw_properties_set(props_context, PW_KEY_MEDIA_ROLE, "Music");

  context = pw_context_new(pw_thread_loop_get_loop(thread_loop), props_context, 0);

  if (context == nullptr) {
    util::fatal("could not create PipeWire context");
  }

  core = pw_context_connect(context, nullptr, 0);

  if (core == nullptr) {
    util::fatal("context connection failed");
  }

  registry = pw_core_get_registry(core, PW_VERSION_REGISTRY, 0);

  if (registry == nullptr) {
    util::fatal("could not get the registry");
  }

  pw_core_add_listener(core, &core_listener, &core_events, this);  // NOLINT

  pw_registry_add_listener(registry, &registry_listener, &registry_events, this);  // NOLINT

  if (ee_sink_node.id == SPA_ID_INVALID || ee_source_node.id == SPA_ID_INVALID) {
    load_virtual_devices();
  }

  sync_wait_unlock();

  do {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    ee_sink_node = model_nodes.get_node_by_name(tags::pipewire::ee_sink_name);
    ee_source_node = model_nodes.get_node_by_name(tags::pipewire::ee_source_name);
  } while (ee_sink_node.id == SPA_ID_INVALID || ee_source_node.id == SPA_ID_INVALID);

  if (ee_sink_node.id != SPA_ID_INVALID) {
    util::warning(std::format("{} node successfully retrieved with id {} and serial {}", tags::pipewire::ee_sink_name,
                              ee_sink_node.id, ee_sink_node.serial));
  }

  if (ee_source_node.id != SPA_ID_INVALID) {
    util::warning(std::format("{} node successfully retrieved with id {} and serial {}", tags::pipewire::ee_source_name,
                              ee_source_node.id, ee_source_node.serial));
  }

  /**
   * By the time our virtual devices are loaded we may have already received
   * some streams. So we connected them here now that our virtual devices are
   * available.
   */

  for (const auto& node : model_nodes.get_list()) {
    if (node.media_class == tags::pipewire::media_class::output_stream) {
      if (db::Main::processAllOutputs() && !node.is_blocklisted) {
        connectStreamOutput(node.id);
      }
    } else if (node.media_class == tags::pipewire::media_class::input_stream) {
      if (db::Main::processAllInputs() && !node.is_blocklisted) {
        connectStreamInput(node.id);
      }
    }
  }
}

Manager::~Manager() {
  exiting = true;

  lock();

  spa_hook_remove(&registry_listener);
  spa_hook_remove(&core_listener);
  spa_hook_remove(&metadata_listener);

  if (metadata != nullptr) {
    pw_proxy_destroy((struct pw_proxy*)metadata);
  }

  pw_proxy_destroy(proxy_stream_output_sink);
  pw_proxy_destroy(proxy_stream_input_source);

  util::debug("Destroying PipeWire registry...");
  pw_proxy_destroy((struct pw_proxy*)registry);

  util::debug("Disconnecting PipeWire core...");
  pw_core_disconnect(core);

  unlock();

  util::debug("Stopping PipeWire's loop...");
  pw_thread_loop_stop(thread_loop);

  util::debug("Destroying PipeWire's context...");
  pw_context_destroy(context);

  util::debug("Destroying PipeWire's loop...");
  pw_thread_loop_destroy(thread_loop);
}

void Manager::register_models() {
  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDelete)
  qmlRegisterSingletonInstance<pw::Manager>("ee.pipewire", VERSION_MAJOR, VERSION_MINOR, "Manager", this);

  qmlRegisterSingletonInstance<pw::models::Nodes>("ee.pipewire", VERSION_MAJOR, VERSION_MINOR, "ModelNodes",
                                                  &model_nodes);

  qmlRegisterSingletonInstance<pw::models::Modules>("ee.pipewire", VERSION_MAJOR, VERSION_MINOR, "ModelModules",
                                                    &model_modules);

  qmlRegisterSingletonInstance<pw::models::Clients>("ee.pipewire", VERSION_MAJOR, VERSION_MINOR, "ModelClients",
                                                    &model_clients);
  // NOLINTEND(clang-analyzer-cplusplus.NewDelete)
}

void Manager::load_virtual_devices() {
  // loading Easy Effects sink

  pw_properties* props_sink = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_sink, PW_KEY_APP_ID, tags::app::id);
  pw_properties_set(props_sink, PW_KEY_NODE_NAME, tags::pipewire::ee_sink_name);
  pw_properties_set(props_sink, PW_KEY_NODE_DESCRIPTION, "Easy Effects Sink");
  pw_properties_set(props_sink, PW_KEY_NODE_VIRTUAL, "true");
  pw_properties_set(props_sink, "factory.name", "support.null-audio-sink");
  pw_properties_set(props_sink, PW_KEY_MEDIA_CLASS, tags::pipewire::media_class::sink);
  pw_properties_set(props_sink, "audio.position", "FL,FR");
  pw_properties_set(props_sink, "monitor.channel-volumes", "false");
  pw_properties_set(props_sink, "monitor.passthrough", "true");
  pw_properties_set(props_sink, "priority.session", "0");

  proxy_stream_output_sink = static_cast<pw_proxy*>(
      pw_core_create_object(core, "adapter", PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, &props_sink->dict, 0));

  pw_properties_free(props_sink);

  // loading our source

  pw_properties* props_source = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_source, PW_KEY_APP_ID, tags::app::id);
  pw_properties_set(props_source, PW_KEY_NODE_NAME, tags::pipewire::ee_source_name);
  pw_properties_set(props_source, PW_KEY_NODE_DESCRIPTION, "Easy Effects Source");
  pw_properties_set(props_source, PW_KEY_NODE_VIRTUAL, "true");
  pw_properties_set(props_source, "factory.name", "support.null-audio-sink");
  pw_properties_set(props_source, PW_KEY_MEDIA_CLASS, tags::pipewire::media_class::virtual_source);
  pw_properties_set(props_source, "audio.position", "FL,FR");
  pw_properties_set(props_source, "monitor.channel-volumes", "false");
  pw_properties_set(props_source, "monitor.passthrough", "true");
  pw_properties_set(props_source, "priority.session", "0");

  proxy_stream_input_source = static_cast<pw_proxy*>(
      pw_core_create_object(core, "adapter", PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, &props_source->dict, 0));

  pw_properties_free(props_source);
}

auto Manager::stream_is_connected(const uint& id, const QString& media_class) -> bool {
  if (media_class == tags::pipewire::media_class::output_stream) {
    for (const auto& link : list_links) {
      if (link.output_node_id == id && link.input_node_id == ee_sink_node.id) {
        return true;
      }
    }
  } else if (media_class == tags::pipewire::media_class::input_stream) {
    for (const auto& link : list_links) {
      if (link.output_node_id == ee_source_node.id && link.input_node_id == id) {
        return true;
      }
    }
  }

  return false;
}

void Manager::connectStreamOutput(const uint& id) const {
  set_metadata_target_node(id, ee_sink_node.id, ee_sink_node.serial);
}

void Manager::connectStreamInput(const uint& id) const {
  set_metadata_target_node(id, ee_source_node.id, ee_source_node.serial);
}

void Manager::set_metadata_target_node(const uint& origin_id,
                                       const uint& target_id,
                                       const uint64_t& target_serial) const {
  if (metadata == nullptr) {
    return;
  }

  lock();

  // target.node for backward compatibility with old PW session managers
  // NOLINTNEXTLINE
  pw_metadata_set_property(metadata, origin_id, "target.node", "Spa:Id", util::to_string(target_id).c_str());
  // NOLINTNEXTLINE
  pw_metadata_set_property(metadata, origin_id, "target.object", "Spa:Id", util::to_string(target_serial).c_str());

  sync_wait_unlock();
}

void Manager::disconnectStream(const uint& stream_id) const {
  if (metadata == nullptr) {
    return;
  }

  lock();

  // target.node for backward compatibility with old PW session managers
  pw_metadata_set_property(metadata, stream_id, "target.node", nullptr, nullptr);    // NOLINT
  pw_metadata_set_property(metadata, stream_id, "target.object", nullptr, nullptr);  // NOLINT

  sync_wait_unlock();
}

void Manager::setNodeVolume(const uint& serial, const uint& n_vol_ch, const float& value) {
  if (auto* proxy = model_nodes.get_proxy_by_serial(serial); proxy != nullptr) {
    std::array<float, SPA_AUDIO_MAX_CHANNELS> volumes{};

    std::ranges::fill(volumes, 0.0F);
    std::fill_n(volumes.begin(), n_vol_ch, value);

    std::array<char, 1024U> buffer{};

    auto builder = SPA_POD_BUILDER_INIT(buffer.data(), sizeof(buffer));  // NOLINT

    lock();

    // NOLINTNEXTLINE
    pw_node_set_param(
        (struct pw_node*)proxy, SPA_PARAM_Props, 0,
        (spa_pod*)spa_pod_builder_add_object(&builder, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props, SPA_PROP_channelVolumes,
                                             SPA_POD_Array(sizeof(float), SPA_TYPE_Float, n_vol_ch, volumes.data())));

    sync_wait_unlock();
  }
}

void Manager::setNodeMute(const uint& serial, const bool& state) {
  if (auto* proxy = model_nodes.get_proxy_by_serial(serial); proxy != nullptr) {
    std::array<char, 1024U> buffer{};

    auto builder = SPA_POD_BUILDER_INIT(buffer.data(), sizeof(buffer));  // NOLINT

    lock();

    // NOLINTNEXTLINE
    pw_node_set_param((pw_node*)proxy, SPA_PARAM_Props, 0,
                      (spa_pod*)spa_pod_builder_add_object(&builder, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props,
                                                           SPA_PROP_mute, SPA_POD_Bool(state)));

    sync_wait_unlock();
  }
}

auto Manager::count_node_ports(const uint& node_id) -> uint {
  uint count = 0U;

  for (const auto& port : list_ports) {
    if (port.node_id == node_id) {
      count++;
    }
  }

  return count;
}

auto Manager::link_nodes(const uint& output_node_id,
                         const uint& input_node_id,
                         const bool& probe_link,
                         const bool& link_passive) -> std::vector<pw_proxy*> {
  std::vector<pw_proxy*> list;
  std::vector<PortInfo> list_output_ports;
  std::vector<PortInfo> list_input_ports;
  auto use_audio_channel = true;

  for (const auto& port : list_ports) {
    if (port.node_id == output_node_id && port.direction == "out") {
      list_output_ports.push_back(port);

      if (!probe_link) {
        if (port.audio_channel != "FL" && port.audio_channel != "FR") {
          use_audio_channel = false;
        }
      }
    }

    if (port.node_id == input_node_id && port.direction == "in") {
      if (!probe_link) {
        list_input_ports.push_back(port);

        if (port.audio_channel != "FL" && port.audio_channel != "FR") {
          use_audio_channel = false;
        }
      } else {
        if (port.audio_channel == "PROBE_FL" || port.audio_channel == "PROBE_FR") {
          list_input_ports.push_back(port);
        }
      }
    }
  }

  if (list_input_ports.empty()) {
    util::debug(std::format("node {} has no input ports yet. Aborting the link", input_node_id));

    return list;
  }

  if (list_output_ports.empty()) {
    util::debug(std::format("node {} has no output ports yet. Aborting the link", output_node_id));

    return list;
  }

  for (const auto& outp : list_output_ports) {
    for (const auto& inp : list_input_ports) {
      bool ports_match = false;

      if (!probe_link) {
        if (use_audio_channel) {
          ports_match = outp.audio_channel == inp.audio_channel;
        } else {
          ports_match = outp.port_id == inp.port_id;
        }
      } else {
        if (outp.audio_channel == "FL" && inp.audio_channel == "PROBE_FL") {
          ports_match = true;
        }

        if (outp.audio_channel == "FR" && inp.audio_channel == "PROBE_FR") {
          ports_match = true;
        }
      }

      if (ports_match) {
        pw_properties* props = pw_properties_new(nullptr, nullptr);

        pw_properties_set(props, PW_KEY_LINK_PASSIVE, (link_passive) ? "true" : "false");
        pw_properties_set(props, PW_KEY_OBJECT_LINGER, "false");
        pw_properties_set(props, PW_KEY_LINK_OUTPUT_NODE, util::to_string(output_node_id).c_str());
        pw_properties_set(props, PW_KEY_LINK_OUTPUT_PORT, util::to_string(outp.id).c_str());
        pw_properties_set(props, PW_KEY_LINK_INPUT_NODE, util::to_string(input_node_id).c_str());
        pw_properties_set(props, PW_KEY_LINK_INPUT_PORT, util::to_string(inp.id).c_str());

        lock();

        auto* proxy = static_cast<pw_proxy*>(
            pw_core_create_object(core, "link-factory", PW_TYPE_INTERFACE_Link, PW_VERSION_LINK, &props->dict, 0));

        pw_properties_free(props);

        if (proxy == nullptr) {
          util::warning(std::format("failed to link the node {} to {}", output_node_id, input_node_id));

          unlock();

          return list;
        }

        sync_wait_unlock();

        list.push_back(proxy);
      }
    }
  }

  return list;
}

void Manager::lock() const {
  pw_thread_loop_lock(thread_loop);
}

void Manager::unlock() const {
  pw_thread_loop_unlock(thread_loop);
}

void Manager::sync_wait_unlock() const {
  pw_core_sync(core, PW_ID_CORE, 0);  // NOLINT

  pw_thread_loop_wait(thread_loop);

  pw_thread_loop_unlock(thread_loop);
}

auto Manager::wait_full() const -> int {
  timespec abstime;

  pw_thread_loop_get_time(thread_loop, &abstime, 30 * SPA_NSEC_PER_SEC);

  return pw_thread_loop_timed_wait_full(thread_loop, &abstime);
}

void Manager::destroy_object(const int& id) const {
  lock();

  pw_registry_destroy(registry, id);  // NOLINT

  sync_wait_unlock();
}

void Manager::destroy_links(const std::vector<pw_proxy*>& list) const {
  for (auto* proxy : list) {
    if (proxy != nullptr) {
      lock();

      pw_proxy_destroy(proxy);

      sync_wait_unlock();
    }
  }
}

}  // namespace pw
