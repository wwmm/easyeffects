/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "pipe_manager.hpp"

namespace {

struct node_data {
  pw_proxy* proxy = nullptr;

  spa_hook proxy_listener{};

  spa_hook object_listener{};

  PipeManager* pm = nullptr;

  NodeInfo nd_info;
};

struct proxy_data {
  pw_proxy* proxy = nullptr;

  spa_hook proxy_listener{};

  spa_hook object_listener{};

  PipeManager* pm = nullptr;

  uint id = 0U;
};

void on_removed_proxy(void* data) {
  auto* const pd = static_cast<proxy_data*>(data);

  if (pd->object_listener.link.next != nullptr || pd->object_listener.link.prev != nullptr) {
    spa_hook_remove(&pd->object_listener);
  }

  pw_proxy_destroy(pd->proxy);
}

auto link_info_from_props(const spa_dict* props) -> LinkInfo {
  LinkInfo info;

  if (const auto* id = spa_dict_lookup(props, PW_KEY_LINK_ID)) {
    info.id = std::stoi(id);
  }

  if (const auto* path = spa_dict_lookup(props, PW_KEY_OBJECT_PATH)) {
    info.path = path;
  }

  if (const auto* input_node_id = spa_dict_lookup(props, PW_KEY_LINK_INPUT_NODE)) {
    info.input_node_id = std::stoi(input_node_id);
  }

  if (const auto* input_port_id = spa_dict_lookup(props, PW_KEY_LINK_INPUT_PORT)) {
    info.input_port_id = std::stoi(input_port_id);
  }

  if (const auto* output_node_id = spa_dict_lookup(props, PW_KEY_LINK_OUTPUT_NODE)) {
    info.output_node_id = std::stoi(output_node_id);
  }

  if (const auto* output_port_id = spa_dict_lookup(props, PW_KEY_LINK_OUTPUT_PORT)) {
    info.output_port_id = std::stoi(output_port_id);
  }

  if (const auto* passive = spa_dict_lookup(props, PW_KEY_LINK_PASSIVE)) {
    if (g_strcmp0(passive, "true") == 0) {
      info.passive = true;
    }
  }

  return info;
}

auto port_info_from_props(const spa_dict* props) -> PortInfo {
  PortInfo info;

  if (const auto* port_id = spa_dict_lookup(props, PW_KEY_PORT_ID)) {
    info.port_id = std::stoi(port_id);
  }

  if (const auto* name = spa_dict_lookup(props, PW_KEY_PORT_NAME)) {
    info.name = name;
  }

  if (const auto* node_id = spa_dict_lookup(props, PW_KEY_NODE_ID)) {
    info.node_id = std::stoi(node_id);
  }

  if (const auto* direction = spa_dict_lookup(props, PW_KEY_PORT_DIRECTION)) {
    info.direction = direction;
  }

  if (const auto* port_channel = spa_dict_lookup(props, PW_KEY_AUDIO_CHANNEL)) {
    info.audio_channel = port_channel;
  }

  if (const auto* port_audio_format = spa_dict_lookup(props, PW_KEY_AUDIO_FORMAT)) {
    info.format_dsp = port_audio_format;
  }

  if (const auto* port_physical = spa_dict_lookup(props, PW_KEY_PORT_PHYSICAL)) {
    if (g_strcmp0(port_physical, "true") == 0) {
      info.physical = true;
    }
  }

  if (const auto* port_terminal = spa_dict_lookup(props, PW_KEY_PORT_TERMINAL)) {
    if (g_strcmp0(port_terminal, "true") == 0) {
      info.terminal = true;
    }
  }

  if (const auto* port_monitor = spa_dict_lookup(props, PW_KEY_PORT_MONITOR)) {
    if (g_strcmp0(port_monitor, "true") == 0) {
      info.monitor = true;
    }
  }

  return info;
}

void on_removed_node_proxy(void* data) {
  auto* const pd = static_cast<node_data*>(data);

  spa_hook_remove(&pd->object_listener);

  pw_proxy_destroy(pd->proxy);
}

void on_destroy_node_proxy(void* data) {
  auto* const pd = static_cast<node_data*>(data);

  auto* const pm = pd->pm;

  spa_hook_remove(&pd->proxy_listener);

  pd->nd_info.proxy = nullptr;

  pm->node_map.at(pd->nd_info.timestamp).proxy = nullptr;

  pm->node_map.erase(pd->nd_info.timestamp);

  if (pd->nd_info.media_class == pm->media_class_source) {
    const auto nd_info_copy = pd->nd_info;

    Glib::signal_idle().connect_once([pm, nd_info_copy] { pm->source_removed.emit(nd_info_copy); });
  } else if (pd->nd_info.media_class == pm->media_class_sink) {
    const auto nd_info_copy = pd->nd_info;

    Glib::signal_idle().connect_once([pm, nd_info_copy] { pm->sink_removed.emit(nd_info_copy); });
  } else if (pd->nd_info.media_class == pm->media_class_output_stream) {
    const auto node_ts = pd->nd_info.timestamp;

    Glib::signal_idle().connect_once([pm, node_ts] { pm->stream_output_removed.emit(node_ts); });
  } else if (pd->nd_info.media_class == pm->media_class_input_stream) {
    const auto node_ts = pd->nd_info.timestamp;

    Glib::signal_idle().connect_once([pm, node_ts] { pm->stream_input_removed.emit(node_ts); });
  }

  util::debug(PipeManager::log_tag + pd->nd_info.media_class + " " + pd->nd_info.name + " was removed");
}

void on_node_info(void* object, const struct pw_node_info* info) {
  auto* const nd = static_cast<node_data*>(object);

  auto* const pm = nd->pm;

  if (pm->node_map.contains(nd->nd_info.timestamp)) {
    if (g_strcmp0(spa_dict_lookup(info->props, PW_KEY_STREAM_MONITOR), "true") == 0) {
      /*
        This is a workaround for issue #1128.
        Sometimes monitor streams like Pavucontrol can't be blocklisted inside on_registry_global
        because PipeWire sets localized app name in PW_KEY_NODE_NAME or PW_KEY_STREAM_MONITOR is
        empty and set afterwards.
        Therefore we check here the PW_KEY_STREAM_MONITOR of already added nodes inside the map
        and remove them accordingly.
      */

      spa_hook_remove(&nd->proxy_listener);

      nd->nd_info.proxy = nullptr;

      pm->node_map.at(nd->nd_info.timestamp).proxy = nullptr;

      pm->node_map.erase(nd->nd_info.timestamp);

      if (nd->nd_info.media_class == pm->media_class_source) {
        const auto nd_info_copy = nd->nd_info;

        Glib::signal_idle().connect_once([pm, nd_info_copy] { pm->source_removed.emit(nd_info_copy); });
      } else if (nd->nd_info.media_class == pm->media_class_sink) {
        const auto nd_info_copy = nd->nd_info;

        Glib::signal_idle().connect_once([pm, nd_info_copy] { pm->sink_removed.emit(nd_info_copy); });
      } else if (nd->nd_info.media_class == pm->media_class_output_stream) {
        const auto node_ts = nd->nd_info.timestamp;

        Glib::signal_idle().connect_once([pm, node_ts] { pm->stream_output_removed.emit(node_ts); });
      } else if (nd->nd_info.media_class == pm->media_class_input_stream) {
        const auto node_ts = nd->nd_info.timestamp;

        Glib::signal_idle().connect_once([pm, node_ts] { pm->stream_input_removed.emit(node_ts); });
      }

      util::debug(PipeManager::log_tag + " monitor stream " + nd->nd_info.media_class + " " + nd->nd_info.name +
                  " was removed");

      return;
    }

    auto app_info_ui_changed = false;

    if (info->state != nd->nd_info.state) {
      nd->nd_info.state = info->state;

      app_info_ui_changed = true;
    }

    nd->nd_info.n_input_ports = static_cast<int>(info->n_input_ports);
    nd->nd_info.n_output_ports = static_cast<int>(info->n_output_ports);

    if (const auto* prio_session = spa_dict_lookup(info->props, PW_KEY_PRIORITY_SESSION)) {
      nd->nd_info.priority = std::stoi(prio_session);
    }

    if (const auto* app_icon_name = spa_dict_lookup(info->props, PW_KEY_APP_ICON_NAME)) {
      if (app_icon_name != nd->nd_info.app_icon_name) {
        nd->nd_info.app_icon_name = app_icon_name;

        app_info_ui_changed = true;
      }
    }

    if (const auto* media_icon_name = spa_dict_lookup(info->props, PW_KEY_MEDIA_ICON_NAME)) {
      if (media_icon_name == nd->nd_info.media_icon_name) {
        nd->nd_info.media_icon_name = media_icon_name;

        app_info_ui_changed = true;
      }
    }

    if (const auto* device_icon_name = spa_dict_lookup(info->props, PW_KEY_DEVICE_ICON_NAME)) {
      nd->nd_info.device_icon_name = device_icon_name;
    }

    if (const auto* media_name = spa_dict_lookup(info->props, PW_KEY_MEDIA_NAME)) {
      if (media_name == nd->nd_info.media_name) {
        nd->nd_info.media_name = media_name;

        app_info_ui_changed = true;
      }
    }

    if (const auto* node_latency = spa_dict_lookup(info->props, PW_KEY_NODE_LATENCY)) {
      const auto& str = std::string(node_latency);

      const auto& delimiter_pos = str.find('/');

      const auto& latency_str = str.substr(0, delimiter_pos);

      const auto& rate_str = str.substr(delimiter_pos + 1);

      if (auto rate = std::stoi(rate_str); rate != nd->nd_info.rate) {
        nd->nd_info.rate = rate;

        app_info_ui_changed = true;
      }

      if (auto latency = (std::stof(latency_str) / static_cast<float>(nd->nd_info.rate));
          latency != nd->nd_info.latency) {
        nd->nd_info.latency = latency;

        app_info_ui_changed = true;
      }
    }

    if (const auto* device_id = spa_dict_lookup(info->props, PW_KEY_DEVICE_ID)) {
      nd->nd_info.device_id = std::stoi(device_id);
    }

    if ((info->change_mask & PW_NODE_CHANGE_MASK_PARAMS) != 0U) {
      for (uint i = 0U; i < info->n_params; i++) {
        if ((info->params[i].flags & SPA_PARAM_INFO_READ) == 0U) {
          continue;
        }

        if (const auto& id = info->params[i].id;
            id == SPA_PARAM_Props || id == SPA_PARAM_EnumFormat || id == SPA_PARAM_Format) {
          pw_node_enum_params((struct pw_node*)nd->proxy, 0, id, 0, -1, nullptr);
        }
      }
    }

    // update NodeInfo inside map

    pm->node_map.insert_or_assign(nd->nd_info.timestamp, nd->nd_info);

    // sometimes PipeWire destroys the pointer before signal_idle is called,
    // therefore we make a copy

    if (app_info_ui_changed) {
      const auto node_ts = nd->nd_info.timestamp;

      if (nd->nd_info.media_class == pm->media_class_output_stream) {
        Glib::signal_idle().connect_once([pm, node_ts] { pm->stream_output_changed.emit(node_ts); });
      } else if (nd->nd_info.media_class == pm->media_class_input_stream) {
        Glib::signal_idle().connect_once([pm, node_ts] { pm->stream_input_changed.emit(node_ts); });
      }
    } else if (nd->nd_info.media_class == pm->media_class_source) {
      const auto nd_info_copy = nd->nd_info;

      Glib::signal_idle().connect_once([pm, nd_info_copy] { pm->source_changed.emit(nd_info_copy); });
    } else if (nd->nd_info.media_class == pm->media_class_sink) {
      const auto nd_info_copy = nd->nd_info;

      Glib::signal_idle().connect_once([pm, nd_info_copy] { pm->sink_changed.emit(nd_info_copy); });
    }
  }

  // const struct spa_dict_item* item = nullptr;
  // spa_dict_for_each(item, info->props) printf("\t\t%s: \"%s\"\n", item->key, item->value);
}

void on_node_event_param(void* object,
                         int seq,
                         uint32_t id,
                         uint32_t index,
                         uint32_t next,
                         const struct spa_pod* param) {
  auto* const nd = static_cast<node_data*>(object);

  auto* const pm = nd->pm;

  if (param != nullptr) {
    spa_pod_prop* pod_prop = nullptr;
    auto* obj = (spa_pod_object*)param;

    auto notify = false;
    auto app_info_ui_changed = false;

    SPA_POD_OBJECT_FOREACH(obj, pod_prop) {
      switch (pod_prop->key) {
        case SPA_FORMAT_AUDIO_format: {
          uint format = 0U;

          if (spa_pod_get_id(&pod_prop->value, &format) == 0) {
            if (auto* node = pm->get_nodeptr(nd->nd_info.timestamp); node != nullptr) {
              std::string format_str;

              switch (format) {
                case SPA_AUDIO_FORMAT_S16_LE:
                  format_str = "S16LE";
                  break;
                case SPA_AUDIO_FORMAT_S24_LE:
                  format_str = "S24LE";
                  break;
                case SPA_AUDIO_FORMAT_S32_LE:
                  format_str = "S32LE";
                  break;
                case SPA_AUDIO_FORMAT_F32_LE:
                  format_str = "F32LE";
                  break;
                case SPA_AUDIO_FORMAT_F64_LE:
                  format_str = "F64LE";
                  break;
                case SPA_AUDIO_FORMAT_F32P:
                  format_str = "F32P";
                  break;
                default:
                  format_str = std::to_string(format);
                  // util::warning(format_str + " " + std::to_string(SPA_AUDIO_FORMAT_F32_LE));
                  break;
              }

              if (format_str != nd->nd_info.format) {
                node->format = format_str;

                nd->nd_info.format = format_str;

                app_info_ui_changed = true;

                notify = true;
              }
            }
          }

          break;
        }
        case SPA_FORMAT_AUDIO_rate: {
          int rate = 1;

          if (spa_pod_get_int(&pod_prop->value, &rate) == 0) {
            if (rate != nd->nd_info.rate) {
              if (auto* node = pm->get_nodeptr(nd->nd_info.timestamp); node != nullptr) {
                node->rate = rate;

                nd->nd_info.rate = rate;

                app_info_ui_changed = true;

                notify = true;
              }
            }
          }

          break;
        }
        case SPA_PROP_mute: {
          auto v = false;

          if (spa_pod_get_bool(&pod_prop->value, &v) == 0) {
            if (v != nd->nd_info.mute) {
              if (auto* node = pm->get_nodeptr(nd->nd_info.timestamp); node != nullptr) {
                node->mute = v;

                nd->nd_info.mute = v;

                app_info_ui_changed = true;

                notify = true;
              }
            }
          }

          break;
        }
        case SPA_PROP_channelVolumes: {
          if (auto* node = pm->get_nodeptr(nd->nd_info.timestamp); node != nullptr) {
            std::array<float, SPA_AUDIO_MAX_CHANNELS> volumes{};

            const auto& n_volumes =
                spa_pod_copy_array(&pod_prop->value, SPA_TYPE_Float, volumes.data(), SPA_AUDIO_MAX_CHANNELS);

            float max = 0.0F;

            for (uint i = 0U; i < n_volumes; max = std::max(volumes.at(i++), max))
              ;

            if (n_volumes != nd->nd_info.n_volume_channels || max != nd->nd_info.volume) {
              node->n_volume_channels = n_volumes;
              node->volume = max;

              nd->nd_info.n_volume_channels = n_volumes;
              nd->nd_info.volume = max;

              app_info_ui_changed = true;

              notify = true;
            }
          }

          break;
        }
        default:
          break;
      }
    }

    if (notify) {
      // sometimes PipeWire destroys the pointer before signal_idle is called,
      // therefore we make a copy

      if (app_info_ui_changed) {
        const auto node_ts = nd->nd_info.timestamp;

        if (nd->nd_info.media_class == pm->media_class_output_stream) {
          Glib::signal_idle().connect_once([pm, node_ts] { pm->stream_output_changed.emit(node_ts); });
        } else if (nd->nd_info.media_class == pm->media_class_input_stream) {
          Glib::signal_idle().connect_once([pm, node_ts] { pm->stream_input_changed.emit(node_ts); });
        }
      } else if (nd->nd_info.media_class == pm->media_class_virtual_source) {
        const auto nd_info_copy = nd->nd_info;

        if (nd_info_copy.id == pm->ee_source_node.id) {
          pm->ee_source_node = nd_info_copy;
        }

        Glib::signal_idle().connect_once([pm, nd_info_copy] { pm->source_changed.emit(nd_info_copy); });
      } else if (nd->nd_info.media_class == pm->media_class_sink) {
        const auto nd_info_copy = nd->nd_info;

        if (nd_info_copy.id == pm->ee_sink_node.id) {
          pm->ee_sink_node = nd_info_copy;
        }

        Glib::signal_idle().connect_once([pm, nd_info_copy] { pm->sink_changed.emit(nd_info_copy); });
      }
    }
  }
}

void on_link_info(void* object, const struct pw_link_info* info) {
  auto* const ld = static_cast<proxy_data*>(object);
  auto* const pm = ld->pm;

  LinkInfo link_copy;

  for (auto& l : ld->pm->list_links) {
    if (l.id == ld->id) {
      l.state = info->state;

      link_copy = l;

      Glib::signal_idle().connect_once([pm, link_copy] { pm->link_changed.emit(link_copy); });

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

  ld->pm->list_links.erase(std::remove_if(ld->pm->list_links.begin(), ld->pm->list_links.end(),
                                          [=](const auto& n) { return n.id == ld->id; }),
                           ld->pm->list_links.end());
}

void on_destroy_port_proxy(void* data) {
  auto* const ld = static_cast<proxy_data*>(data);

  spa_hook_remove(&ld->proxy_listener);

  ld->pm->list_ports.erase(std::remove_if(ld->pm->list_ports.begin(), ld->pm->list_ports.end(),
                                          [=](const auto& n) { return n.id == ld->id; }),
                           ld->pm->list_ports.end());
}

void on_module_info(void* object, const struct pw_module_info* info) {
  auto* const ld = static_cast<proxy_data*>(object);

  for (auto& module : ld->pm->list_modules) {
    if (module.id == info->id) {
      if (info->filename != nullptr) {
        module.filename = info->filename;
      }

      if (const auto* description = spa_dict_lookup(info->props, PW_KEY_MODULE_DESCRIPTION)) {
        module.description = description;
      }

      break;
    }
  }
}

void on_destroy_module_proxy(void* data) {
  auto* const md = static_cast<proxy_data*>(data);

  spa_hook_remove(&md->proxy_listener);

  md->pm->list_modules.erase(std::remove_if(md->pm->list_modules.begin(), md->pm->list_modules.end(),
                                            [=](const auto& n) { return n.id == md->id; }),
                             md->pm->list_modules.end());
}

void on_client_info(void* object, const struct pw_client_info* info) {
  auto* const ld = static_cast<proxy_data*>(object);

  for (auto& client : ld->pm->list_clients) {
    if (client.id == info->id) {
      if (const auto* name = spa_dict_lookup(info->props, PW_KEY_APP_NAME)) {
        client.name = name;
      }

      if (const auto* access = spa_dict_lookup(info->props, PW_KEY_ACCESS)) {
        client.access = access;
      }

      if (const auto* api = spa_dict_lookup(info->props, PW_KEY_CLIENT_API)) {
        client.api = api;
      }

      break;
    }
  }
}

void on_destroy_client_proxy(void* data) {
  auto* const pd = static_cast<proxy_data*>(data);

  spa_hook_remove(&pd->proxy_listener);

  pd->pm->list_clients.erase(std::remove_if(pd->pm->list_clients.begin(), pd->pm->list_clients.end(),
                                            [=](const auto& n) { return n.id == pd->id; }),
                             pd->pm->list_clients.end());
}

void on_device_info(void* object, const struct pw_device_info* info) {
  auto* const ld = static_cast<proxy_data*>(object);

  for (auto& device : ld->pm->list_devices) {
    if (device.id == info->id) {
      if (const auto* name = spa_dict_lookup(info->props, PW_KEY_DEVICE_NAME)) {
        device.name = name;
      }

      if (const auto* nick = spa_dict_lookup(info->props, PW_KEY_DEVICE_NAME)) {
        device.nick = nick;
      }

      if (const auto* description = spa_dict_lookup(info->props, PW_KEY_DEVICE_DESCRIPTION)) {
        device.description = description;
      }

      if (const auto* api = spa_dict_lookup(info->props, PW_KEY_DEVICE_API)) {
        device.api = api;
      }

      if ((info->change_mask & PW_DEVICE_CHANGE_MASK_PARAMS) != 0U) {
        for (uint i = 0U; i < info->n_params; i++) {
          if ((info->params[i].flags & SPA_PARAM_INFO_READ) == 0U) {
            continue;
          }

          if (const auto& id = info->params[i].id; id == SPA_PARAM_Route) {
            pw_device_enum_params((struct pw_device*)ld->proxy, 0, id, 0, -1, nullptr);
          }
        }
      }

      break;
    }
  }
}

void on_device_event_param(void* object,
                           int seq,
                           uint32_t id,
                           uint32_t index,
                           uint32_t next,
                           const struct spa_pod* param) {
  auto* const dd = static_cast<proxy_data*>(object);

  if (id == SPA_PARAM_Route) {
    const char* name = nullptr;
    enum spa_direction direction {};
    enum spa_param_availability available {};

    if (spa_pod_parse_object(param, SPA_TYPE_OBJECT_ParamRoute, nullptr, SPA_PARAM_ROUTE_direction,
                             SPA_POD_Id(&direction), SPA_PARAM_ROUTE_name, SPA_POD_String(&name),
                             SPA_PARAM_ROUTE_available, SPA_POD_Id(&available)) < 0) {
      return;
    }

    if (name != nullptr) {
      for (auto& device : dd->pm->list_devices) {
        if (device.id == dd->id) {
          auto* const pm = dd->pm;

          if (direction == SPA_DIRECTION_INPUT) {
            if (name != device.input_route_name || available != device.input_route_available) {
              device.input_route_name = name;
              device.input_route_available = available;

              Glib::signal_idle().connect_once([pm, device] { pm->device_input_route_changed.emit(device); });
            }
          } else if (direction == SPA_DIRECTION_OUTPUT) {
            if (name != device.output_route_name || available != device.output_route_available) {
              device.output_route_name = name;
              device.output_route_available = available;

              Glib::signal_idle().connect_once([pm, device] { pm->device_output_route_changed.emit(device); });
            }
          }

          break;
        }
      }
    }
  }
}

void on_destroy_device_proxy(void* data) {
  auto* const pd = static_cast<proxy_data*>(data);

  spa_hook_remove(&pd->proxy_listener);

  pd->pm->list_devices.erase(std::remove_if(pd->pm->list_devices.begin(), pd->pm->list_devices.end(),
                                            [=](const auto& n) { return n.id == pd->id; }),
                             pd->pm->list_devices.end());
}

auto on_metadata_property(void* data, uint32_t id, const char* key, const char* type, const char* value) -> int {
  auto* const pm = static_cast<PipeManager*>(data);

  const std::string str_key = (key != nullptr) ? key : std::string();
  const std::string str_type = (type != nullptr) ? type : std::string();
  const std::string str_value = (value != nullptr) ? value : std::string();

  util::debug(PipeManager::log_tag + "new metadata property: " + std::to_string(id) + ", " + str_key + ", " + str_type +
              ", " + str_value);

  if (str_value.empty()) {
    return 0;
  }

  if (str_key == "default.audio.sink") {
    std::array<char, 1024U> v{};

    PipeManager::json_object_find(str_value.c_str(), "name", v.data(), v.size() * sizeof(char));

    for (const auto& [ts, node] : pm->node_map) {
      if (node.name == v.data()) {
        if (node.name == pm->ee_sink_name) {
          pm->default_output_device.id = SPA_ID_INVALID;

          return 0;
        }

        if (node.media_class == pm->media_class_sink) {
          pm->default_output_device = node;

          Glib::signal_idle().connect_once([pm, node] { pm->new_default_sink.emit(node); });
        }

        break;
      }
    }
  }

  if (str_key == "default.audio.source") {
    std::array<char, 1024U> v{};

    PipeManager::json_object_find(str_value.c_str(), "name", v.data(), v.size() * sizeof(char));

    for (const auto& [ts, node] : pm->node_map) {
      if (node.name == v.data()) {
        if (node.name == pm->ee_source_name) {
          pm->default_input_device.id = SPA_ID_INVALID;

          return 0;
        }

        if (node.media_class == pm->media_class_source || node.media_class == pm->media_class_virtual_source) {
          pm->default_input_device = node;

          Glib::signal_idle().connect_once([pm, node] { pm->new_default_source.emit(node); });
        }

        break;
      }
    }
  }

  return 0;
}

const struct pw_metadata_events metadata_events = {PW_VERSION_METADATA_EVENTS, on_metadata_property};

const struct pw_proxy_events link_proxy_events = {.destroy = on_destroy_link_proxy,
                                                  .bound = nullptr,
                                                  .removed = on_removed_proxy,
                                                  .done = nullptr,
                                                  .error = nullptr};

const struct pw_proxy_events port_proxy_events = {.destroy = on_destroy_port_proxy,
                                                  .bound = nullptr,
                                                  .removed = on_removed_proxy,
                                                  .done = nullptr,
                                                  .error = nullptr};

const struct pw_proxy_events module_proxy_events = {.destroy = on_destroy_module_proxy,
                                                    .bound = nullptr,
                                                    .removed = on_removed_proxy,
                                                    .done = nullptr,
                                                    .error = nullptr};

const struct pw_proxy_events client_proxy_events = {.destroy = on_destroy_client_proxy,
                                                    .bound = nullptr,
                                                    .removed = on_removed_proxy,
                                                    .done = nullptr,
                                                    .error = nullptr};

const struct pw_proxy_events device_proxy_events = {.destroy = on_destroy_device_proxy,
                                                    .bound = nullptr,
                                                    .removed = on_removed_proxy,
                                                    .done = nullptr,
                                                    .error = nullptr};

const struct pw_proxy_events node_proxy_events = {.destroy = on_destroy_node_proxy,
                                                  .bound = nullptr,
                                                  .removed = on_removed_node_proxy,
                                                  .done = nullptr,
                                                  .error = nullptr};

const struct pw_node_events node_events = {.info = on_node_info, .param = on_node_event_param};

const struct pw_link_events link_events = {
    .info = on_link_info,
};

const struct pw_module_events module_events = {
    .info = on_module_info,
};

const struct pw_client_events client_events = {
    .info = on_client_info,
};

const struct pw_device_events device_events = {.info = on_device_info, .param = on_device_event_param};

void on_registry_global(void* data,
                        uint32_t id,
                        uint32_t permissions,
                        const char* type,
                        uint32_t version,
                        const struct spa_dict* props) {
  if (id == SPA_ID_INVALID) {
    // If PipeWire send us a wrong id, we don't have issues
    return;
  }

  auto* const pm = static_cast<PipeManager*>(data);

  if (g_strcmp0(type, PW_TYPE_INTERFACE_Node) == 0) {
    if (const auto* key_media_role = spa_dict_lookup(props, PW_KEY_MEDIA_ROLE)) {
      if (std::ranges::find(pm->blocklist_media_role, std::string(key_media_role)) != pm->blocklist_media_role.end()) {
        return;
      }

      if (g_strcmp0(key_media_role, "DSP") == 0) {
        const auto* key_media_category = spa_dict_lookup(props, PW_KEY_MEDIA_CATEGORY);

        if (g_strcmp0(key_media_category, "Filter") == 0) {
          const auto* key_node_description = spa_dict_lookup(props, PW_KEY_NODE_DESCRIPTION);

          if (g_strcmp0(key_node_description, "easyeffects_filter") == 0) {
            const auto* node_name = spa_dict_lookup(props, PW_KEY_NODE_NAME);

            util::debug(PipeManager::log_tag + "Filter " + node_name + ", id = " + std::to_string(id) + ", was added");
          }
        }
      }
    }

    if (const auto* key_media_class = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS)) {
      const std::string media_class = key_media_class;

      static const auto class_array = {pm->media_class_output_stream, pm->media_class_input_stream,
                                       pm->media_class_sink, pm->media_class_source, pm->media_class_virtual_source};

      if (std::any_of(class_array.begin(), class_array.end(), [&](const auto& str) { return str == media_class; })) {
        const auto* node_name = spa_dict_lookup(props, PW_KEY_NODE_NAME);

        if (node_name == nullptr) {
          return;
        }

        const std::string name = node_name;

        if (name.empty()) {
          return;
        } else if (std::ranges::find(pm->blocklist_node_name, name) != pm->blocklist_node_name.end()) {
          return;
        }

        // New node can be added in the node map

        std::string ts = std::to_string(
            std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count());

        auto* proxy =
            static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_NODE, sizeof(node_data)));

        auto* const pd = static_cast<node_data*>(pw_proxy_get_user_data(proxy));

        pd->proxy = proxy;
        pd->pm = pm;
        pd->nd_info.timestamp = ts;
        pd->nd_info.proxy = proxy;
        pd->nd_info.id = id;
        pd->nd_info.media_class = media_class;
        pd->nd_info.name = name;

        if (const auto* node_description = spa_dict_lookup(props, PW_KEY_NODE_DESCRIPTION)) {
          pd->nd_info.description = node_description;
        }

        if (const auto* prio_session = spa_dict_lookup(props, PW_KEY_PRIORITY_SESSION)) {
          pd->nd_info.priority = std::stoi(prio_session);
        }

        if (const auto* device_id = spa_dict_lookup(props, PW_KEY_DEVICE_ID)) {
          pd->nd_info.device_id = std::stoi(device_id);
        }

        pw_node_add_listener(proxy, &pd->object_listener, &node_events, pd);
        pw_proxy_add_listener(proxy, &pd->proxy_listener, &node_proxy_events, pd);

        pm->node_map.insert_or_assign(ts, pd->nd_info);

        // sometimes PipeWire destroys the pointer before signal_idle is called,
        // therefore we make a copy of NodeInfo

        const auto nd_info_copy = pd->nd_info;

        if (media_class == pm->media_class_source && name != pm->ee_source_name) {
          Glib::signal_idle().connect_once([pm, nd_info_copy] { pm->source_added.emit(nd_info_copy); });
        } else if (media_class == pm->media_class_sink && name != pm->ee_sink_name) {
          Glib::signal_idle().connect_once([pm, nd_info_copy] { pm->sink_added.emit(nd_info_copy); });
        } else if (media_class == pm->media_class_output_stream) {
          Glib::signal_idle().connect_once([pm, nd_info_copy] { pm->stream_output_added.emit(nd_info_copy); });
        } else if (media_class == pm->media_class_input_stream) {
          Glib::signal_idle().connect_once([pm, nd_info_copy] { pm->stream_input_added.emit(nd_info_copy); });
        }

        util::debug(PipeManager::log_tag + media_class + " " + std::to_string(id) + " " + pd->nd_info.name +
                    " with timestamp " + pd->nd_info.timestamp + " was added");
      }
    }

    return;
  }

