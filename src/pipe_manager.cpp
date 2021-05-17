/*
 *  Copyright © 2017-2020 Wellington Wallace
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

  uint id = 0;
};

void on_removed_proxy(void* data) {
  auto* pd = static_cast<proxy_data*>(data);

  spa_hook_remove(&pd->object_listener);

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
    if (strcmp(passive, "true") == 0) {
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
    if (strcmp(port_physical, "true") == 0) {
      info.physical = true;
    }
  }

  if (const auto* port_terminal = spa_dict_lookup(props, PW_KEY_PORT_TERMINAL)) {
    if (strcmp(port_terminal, "true") == 0) {
      info.terminal = true;
    }
  }

  if (const auto* port_monitor = spa_dict_lookup(props, PW_KEY_PORT_MONITOR)) {
    if (strcmp(port_monitor, "true") == 0) {
      info.monitor = true;
    }
  }

  return info;
}

void on_removed_node_proxy(void* data) {
  auto* pd = static_cast<node_data*>(data);

  spa_hook_remove(&pd->object_listener);

  pw_proxy_destroy(pd->proxy);
}

void on_destroy_node_proxy(void* data) {
  auto* pd = static_cast<node_data*>(data);
  auto* pm = pd->pm;

  auto nd_info = pd->nd_info;

  spa_hook_remove(&pd->proxy_listener);

  pd->pm->list_nodes.erase(std::remove_if(pd->pm->list_nodes.begin(), pd->pm->list_nodes.end(),
                                          [=](auto& n) { return n.id == pd->nd_info.id; }),
                           pd->pm->list_nodes.end());

  util::debug(pd->pm->log_tag + pd->nd_info.media_class + " " + pd->nd_info.name + " was removed");

  if (pd->nd_info.media_class == "Audio/Source") {
    Glib::signal_idle().connect_once([pm, nd_info] { pm->source_removed.emit(nd_info); });
  } else if (pd->nd_info.media_class == "Audio/Sink") {
    Glib::signal_idle().connect_once([pm, nd_info] { pm->sink_removed.emit(nd_info); });
  } else if (pd->nd_info.media_class == "Stream/Output/Audio") {
    Glib::signal_idle().connect_once([pm, nd_info] { pm->stream_output_removed.emit(nd_info); });
  } else if (pd->nd_info.media_class == "Stream/Input/Audio") {
    Glib::signal_idle().connect_once([pm, nd_info] { pm->stream_input_removed.emit(nd_info); });
  }
}

void on_node_info(void* object, const struct pw_node_info* info) {
  auto* nd = static_cast<node_data*>(object);

  for (auto& node : nd->pm->list_nodes) {
    if (node.id == info->id) {
      const auto* app_icon_name = spa_dict_lookup(info->props, PW_KEY_APP_ICON_NAME);
      const auto* media_icon_name = spa_dict_lookup(info->props, PW_KEY_MEDIA_ICON_NAME);
      const auto* media_name = spa_dict_lookup(info->props, PW_KEY_MEDIA_NAME);
      const auto* prio_session = spa_dict_lookup(info->props, PW_KEY_PRIORITY_SESSION);
      const auto* node_latency = spa_dict_lookup(info->props, PW_KEY_NODE_LATENCY);

      nd->nd_info.state = info->state;
      nd->nd_info.n_input_ports = info->n_input_ports;
      nd->nd_info.n_output_ports = info->n_output_ports;

      if (prio_session != nullptr) {
        nd->nd_info.priority = std::stoi(prio_session);
      }

      if (app_icon_name != nullptr) {
        nd->nd_info.app_icon_name = app_icon_name;
      }

      if (media_icon_name != nullptr) {
        nd->nd_info.media_icon_name = media_icon_name;
      }

      if (media_name != nullptr) {
        nd->nd_info.media_name = media_name;
      }

      if (node_latency != nullptr) {
        auto str = std::string(node_latency);

        auto delimiter_pos = str.find('/');

        auto latency_str = str.substr(0, delimiter_pos);

        auto rate_str = str.substr(delimiter_pos + 1);

        nd->nd_info.rate = std::stoi(rate_str);

        nd->nd_info.latency = std::stof(latency_str) / static_cast<float>(nd->nd_info.rate);
      }

      node = nd->nd_info;

      if ((info->change_mask & PW_NODE_CHANGE_MASK_PARAMS) != 0U) {
        for (uint i = 0; i < info->n_params; i++) {
          if ((info->params[i].flags & SPA_PARAM_INFO_READ) == 0U) {
            continue;
          }

          auto id = info->params[i].id;

          if (id == SPA_PARAM_Props || id == SPA_PARAM_EnumFormat || id == SPA_PARAM_Format) {
            pw_node_enum_params((struct pw_node*)nd->proxy, 0, id, 0, -1, nullptr);
          }
        }
      }

      auto* pm = nd->pm;

      if (node.media_class == "Stream/Output/Audio") {
        Glib::signal_idle().connect_once([pm, node] { pm->stream_output_changed.emit(node); });
      } else if (node.media_class == "Stream/Input/Audio") {
        Glib::signal_idle().connect_once([pm, node] { pm->stream_input_changed.emit(node); });
      } else if (nd->nd_info.media_class == "Audio/Source") {
        Glib::signal_idle().connect_once([pm, node] { pm->source_changed.emit(node); });
      } else if (nd->nd_info.media_class == "Audio/Sink") {
        Glib::signal_idle().connect_once([pm, node] { pm->sink_changed.emit(node); });
      }

      break;
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
  auto* nd = static_cast<node_data*>(object);

  if (param != nullptr) {
    spa_pod_prop* pod_prop = nullptr;
    auto* obj = (spa_pod_object*)param;
    bool notify = false;

    SPA_POD_OBJECT_FOREACH(obj, pod_prop) {
      switch (pod_prop->key) {
        case SPA_FORMAT_AUDIO_format: {
          uint format = 0;

          if (spa_pod_get_id(&pod_prop->value, &format) == 0) {
            for (auto& node : nd->pm->list_nodes) {
              if (node.id == nd->nd_info.id) {
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

                node.format = format_str;

                nd->nd_info.format = format_str;

                notify = true;
              }
            }
          }

          break;
        }
        case SPA_FORMAT_AUDIO_rate: {
          int rate = 1;

          if (spa_pod_get_int(&pod_prop->value, &rate) == 0) {
            for (auto& node : nd->pm->list_nodes) {
              if (node.id == nd->nd_info.id) {
                node.rate = rate;

                nd->nd_info.rate = rate;

                notify = true;

                // util::debug(node.name + " sampling rate: " + std::to_string(rate));

                break;
              }
            }
          }

          break;
        }
        case SPA_PROP_mute: {
          bool v = false;

          if (spa_pod_get_bool(&pod_prop->value, &v) == 0) {
            for (auto& node : nd->pm->list_nodes) {
              if (node.id == nd->nd_info.id) {
                node.mute = v;

                nd->nd_info.mute = v;

                notify = true;

                // util::warning("mute " + std::to_string(static_cast<int>(v)));

                break;
              }
            }
          }

          break;
        }
        case SPA_PROP_channelVolumes: {
          // float volumes[SPA_AUDIO_MAX_CHANNELS];
          std::array<float, SPA_AUDIO_MAX_CHANNELS> volumes{};

          auto n_volumes = spa_pod_copy_array(&pod_prop->value, SPA_TYPE_Float, volumes.data(), SPA_AUDIO_MAX_CHANNELS);

          for (auto& node : nd->pm->list_nodes) {
            if (node.id == nd->nd_info.id) {
              float max = 0.0F;

              for (uint i = 0; i < n_volumes; i++) {
                max = (volumes.at(i) > max) ? volumes.at(i) : max;
              }

              node.n_volume_channels = n_volumes;
              node.volume = max;

              nd->nd_info.n_volume_channels = n_volumes;
              nd->nd_info.volume = max;

              notify = true;

              // util::debug(node.name + " volume: " + std::to_string(max));

              break;
            }
          }

          break;
        }
        default:
          break;
      }
    }

    if (notify) {
      auto* pm = nd->pm;
      NodeInfo nd_info = nd->nd_info;  // sometimes PipeWire destroys the pointer before signal_idle is called

      if (nd_info.media_class == "Stream/Output/Audio") {
        Glib::signal_idle().connect_once([pm, nd_info] { pm->stream_output_changed.emit(nd_info); });
      } else if (nd_info.media_class == "Stream/Input/Audio") {
        Glib::signal_idle().connect_once([pm, nd_info] { pm->stream_input_changed.emit(nd_info); });
      } else if (nd_info.media_class == "Audio/Source/Virtual") {
        if (nd_info.id == pm->pe_source_node.id) {
          pm->pe_source_node = nd_info;
        }

        Glib::signal_idle().connect_once([pm, nd_info] { pm->source_changed.emit(nd_info); });
      } else if (nd_info.media_class == "Audio/Sink") {
        if (nd_info.id == pm->pe_sink_node.id) {
          pm->pe_sink_node = nd_info;
        }

        Glib::signal_idle().connect_once([pm, nd_info] { pm->sink_changed.emit(nd_info); });
      }
    }
  }
}

void on_link_info(void* object, const struct pw_link_info* info) {
  auto* ld = static_cast<proxy_data*>(object);
  auto* pm = ld->pm;

  LinkInfo link;

  for (auto& l : ld->pm->list_links) {
    if (l.id == ld->id) {
      l.state = info->state;

      link = l;

      Glib::signal_idle().connect_once([pm, link] { pm->link_changed.emit(link); });

      // util::warning(pw_link_state_as_string(l.state));

      break;
    }
  }

  // const struct spa_dict_item* item = nullptr;
  // spa_dict_for_each(item, info->props) printf("\t\t%s: \"%s\"\n", item->key, item->value);
}

void on_destroy_link_proxy(void* data) {
  auto* ld = static_cast<proxy_data*>(data);

  spa_hook_remove(&ld->proxy_listener);

  ld->pm->list_links.erase(
      std::remove_if(ld->pm->list_links.begin(), ld->pm->list_links.end(), [=](auto& n) { return n.id == ld->id; }),
      ld->pm->list_links.end());
}

void on_port_info(void* object, const struct pw_port_info* info) {
  auto* pd = static_cast<proxy_data*>(object);
  auto* pm = pd->pm;

  PortInfo port_info;

  for (auto& port : pd->pm->list_ports) {
    if (port.id == info->id) {
      port_info = port_info_from_props(info->props);
      port_info.id = port.id;

      port = port_info;

      Glib::signal_idle().connect_once([pm, port_info] { pm->port_changed.emit(port_info); });
    }

    break;
  }

  // const struct spa_dict_item* item = nullptr;
  // spa_dict_for_each(item, info->props) printf("\t\t%s: \"%s\"\n", item->key, item->value);
}

void on_destroy_port_proxy(void* data) {
  auto* ld = static_cast<proxy_data*>(data);

  spa_hook_remove(&ld->proxy_listener);

  ld->pm->list_ports.erase(
      std::remove_if(ld->pm->list_ports.begin(), ld->pm->list_ports.end(), [=](auto& n) { return n.id == ld->id; }),
      ld->pm->list_ports.end());
}

void on_module_info(void* object, const struct pw_module_info* info) {
  auto* ld = static_cast<proxy_data*>(object);

  for (auto& module : ld->pm->list_modules) {
    if (module.id == info->id) {
      if (info->filename != nullptr) {
        module.filename = info->filename;
      }

      const auto* description = spa_dict_lookup(info->props, PW_KEY_MODULE_DESCRIPTION);

      if (description != nullptr) {
        module.description = description;
      }

      break;
    }
  }
}

void on_destroy_module_proxy(void* data) {
  auto* md = static_cast<proxy_data*>(data);

  spa_hook_remove(&md->proxy_listener);

  md->pm->list_modules.erase(
      std::remove_if(md->pm->list_modules.begin(), md->pm->list_modules.end(), [=](auto& n) { return n.id == md->id; }),
      md->pm->list_modules.end());
}

void on_client_info(void* object, const struct pw_client_info* info) {
  auto* ld = static_cast<proxy_data*>(object);

  for (auto& client : ld->pm->list_clients) {
    if (client.id == info->id) {
      const auto* name = spa_dict_lookup(info->props, PW_KEY_APP_NAME);
      const auto* access = spa_dict_lookup(info->props, PW_KEY_ACCESS);
      const auto* api = spa_dict_lookup(info->props, PW_KEY_CLIENT_API);

      if (name != nullptr) {
        client.name = name;
      }

      if (access != nullptr) {
        client.access = access;
      }

      if (api != nullptr) {
        client.api = api;
      }

      break;
    }
  }
}

void on_destroy_client_proxy(void* data) {
  auto* pd = static_cast<proxy_data*>(data);

  spa_hook_remove(&pd->proxy_listener);

  pd->pm->list_clients.erase(
      std::remove_if(pd->pm->list_clients.begin(), pd->pm->list_clients.end(), [=](auto& n) { return n.id == pd->id; }),
      pd->pm->list_clients.end());
}

auto on_metadata_property(void* data, uint32_t id, const char* key, const char* type, const char* value) -> int {
  auto* pm = static_cast<PipeManager*>(data);

  std::string str_key;
  std::string str_type;
  std::string str_value;
  auto str_id = std::to_string(id);

  if (key != nullptr) {
    str_key = key;
  }

  if (type != nullptr) {
    str_type = type;
  }

  if (value != nullptr) {
    str_value = value;
  }

  util::debug(pm->log_tag + "new metadata property: " + str_id + ", " + str_key + ", " + str_type + ", " + str_value);

  if (str_value.empty()) {
    return 0;
  }

  if (str_key == "default.audio.sink") {
    std::array<char, 1024> v{};

    PipeManager::json_object_find(str_value.c_str(), "name", v.data(), v.size() * sizeof(char));

    for (auto& node : pm->list_nodes) {
      if (node.name == v.data()) {
        if (node.name == "easyeffects_sink") {
          pm->default_output_device.id = SPA_ID_INVALID;

          return 0;
        }

        if (node.media_class == "Audio/Sink") {
          pm->default_output_device = node;

          Glib::signal_idle().connect_once([pm, node] { pm->new_default_sink.emit(node); });
        }

        break;
      }
    }
  }

  if (str_key == "default.audio.source") {
    std::array<char, 1024> v{};

    PipeManager::json_object_find(str_value.c_str(), "name", v.data(), v.size() * sizeof(char));

    for (auto& node : pm->list_nodes) {
      if (node.name == v.data()) {
        if (node.name == "easyeffects_source") {
          pm->default_input_device.id = SPA_ID_INVALID;

          return 0;
        }

        if (node.media_class == "Audio/Source" || node.media_class == "Audio/Source/Virtual") {
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

const struct pw_proxy_events node_proxy_events = {.destroy = on_destroy_node_proxy,
                                                  .bound = nullptr,
                                                  .removed = on_removed_node_proxy,
                                                  .done = nullptr,
                                                  .error = nullptr};

const struct pw_node_events node_events = {.info = on_node_info, .param = on_node_event_param};

const struct pw_link_events link_events = {
    .info = on_link_info,
};

const struct pw_port_events port_events = {
    .info = on_port_info,
};

const struct pw_module_events module_events = {
    .info = on_module_info,
};

const struct pw_client_events client_events = {
    .info = on_client_info,
};

void on_registry_global(void* data,
                        uint32_t id,
                        uint32_t permissions,
                        const char* type,
                        uint32_t version,
                        const struct spa_dict* props) {
  auto* pm = static_cast<PipeManager*>(data);

  if (strcmp(type, PW_TYPE_INTERFACE_Node) == 0) {
    const auto* key_media_role = spa_dict_lookup(props, PW_KEY_MEDIA_ROLE);

    if (key_media_role != nullptr) {
      if (std::ranges::find(pm->blocklist_media_role, std::string(key_media_role)) != pm->blocklist_media_role.end()) {
        return;
      }

      if (strcmp(key_media_role, "DSP") == 0) {
        const auto* key_media_category = spa_dict_lookup(props, PW_KEY_MEDIA_CATEGORY);

        if (key_media_category == nullptr) {
          return;
        }

        if (strcmp(key_media_category, "Filter") == 0) {
          const auto* key_node_description = spa_dict_lookup(props, PW_KEY_NODE_DESCRIPTION);

          if (key_node_description == nullptr) {
            return;
          }

          if (strcmp(key_node_description, "easyeffects_filter") == 0) {
            const auto* node_name = spa_dict_lookup(props, PW_KEY_NODE_NAME);

            util::debug(pm->log_tag + "Filter " + node_name + ", id = " + std::to_string(id) + ", was added");
          }
        }
      }
    }

    const auto* key_media_class = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);

    if (key_media_class != nullptr) {
      std::string media_class = key_media_class;

      if (media_class == "Audio/Sink" || media_class == "Audio/Source" || media_class == "Audio/Source/Virtual" ||
          media_class == "Stream/Output/Audio" || media_class == "Stream/Input/Audio") {
        const auto* node_name = spa_dict_lookup(props, PW_KEY_NODE_NAME);
        const auto* node_description = spa_dict_lookup(props, PW_KEY_NODE_DESCRIPTION);
        const auto* prio_session = spa_dict_lookup(props, PW_KEY_PRIORITY_SESSION);

        if (node_name == nullptr) {
          return;
        }

        std::string name = node_name;

        if (name.empty()) {
          return;
        }

        if (std::ranges::find(pm->blocklist_node_name, name) != pm->blocklist_node_name.end()) {
          return;
        }

        auto* proxy =
            static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_NODE, sizeof(node_data)));

        auto* pd = static_cast<node_data*>(pw_proxy_get_user_data(proxy));

        pd->proxy = proxy;
        pd->pm = pm;
        pd->nd_info.proxy = proxy;
        pd->nd_info.id = id;
        pd->nd_info.media_class = media_class;
        pd->nd_info.name = name;

        if (node_description != nullptr) {
          pd->nd_info.description = node_description;
        }

        if (prio_session != nullptr) {
          pd->nd_info.priority = std::stoi(prio_session);
        }

        pw_node_add_listener(proxy, &pd->object_listener, &node_events, pd);
        pw_proxy_add_listener(proxy, &pd->proxy_listener, &node_proxy_events, pd);

        pm->list_nodes.emplace_back(pd->nd_info);

        util::debug(pm->log_tag + media_class + " " + std::to_string(id) + " " + pd->nd_info.name + " was added");

        NodeInfo nd_info = pd->nd_info;

        if (media_class == "Audio/Source" && nd_info.name != "easyeffects_source") {
          Glib::signal_idle().connect_once([pm, nd_info] { pm->source_added.emit(nd_info); });
        } else if (media_class == "Audio/Sink" && nd_info.name != "easyeffects_sink") {
          Glib::signal_idle().connect_once([pm, nd_info] { pm->sink_added.emit(nd_info); });
        } else if (media_class == "Stream/Output/Audio") {
          Glib::signal_idle().connect_once([pm, nd_info] { pm->stream_output_added.emit(nd_info); });
        } else if (media_class == "Stream/Input/Audio") {
          Glib::signal_idle().connect_once([pm, nd_info] { pm->stream_input_added.emit(nd_info); });
        }
      }
    }

    return;
  }

  if (strcmp(type, PW_TYPE_INTERFACE_Link) == 0) {
    auto* proxy = static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_LINK, sizeof(proxy_data)));

    auto* pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

    pd->proxy = proxy;
    pd->pm = pm;
    pd->id = id;

    pw_link_add_listener(proxy, &pd->object_listener, &link_events, pd);
    pw_proxy_add_listener(proxy, &pd->proxy_listener, &link_proxy_events, pd);

    auto link_info = link_info_from_props(props);

    link_info.id = id;

    pm->list_links.emplace_back(link_info);

    bool found_input = false;
    bool found_output = false;
    NodeInfo input_node;
    NodeInfo output_node;

    for (auto& node : pm->list_nodes) {
      if (link_info.input_node_id == node.id) {
        found_input = true;

        input_node = node;
      } else if (link_info.output_node_id == node.id) {
        found_output = true;

        output_node = node;
      }
    }

    if (found_input and found_output) {
      util::debug(pm->log_tag + output_node.name + " port " + std::to_string(link_info.output_port_id) +
                  " is connected to " + input_node.name + " port " + std::to_string(link_info.input_port_id));
    }

    return;
  }

  if (strcmp(type, PW_TYPE_INTERFACE_Port) == 0) {
    auto* proxy = static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_PORT, sizeof(proxy_data)));

    auto* pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

    pd->proxy = proxy;
    pd->pm = pm;
    pd->id = id;

    pw_port_add_listener(proxy, &pd->object_listener, &port_events, pd);
    pw_proxy_add_listener(proxy, &pd->proxy_listener, &port_proxy_events, pd);

    auto port_info = port_info_from_props(props);

    port_info.id = id;

    pm->list_ports.emplace_back(port_info);

    return;
  }

  if (strcmp(type, PW_TYPE_INTERFACE_Module) == 0) {
    auto* proxy =
        static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_MODULE, sizeof(proxy_data)));

    auto* pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

    pd->proxy = proxy;
    pd->pm = pm;
    pd->id = id;

    pw_module_add_listener(proxy, &pd->object_listener, &module_events, pd);
    pw_proxy_add_listener(proxy, &pd->proxy_listener, &module_proxy_events, pd);

    ModuleInfo m_info{.id = id};

    const auto* name = spa_dict_lookup(props, PW_KEY_MODULE_NAME);

    if (name != nullptr) {
      m_info.name = name;
    }

    pm->list_modules.emplace_back(m_info);

    return;
  }

  if (strcmp(type, PW_TYPE_INTERFACE_Client) == 0) {
    auto* proxy =
        static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_CLIENT, sizeof(proxy_data)));

    auto* pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

    pd->proxy = proxy;
    pd->pm = pm;
    pd->id = id;

    pw_client_add_listener(proxy, &pd->object_listener, &client_events, pd);
    pw_proxy_add_listener(proxy, &pd->proxy_listener, &client_proxy_events, pd);

    ClientInfo c_info{.id = id};

    pm->list_clients.emplace_back(c_info);

    return;
  }

  if (strcmp(type, PW_TYPE_INTERFACE_Metadata) == 0) {
    const auto* name = spa_dict_lookup(props, PW_KEY_METADATA_NAME);

    if (name != nullptr) {
      util::debug(pm->log_tag + "found metadata: " + name);

      if (pm->metadata != nullptr) {
        return;
      }

      pm->metadata = static_cast<pw_metadata*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_METADATA, 0));

      pw_metadata_add_listener(pm->metadata, &pm->metadata_listener, &metadata_events, pm);

      return;
    }

    return;
  }
}

void on_core_error(void* data, uint32_t id, int seq, int res, const char* message) {
  auto* pm = static_cast<PipeManager*>(data);

  if (id == PW_ID_CORE) {
    util::warning(pm->log_tag + "Remote error res: " + spa_strerror(res));
    util::warning(pm->log_tag + "Remote error message: " + message);

    pw_thread_loop_signal(pm->thread_loop, false);
  }
}

void on_core_info(void* data, const struct pw_core_info* info) {
  auto* pm = static_cast<PipeManager*>(data);

  util::debug(pm->log_tag + "core version: " + info->version);
  util::debug(pm->log_tag + "core name: " + info->name);

  const auto* rate = spa_dict_lookup(info->props, "default.clock.rate");
  const auto* min_quantum = spa_dict_lookup(info->props, "default.clock.min-quantum");
  const auto* max_quantum = spa_dict_lookup(info->props, "default.clock.max-quantum");
  const auto* quantum = spa_dict_lookup(info->props, "default.clock.quantum");

  pm->core_name = info->name;

  if (rate != nullptr) {
    pm->default_clock_rate = rate;
  }

  if (min_quantum != nullptr) {
    pm->default_min_quantum = min_quantum;
  }

  if (max_quantum != nullptr) {
    pm->default_max_quantum = max_quantum;
  }

  if (quantum != nullptr) {
    pm->default_quantum = quantum;
  }
}

void on_core_done(void* data, uint32_t id, int seq) {
  auto* pm = static_cast<PipeManager*>(data);

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

  pw_thread_loop_lock(thread_loop);

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

  // loading our sink

  pw_properties* props_sink = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_sink, PW_KEY_NODE_NAME, "easyeffects_sink");
  pw_properties_set(props_sink, PW_KEY_NODE_DESCRIPTION, "EasyEffects Sink");
  pw_properties_set(props_sink, "factory.name", "support.null-audio-sink");
  pw_properties_set(props_sink, PW_KEY_MEDIA_CLASS, "Audio/Sink");
  pw_properties_set(props_sink, "audio.position", "FL,FR");
  pw_properties_set(props_sink, "monitor.channel-volumes", "true");

  proxy_stream_output_sink = static_cast<pw_proxy*>(
      pw_core_create_object(core, "adapter", PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, &props_sink->dict, 0));

  // loading our source

  pw_properties* props_source = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_source, PW_KEY_NODE_NAME, "easyeffects_source");
  pw_properties_set(props_source, PW_KEY_NODE_DESCRIPTION, "EasyEffects Source");
  pw_properties_set(props_source, "factory.name", "support.null-audio-sink");
  pw_properties_set(props_source, PW_KEY_MEDIA_CLASS, "Audio/Source/Virtual");
  pw_properties_set(props_source, "audio.position", "FL,FR");
  pw_properties_set(props_source, "monitor.channel-volumes", "true");

  proxy_stream_input_source = static_cast<pw_proxy*>(
      pw_core_create_object(core, "adapter", PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, &props_source->dict, 0));

  pw_core_sync(core, PW_ID_CORE, 0);

  pw_thread_loop_wait(thread_loop);

  pw_thread_loop_unlock(thread_loop);

  while (pe_sink_node.id == SPA_ID_INVALID || pe_source_node.id == SPA_ID_INVALID) {
    for (const auto& node : list_nodes) {
      if (node.name == "easyeffects_sink") {
        pe_sink_node = node;
      }

      if (node.name == "easyeffects_source") {
        pe_source_node = node;
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

PipeManager::~PipeManager() {
  pw_thread_loop_lock(thread_loop);

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

  pw_thread_loop_unlock(thread_loop);

  util::debug(log_tag + "Destroying Pipewire loop...");
  pw_thread_loop_destroy(thread_loop);
}

void PipeManager::connect_stream_output(const NodeInfo& nd_info) const {
  if (nd_info.media_class == "Stream/Output/Audio") {
    pw_thread_loop_lock(thread_loop);

    pw_metadata_set_property(metadata, nd_info.id, "target.node", "Spa:Id", std::to_string(pe_sink_node.id).c_str());

    pw_core_sync(core, PW_ID_CORE, 0);

    pw_thread_loop_wait(thread_loop);

    pw_thread_loop_unlock(thread_loop);
  }
}

void PipeManager::disconnect_stream_output(const NodeInfo& nd_info) const {
  if (nd_info.media_class == "Stream/Output/Audio") {
    pw_thread_loop_lock(thread_loop);

    pw_metadata_set_property(metadata, nd_info.id, "target.node", "Spa:Id",
                             std::to_string(default_output_device.id).c_str());

    pw_core_sync(core, PW_ID_CORE, 0);

    pw_thread_loop_wait(thread_loop);

    pw_thread_loop_unlock(thread_loop);
  }
}

void PipeManager::connect_stream_input(const NodeInfo& nd_info) const {
  if (nd_info.media_class == "Stream/Input/Audio") {
    pw_thread_loop_lock(thread_loop);

    pw_metadata_set_property(metadata, nd_info.id, "target.node", "Spa:Id", std::to_string(pe_source_node.id).c_str());

    pw_core_sync(core, PW_ID_CORE, 0);

    pw_thread_loop_wait(thread_loop);

    pw_thread_loop_unlock(thread_loop);
  }
}

void PipeManager::disconnect_stream_input(const NodeInfo& nd_info) const {
  if (nd_info.media_class == "Stream/Input/Audio") {
    pw_thread_loop_lock(thread_loop);

    pw_metadata_set_property(metadata, nd_info.id, "target.node", "Spa:Id",
                             std::to_string(default_input_device.id).c_str());

    pw_core_sync(core, PW_ID_CORE, 0);

    pw_thread_loop_wait(thread_loop);

    pw_thread_loop_unlock(thread_loop);
  }
}

void PipeManager::set_node_volume(const NodeInfo& nd_info, const float& value) {
  std::array<float, SPA_AUDIO_MAX_CHANNELS> volumes{};

  std::ranges::fill(volumes, 0.0F);
  std::fill_n(volumes.begin(), nd_info.n_volume_channels, value);

  std::array<char, 1024> buffer{};

  auto builder = SPA_POD_BUILDER_INIT(buffer.data(), sizeof(buffer));

  pw_node_set_param((struct pw_node*)nd_info.proxy, SPA_PARAM_Props, 0,
                    (spa_pod*)spa_pod_builder_add_object(
                        &builder, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props, SPA_PROP_channelVolumes,
                        SPA_POD_Array(sizeof(float), SPA_TYPE_Float, nd_info.n_volume_channels, volumes.data())));
}

void PipeManager::set_node_mute(const NodeInfo& nd_info, const bool& state) {
  std::array<char, 1024> buffer{};

  auto builder = SPA_POD_BUILDER_INIT(buffer.data(), sizeof(buffer));

  pw_node_set_param((pw_node*)nd_info.proxy, SPA_PARAM_Props, 0,
                    (spa_pod*)spa_pod_builder_add_object(&builder, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props,
                                                         SPA_PROP_mute, SPA_POD_Bool(state)));
}

auto PipeManager::link_nodes(const uint& output_node_id, const uint& input_node_id) -> std::vector<pw_proxy*> {
  std::vector<pw_proxy*> list;
  std::vector<PortInfo> list_output_ports;
  std::vector<PortInfo> list_input_ports;
  bool use_audio_channel = true;

  for (auto& port : list_ports) {
    if (port.node_id == output_node_id && port.direction == "out") {
      list_output_ports.emplace_back(port);

      if (port.audio_channel != "FL" && port.audio_channel != "FR") {
        use_audio_channel = false;
      }
    }

    if (port.node_id == input_node_id && port.direction == "in") {
      list_input_ports.emplace_back(port);

      if (port.audio_channel != "FL" && port.audio_channel != "FR") {
        use_audio_channel = false;
      }
    }
  }

  for (auto& outp : list_output_ports) {
    for (auto& inp : list_input_ports) {
      bool ports_match = false;

      if (use_audio_channel) {
        ports_match = outp.audio_channel == inp.audio_channel;
      } else {
        ports_match = outp.port_id == inp.port_id;
      }

      if (ports_match) {
        lock();

        pw_properties* props = pw_properties_new(nullptr, nullptr);

        pw_properties_set(props, PW_KEY_LINK_PASSIVE, "true");
        pw_properties_set(props, PW_KEY_OBJECT_LINGER, "false");
        pw_properties_set(props, PW_KEY_LINK_OUTPUT_NODE, std::to_string(output_node_id).c_str());
        pw_properties_set(props, PW_KEY_LINK_OUTPUT_PORT, std::to_string(outp.id).c_str());
        pw_properties_set(props, PW_KEY_LINK_INPUT_NODE, std::to_string(input_node_id).c_str());
        pw_properties_set(props, PW_KEY_LINK_INPUT_PORT, std::to_string(inp.id).c_str());

        auto* proxy = static_cast<pw_proxy*>(
            pw_core_create_object(core, "link-factory", PW_TYPE_INTERFACE_Link, PW_VERSION_LINK, &props->dict, 0));

        if (proxy == nullptr) {
          util::warning(log_tag + "failed to link the node " + std::to_string(output_node_id) + " to " +
                        std::to_string(input_node_id));

          unlock();

          return list;
        }

        pw_core_sync(core, PW_ID_CORE, 0);

        pw_thread_loop_wait(thread_loop);

        unlock();

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

void PipeManager::destroy_object(const int& id) const {
  lock();

  pw_registry_destroy(registry, id);

  pw_core_sync(core, PW_ID_CORE, 0);

  pw_thread_loop_wait(thread_loop);

  unlock();
}

void PipeManager::destroy_links(const std::vector<pw_proxy*>& list) const {
  for (auto* proxy : list) {
    if (proxy != nullptr) {
      lock();

      pw_proxy_destroy(proxy);

      pw_core_sync(core, PW_ID_CORE, 0);

      pw_thread_loop_wait(thread_loop);

      unlock();
    }
  }
}

/*
  Function inspired by code present in PipeWire's sources:
  https://gitlab.freedesktop.org/pipewire/pipewire/-/blob/master/spa/include/spa/utils/json.h#L350
*/

auto PipeManager::json_object_find(const char* obj, const char* key, char* value, size_t len) -> int {
  const char* v = nullptr;

  std::array<spa_json, 2> sjson{};
  std::array<char, 128> res{};

  spa_json_init(sjson.data(), obj, strlen(obj));

  if (spa_json_enter_object(sjson.data(), sjson.data() + 1) <= 0) {
    return -EINVAL;
  }

  while (spa_json_get_string(sjson.data() + 1, res.data(), res.size() * sizeof(char) - 1) > 0) {
    if (strcmp(res.data(), key) == 0) {
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