  if (g_strcmp0(type, PW_TYPE_INTERFACE_Link) == 0) {
    auto* proxy = static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_LINK, sizeof(proxy_data)));

    auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

    pd->proxy = proxy;
    pd->pm = pm;
    pd->id = id;

    pw_link_add_listener(proxy, &pd->object_listener, &link_events, pd);
    pw_proxy_add_listener(proxy, &pd->proxy_listener, &link_proxy_events, pd);

    auto link_info = link_info_from_props(props);

    link_info.id = id;

    pm->list_links.emplace_back(link_info);

    try {
      const auto& input_node = pm->get_node_by_id(link_info.input_node_id);

      const auto& output_node = pm->get_node_by_id(link_info.output_node_id);

      util::debug(PipeManager::log_tag + output_node.name + " port " + std::to_string(link_info.output_port_id) +
                  " is connected to " + input_node.name + " port " + std::to_string(link_info.input_port_id));
    } catch (...) {
    }

    return;
  }

  if (g_strcmp0(type, PW_TYPE_INTERFACE_Port) == 0) {
    auto* proxy = static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_PORT, sizeof(proxy_data)));

    auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

    pd->proxy = proxy;
    pd->pm = pm;
    pd->id = id;

    pw_proxy_add_listener(proxy, &pd->proxy_listener, &port_proxy_events, pd);

    auto port_info = port_info_from_props(props);

    port_info.id = id;

    // std::cout << port_info.name << "\t" << port_info.audio_channel << "\t" << port_info.direction << "\t"
    //           << port_info.format_dsp << "\t" << port_info.port_id << "\t" << port_info.node_id << std::endl;

    pm->list_ports.emplace_back(port_info);

    return;
  }

  if (g_strcmp0(type, PW_TYPE_INTERFACE_Module) == 0) {
    auto* proxy =
        static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_MODULE, sizeof(proxy_data)));

    auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

    pd->proxy = proxy;
    pd->pm = pm;
    pd->id = id;

    pw_module_add_listener(proxy, &pd->object_listener, &module_events, pd);
    pw_proxy_add_listener(proxy, &pd->proxy_listener, &module_proxy_events, pd);

    ModuleInfo m_info{.id = id};

    if (const auto* name = spa_dict_lookup(props, PW_KEY_MODULE_NAME)) {
      m_info.name = name;
    }

    pm->list_modules.emplace_back(m_info);

    return;
  }

  if (g_strcmp0(type, PW_TYPE_INTERFACE_Client) == 0) {
    auto* proxy =
        static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_CLIENT, sizeof(proxy_data)));

    auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

    pd->proxy = proxy;
    pd->pm = pm;
    pd->id = id;

    pw_client_add_listener(proxy, &pd->object_listener, &client_events, pd);
    pw_proxy_add_listener(proxy, &pd->proxy_listener, &client_proxy_events, pd);

    ClientInfo c_info{.id = id};

    pm->list_clients.emplace_back(c_info);

    return;
  }

  if (g_strcmp0(type, PW_TYPE_INTERFACE_Metadata) == 0) {
    if (const auto* name = spa_dict_lookup(props, PW_KEY_METADATA_NAME)) {
      util::debug(PipeManager::log_tag + "found metadata: " + name);

      if (pm->metadata != nullptr) {
        return;
      }

      if (g_strcmp0(name, "default") == 0) {
        pm->metadata = static_cast<pw_metadata*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_METADATA, 0));

        if (pm->metadata != nullptr) {
          pw_metadata_add_listener(pm->metadata, &pm->metadata_listener, &metadata_events, pm);
        } else {
          util::warning(PipeManager::log_tag + "pw_registry_bind returned a null metadata object");
        }
      }
    }

    return;
  }

  if (g_strcmp0(type, PW_TYPE_INTERFACE_Device) == 0) {
    if (const auto* key_media_class = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS)) {
      const std::string media_class = key_media_class;

      if (media_class == "Audio/Device") {
        auto* proxy =
            static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_DEVICE, sizeof(proxy_data)));

        auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

        pd->proxy = proxy;
        pd->pm = pm;
        pd->id = id;

        pw_device_add_listener(proxy, &pd->object_listener, &device_events, pd);
        pw_proxy_add_listener(proxy, &pd->proxy_listener, &device_proxy_events, pd);

        DeviceInfo d_info{.id = id, .media_class = media_class};

        pm->list_devices.emplace_back(d_info);
      }
    }

    return;
  }
}

void on_core_error(void* data, uint32_t id, int seq, int res, const char* message) {
  auto* const pm = static_cast<PipeManager*>(data);

  if (id == PW_ID_CORE) {
    util::warning(PipeManager::log_tag + "Remote error res: " + spa_strerror(res));
    util::warning(PipeManager::log_tag + "Remote error message: " + message);

    pw_thread_loop_signal(pm->thread_loop, false);
  }
}

void on_core_info(void* data, const struct pw_core_info* info) {
  auto* const pm = static_cast<PipeManager*>(data);

  pm->core_name = info->name;

  if (const auto* rate = spa_dict_lookup(info->props, "default.clock.rate")) {
    pm->default_clock_rate = rate;
  }

  if (const auto* min_quantum = spa_dict_lookup(info->props, "default.clock.min-quantum")) {
    pm->default_min_quantum = min_quantum;
  }

  if (const auto* max_quantum = spa_dict_lookup(info->props, "default.clock.max-quantum")) {
    pm->default_max_quantum = max_quantum;
  }

  if (const auto* quantum = spa_dict_lookup(info->props, "default.clock.quantum")) {
    pm->default_quantum = quantum;
  }

  util::debug(PipeManager::log_tag + "core version: " + info->version);
  util::debug(PipeManager::log_tag + "core name: " + info->name);
}

void on_core_done(void* data, uint32_t id, int seq) {
  auto* const pm = static_cast<PipeManager*>(data);

  if (id == PW_ID_CORE) {
    pw_thread_loop_signal(pm->thread_loop, false);
  }
}

const struct pw_core_events core_events = {.version = PW_VERSION_CORE_EVENTS,
                                           .info = on_core_info,
                                           .done = on_core_done,
                                           .error = on_core_error};

const struct pw_registry_events registry_events = {
    .global = on_registry_global,
};

}  // namespace

PipeManager::PipeManager() {
  pw_init(nullptr, nullptr);

  spa_zero(core_listener);
  spa_zero(registry_listener);

  header_version = pw_get_headers_version();
  library_version = pw_get_library_version();

  util::debug(log_tag + "compiled with pipewire: " + header_version);
  util::debug(log_tag + "linked to pipewire: " + library_version);

  thread_loop = pw_thread_loop_new("pe-pipewire-thread", nullptr);

  if (thread_loop == nullptr) {
    util::error(log_tag + "could not create pipewire loop");
  }

  if (pw_thread_loop_start(thread_loop) != 0) {
    util::error(log_tag + "could not start the loop");
  }

  lock();

  pw_properties* props_context = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_context, PW_KEY_MEDIA_TYPE, "Audio");
  pw_properties_set(props_context, PW_KEY_MEDIA_CATEGORY, "Manager");
  pw_properties_set(props_context, PW_KEY_MEDIA_ROLE, "Music");

  context = pw_context_new(pw_thread_loop_get_loop(thread_loop), props_context, 0);

  if (context == nullptr) {
    util::error(log_tag + "could not create pipewire context");
  }

  core = pw_context_connect(context, nullptr, 0);

  if (core == nullptr) {
    util::error(log_tag + "context connection failed");
  }

  registry = pw_core_get_registry(core, PW_VERSION_REGISTRY, 0);

  if (registry == nullptr) {
    util::error(log_tag + "could not get the registry");
  }

  pw_registry_add_listener(registry, &registry_listener, &registry_events, this);

  pw_core_add_listener(core, &core_listener, &core_events, this);

  // loading EasyEffects sink

  pw_properties* props_sink = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_sink, PW_KEY_NODE_NAME, ee_sink_name.c_str());
  pw_properties_set(props_sink, PW_KEY_NODE_DESCRIPTION, "EasyEffects Sink");
  pw_properties_set(props_sink, "factory.name", "support.null-audio-sink");
  pw_properties_set(props_sink, PW_KEY_MEDIA_CLASS, media_class_sink.c_str());
  pw_properties_set(props_sink, "audio.position", "FL,FR");
  pw_properties_set(props_sink, "monitor.channel-volumes", "true");

  proxy_stream_output_sink = static_cast<pw_proxy*>(
      pw_core_create_object(core, "adapter", PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, &props_sink->dict, 0));

  // loading our source

  pw_properties* props_source = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_source, PW_KEY_NODE_NAME, ee_source_name.c_str());
  pw_properties_set(props_source, PW_KEY_NODE_DESCRIPTION, "EasyEffects Source");
  pw_properties_set(props_source, "factory.name", "support.null-audio-sink");
  pw_properties_set(props_source, PW_KEY_MEDIA_CLASS, media_class_virtual_source.c_str());
  pw_properties_set(props_source, "audio.position", "FL,FR");
  pw_properties_set(props_source, "monitor.channel-volumes", "true");

  proxy_stream_input_source = static_cast<pw_proxy*>(
      pw_core_create_object(core, "adapter", PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, &props_source->dict, 0));

  sync_wait_unlock();

  do {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    for (const auto& [ts, node] : node_map) {
      if (ee_sink_node.name.empty() && node.name == ee_sink_name) {
        ee_sink_node = node;

        util::debug(log_tag + ee_sink_name + " node successfully retrieved with id " + std::to_string(node.id) +
                    " and timestamp " + node.timestamp);
      } else if (ee_source_node.name.empty() && node.name == ee_source_name) {
        ee_source_node = node;

        util::debug(log_tag + ee_source_name + " node successfully retrieved with id " + std::to_string(node.id) +
                    " and timestamp " + node.timestamp);
      }
    }
  } while (ee_sink_node.id == SPA_ID_INVALID || ee_source_node.id == SPA_ID_INVALID);
}

PipeManager::~PipeManager() {
  lock();

  spa_hook_remove(&registry_listener);
  spa_hook_remove(&core_listener);
  spa_hook_remove(&metadata_listener);

  if (metadata != nullptr) {
    pw_proxy_destroy((struct pw_proxy*)metadata);
  }

  pw_proxy_destroy(proxy_stream_output_sink);
  pw_proxy_destroy(proxy_stream_input_source);

  util::debug(log_tag + "Destroying Pipewire registry...");
  pw_proxy_destroy((struct pw_proxy*)registry);

  util::debug(log_tag + "Disconnecting Pipewire core...");
  pw_core_disconnect(core);

  util::debug(log_tag + "Destroying Pipewire context...");
  pw_context_destroy(context);

  unlock();

  util::debug(log_tag + "Destroying Pipewire loop...");
  pw_thread_loop_destroy(thread_loop);
}

auto PipeManager::get_node_by_id(const uint& id) -> NodeInfo& {
  for (auto& [ts, node] : node_map) {
    if (node.id == id) {
      return node;
    }
  }

  throw std::out_of_range("");
}

auto PipeManager::get_nodeptr(const std::string& ts) -> NodeInfo* {
  // we don't want always wrap a node getter in a try catch statement
  // so we get a pointer to check for nullptr

  try {
    auto& node = node_map.at(ts);

    return &node;
  } catch (...) {
    return nullptr;
  }
}

auto PipeManager::get_nodeptr_by_id(const uint& id) -> NodeInfo* {
  // we don't want always wrap a node getter in a try catch statement
  // so we get a pointer to check for nullptr

  for (auto& [ts, node] : node_map) {
    if (node.id == id) {
      return &node;
    }
  }

  return nullptr;
}

auto PipeManager::stream_is_connected(const uint& id, const std::string& media_class) -> bool {
  if (media_class == media_class_output_stream) {
    for (const auto& link : list_links) {
      if (link.output_node_id == id && link.input_node_id == ee_sink_node.id) {
        return true;
      }
    }
  } else if (media_class == media_class_input_stream) {
    for (const auto& link : list_links) {
      if (link.output_node_id == ee_source_node.id && link.input_node_id == id) {
        return true;
      }
    }
  }

  return false;
}

void PipeManager::connect_stream_output(const uint& id) const {
  set_metadata_target_node(id, ee_sink_node.id);
}

void PipeManager::disconnect_stream_output(const uint& id) const {
  set_metadata_target_node(id, default_output_device.id);
}

void PipeManager::connect_stream_input(const uint& id) const {
  set_metadata_target_node(id, ee_source_node.id);
}

void PipeManager::disconnect_stream_input(const uint& id) const {
  set_metadata_target_node(id, default_input_device.id);
}

void PipeManager::set_metadata_target_node(const uint& origin_id, const uint& target_id) const {
  lock();

  pw_metadata_set_property(metadata, origin_id, "target.node", "Spa:Id", std::to_string(target_id).c_str());

  sync_wait_unlock();
}

void PipeManager::set_node_volume(pw_proxy* proxy, const uint& n_vol_ch, const float& value) {
  std::array<float, SPA_AUDIO_MAX_CHANNELS> volumes{};

  std::ranges::fill(volumes, 0.0F);
  std::fill_n(volumes.begin(), n_vol_ch, value);

  std::array<char, 1024U> buffer{};

  auto builder = SPA_POD_BUILDER_INIT(buffer.data(), sizeof(buffer));

  pw_node_set_param(
      (struct pw_node*)proxy, SPA_PARAM_Props, 0,
      (spa_pod*)spa_pod_builder_add_object(&builder, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props, SPA_PROP_channelVolumes,
                                           SPA_POD_Array(sizeof(float), SPA_TYPE_Float, n_vol_ch, volumes.data())));
}

void PipeManager::set_node_mute(pw_proxy* proxy, const bool& state) {
  std::array<char, 1024U> buffer{};

  auto builder = SPA_POD_BUILDER_INIT(buffer.data(), sizeof(buffer));

  pw_node_set_param((pw_node*)proxy, SPA_PARAM_Props, 0,
                    (spa_pod*)spa_pod_builder_add_object(&builder, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props,
                                                         SPA_PROP_mute, SPA_POD_Bool(state)));
}

auto PipeManager::link_nodes(const uint& output_node_id,
                             const uint& input_node_id,
                             const bool& probe_link,
                             const bool& link_passive) -> std::vector<pw_proxy*> {
  std::vector<pw_proxy*> list;
  std::vector<PortInfo> list_output_ports;
  std::vector<PortInfo> list_input_ports;
  auto use_audio_channel = true;

  for (const auto& port : list_ports) {
    if (port.node_id == output_node_id && port.direction == "out") {
      list_output_ports.emplace_back(port);

      if (!probe_link) {
        if (port.audio_channel != "FL" && port.audio_channel != "FR") {
          use_audio_channel = false;
        }
      }
    }

    if (port.node_id == input_node_id && port.direction == "in") {
      if (!probe_link) {
        list_input_ports.emplace_back(port);

        if (port.audio_channel != "FL" && port.audio_channel != "FR") {
          use_audio_channel = false;
        }
      } else {
        if (port.audio_channel == "PROBE_FL" || port.audio_channel == "PROBE_FR") {
          list_input_ports.emplace_back(port);
        }
      }
    }
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
        pw_properties_set(props, PW_KEY_LINK_OUTPUT_NODE, std::to_string(output_node_id).c_str());
        pw_properties_set(props, PW_KEY_LINK_OUTPUT_PORT, std::to_string(outp.id).c_str());
        pw_properties_set(props, PW_KEY_LINK_INPUT_NODE, std::to_string(input_node_id).c_str());
        pw_properties_set(props, PW_KEY_LINK_INPUT_PORT, std::to_string(inp.id).c_str());

        lock();

        auto* proxy = static_cast<pw_proxy*>(
            pw_core_create_object(core, "link-factory", PW_TYPE_INTERFACE_Link, PW_VERSION_LINK, &props->dict, 0));

        if (proxy == nullptr) {
          util::warning(log_tag + "failed to link the node " + std::to_string(output_node_id) + " to " +
                        std::to_string(input_node_id));

          unlock();

          return list;
        }

        sync_wait_unlock();

        list.emplace_back(proxy);
      }
    }
  }

  return list;
}

void PipeManager::lock() const {
  pw_thread_loop_lock(thread_loop);
}

void PipeManager::unlock() const {
  pw_thread_loop_unlock(thread_loop);
}

void PipeManager::sync_wait_unlock() const {
  pw_core_sync(core, PW_ID_CORE, 0);

  pw_thread_loop_wait(thread_loop);

  pw_thread_loop_unlock(thread_loop);
}

void PipeManager::destroy_object(const int& id) const {
  lock();

  pw_registry_destroy(registry, id);

  sync_wait_unlock();
}

void PipeManager::destroy_links(const std::vector<pw_proxy*>& list) const {
  for (auto* proxy : list) {
    if (proxy != nullptr) {
      lock();

      pw_proxy_destroy(proxy);

      sync_wait_unlock();
    }
  }
}

/*
  Function inspired by code present in PipeWire's sources:
  https://gitlab.freedesktop.org/pipewire/pipewire/-/blob/master/spa/include/spa/utils/json.h#L350
*/

auto PipeManager::json_object_find(const char* obj, const char* key, char* value, const size_t& len) -> int {
  const char* v = nullptr;

  std::array<spa_json, 2U> sjson{};
  std::array<char, 128U> res{};

  spa_json_init(sjson.data(), obj, strlen(obj));

  if (spa_json_enter_object(sjson.data(), sjson.data() + 1) <= 0) {
    return -EINVAL;
  }

  while (spa_json_get_string(sjson.data() + 1, res.data(), res.size() * sizeof(char) - 1) > 0) {
    if (g_strcmp0(res.data(), key) == 0) {
      if (spa_json_get_string(sjson.data() + 1, value, len) <= 0) {
        continue;
      }

      return 0;
    }

    if (spa_json_next(sjson.data() + 1, &v) <= 0) {
      break;
    }
  }

  return -ENOENT;
}
