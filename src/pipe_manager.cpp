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

#include "pipe_manager.hpp"
#include <string>
#include "pipewire/keys.h"
#include "spa/param/audio/format.h"
#include "spa/param/audio/raw.h"
#include "spa/param/format.h"
#include "spa/param/param.h"
#include "spa/param/props.h"
#include "spa/pod/iter.h"
#include "spa/pod/parser.h"
#include "spa/pod/vararg.h"
#include "util.hpp"

namespace {

struct node_data {
  pw_proxy* proxy = nullptr;

  spa_hook proxy_listener{};

  spa_hook object_listener{};

  PipeManager* pm = nullptr;

  NodeInfo nd_info;
};

struct port_data {
  pw_proxy* proxy = nullptr;

  spa_hook proxy_listener{};

  spa_hook object_listener{};

  PipeManager* pm = nullptr;

  uint id = 0;
};

struct link_data {
  pw_proxy* proxy = nullptr;

  spa_hook proxy_listener{};

  spa_hook object_listener{};

  PipeManager* pm = nullptr;

  uint id = 0;
};

auto port_info_from_props(const spa_dict* props) -> PortInfo {
  PortInfo info;

  const auto* path = spa_dict_lookup(props, PW_KEY_OBJECT_PATH);
  const auto* node_id = spa_dict_lookup(props, PW_KEY_NODE_ID);
  const auto* port_name = spa_dict_lookup(props, PW_KEY_PORT_NAME);
  const auto* port_direction = spa_dict_lookup(props, PW_KEY_PORT_DIRECTION);
  const auto* port_channel = spa_dict_lookup(props, PW_KEY_AUDIO_CHANNEL);
  const auto* port_audio_format = spa_dict_lookup(props, PW_KEY_AUDIO_FORMAT);
  const auto* port_physical = spa_dict_lookup(props, PW_KEY_PORT_PHYSICAL);
  const auto* port_terminal = spa_dict_lookup(props, PW_KEY_PORT_TERMINAL);
  const auto* port_monitor = spa_dict_lookup(props, PW_KEY_PORT_MONITOR);

  if (path != nullptr) {
    info.path = path;
  }

  if (node_id != nullptr) {
    info.node_id = std::stoi(node_id);
  }

  if (port_name != nullptr) {
    info.name = port_name;
  }

  if (port_direction != nullptr) {
    info.direction = port_direction;
  }

  if (port_channel != nullptr) {
    info.audio_channel = port_channel;
  }

  if (port_audio_format != nullptr) {
    info.format_dsp = port_audio_format;
  }

  if (port_physical != nullptr) {
    if (strcmp(port_physical, "true") == 0) {
      info.physical = true;
    }
  }

  if (port_terminal != nullptr) {
    if (strcmp(port_terminal, "true") == 0) {
      info.terminal = true;
    }
  }

  if (port_monitor != nullptr) {
    if (strcmp(port_monitor, "true") == 0) {
      info.monitor = true;
    }
  }

  return info;
}

auto link_info_from_props(const spa_dict* props) -> LinkInfo {
  LinkInfo info;

  const auto* path = spa_dict_lookup(props, PW_KEY_OBJECT_PATH);
  const auto* input_node_id = spa_dict_lookup(props, PW_KEY_LINK_INPUT_NODE);
  const auto* input_port_id = spa_dict_lookup(props, PW_KEY_LINK_INPUT_PORT);
  const auto* output_node_id = spa_dict_lookup(props, PW_KEY_LINK_OUTPUT_NODE);
  const auto* output_port_id = spa_dict_lookup(props, PW_KEY_LINK_OUTPUT_PORT);
  const auto* passive = spa_dict_lookup(props, PW_KEY_LINK_PASSIVE);

  if (path != nullptr) {
    info.path = path;
  }

  if (input_node_id != nullptr) {
    info.input_node_id = std::stoi(input_node_id);
  }

  if (input_port_id != nullptr) {
    info.input_port_id = std::stoi(input_port_id);
  }

  if (output_node_id != nullptr) {
    info.output_node_id = std::stoi(output_node_id);
  }

  if (output_port_id != nullptr) {
    info.output_port_id = std::stoi(output_port_id);
  }

  if (passive != nullptr) {
    if (strcmp(passive, "true") == 0) {
      info.passive = true;
    }
  }

  return info;
}

void on_removed_node_proxy(void* data) {
  auto* pd = static_cast<node_data*>(data);

  pw_proxy_destroy(pd->proxy);

  pd->pm->list_nodes.erase(std::remove_if(pd->pm->list_nodes.begin(), pd->pm->list_nodes.end(),
                                          [=](auto& n) { return n.id == pd->nd_info.id; }),
                           pd->pm->list_nodes.end());

  util::debug(pd->pm->log_tag + pd->nd_info.media_class + " " + pd->nd_info.name + " was removed");

  if (pd->nd_info.media_class == "Audio/Source") {
    Glib::signal_idle().connect_once([pd] { pd->pm->source_removed.emit(pd->nd_info); });
  } else if (pd->nd_info.media_class == "Audio/Sink") {
    Glib::signal_idle().connect_once([pd] { pd->pm->sink_removed.emit(pd->nd_info); });
  } else if (pd->nd_info.media_class == "Stream/Output/Audio") {
    Glib::signal_idle().connect_once([pd] { pd->pm->stream_output_removed.emit(pd->nd_info); });
  } else if (pd->nd_info.media_class == "Stream/Input/Audio") {
    Glib::signal_idle().connect_once([pd] { pd->pm->stream_input_removed.emit(pd->nd_info); });
  }
}

void on_destroy_node_proxy(void* data) {
  auto* pd = static_cast<node_data*>(data);

  spa_hook_remove(&pd->proxy_listener);
  spa_hook_remove(&pd->object_listener);
}

const struct pw_proxy_events node_proxy_events = {PW_VERSION_PROXY_EVENTS, .destroy = on_destroy_node_proxy,
                                                  .removed = on_removed_node_proxy};

void on_node_info(void* object, const struct pw_node_info* info) {
  auto* nd = static_cast<node_data*>(object);

  for (auto& node : nd->pm->list_nodes) {
    if (node.id == info->id) {
      const auto* icon_name = spa_dict_lookup(info->props, PW_KEY_MEDIA_ICON_NAME);
      const auto* media_name = spa_dict_lookup(info->props, PW_KEY_MEDIA_NAME);
      const auto* prio_session = spa_dict_lookup(info->props, PW_KEY_PRIORITY_SESSION);
      const auto* node_latency = spa_dict_lookup(info->props, PW_KEY_NODE_LATENCY);

      nd->nd_info.state = info->state;
      nd->nd_info.n_input_ports = info->n_input_ports;
      nd->nd_info.n_output_ports = info->n_output_ports;

      if (prio_session != nullptr) {
        nd->nd_info.priority = std::stoi(prio_session);
      }

      if (icon_name != nullptr) {
        nd->nd_info.icon_name = icon_name;
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

      if (node.media_class == "Stream/Output/Audio") {
        Glib::signal_idle().connect_once([nd] { nd->pm->stream_output_changed.emit(nd->nd_info); });
      } else if (node.media_class == "Stream/Input/Audio") {
        Glib::signal_idle().connect_once([nd] { nd->pm->stream_input_changed.emit(nd->nd_info); });
      } else if (nd->nd_info.media_class == "Audio/Source") {
        Glib::signal_idle().connect_once([nd] { nd->pm->source_changed.emit(nd->nd_info); });
      } else if (nd->nd_info.media_class == "Audio/Sink") {
        Glib::signal_idle().connect_once([nd] { nd->pm->sink_changed.emit(nd->nd_info); });
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
          int rate = -1;

          if (spa_pod_get_int(&pod_prop->value, &rate) == 0) {
            for (auto& node : nd->pm->list_nodes) {
              if (node.id == nd->nd_info.id) {
                node.rate = rate;

                nd->nd_info.rate = rate;

                notify = true;

                util::debug(node.name + " sampling rate: " + std::to_string(rate));

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
          float volumes[SPA_AUDIO_MAX_CHANNELS];

          auto n_volumes = spa_pod_copy_array(&pod_prop->value, SPA_TYPE_Float, volumes, SPA_AUDIO_MAX_CHANNELS);

          for (auto& node : nd->pm->list_nodes) {
            if (node.id == nd->nd_info.id) {
              float max = 0.0F;

              for (uint i = 0; i < n_volumes; i++) {
                max = (volumes[i] > max) ? volumes[i] : max;
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
      if (nd->nd_info.media_class == "Stream/Output/Audio") {
        Glib::signal_idle().connect_once([nd] { nd->pm->stream_output_changed.emit(nd->nd_info); });
      } else if (nd->nd_info.media_class == "Stream/Input/Audio") {
        Glib::signal_idle().connect_once([nd] { nd->pm->stream_input_changed.emit(nd->nd_info); });
      } else if (nd->nd_info.media_class == "Audio/Source") {
        Glib::signal_idle().connect_once([nd] { nd->pm->source_changed.emit(nd->nd_info); });
      } else if (nd->nd_info.media_class == "Audio/Sink") {
        Glib::signal_idle().connect_once([nd] { nd->pm->sink_changed.emit(nd->nd_info); });
      }
    }
  }
}

void on_removed_port_proxy(void* data) {
  auto* pd = static_cast<port_data*>(data);

  pw_proxy_destroy(pd->proxy);

  pd->pm->list_ports.erase(
      std::remove_if(pd->pm->list_ports.begin(), pd->pm->list_ports.end(), [=](auto& n) { return n.id == pd->id; }),
      pd->pm->list_ports.end());
}

void on_destroy_port_proxy(void* data) {
  auto* pd = static_cast<port_data*>(data);

  spa_hook_remove(&pd->proxy_listener);
  spa_hook_remove(&pd->object_listener);
}

const struct pw_proxy_events port_proxy_events = {PW_VERSION_PROXY_EVENTS, .destroy = on_destroy_port_proxy,
                                                  .removed = on_removed_port_proxy};

void on_port_info(void* object, const struct pw_port_info* info) {
  auto* pd = static_cast<port_data*>(object);

  for (auto& port : pd->pm->list_ports) {
    if (port.id == info->id) {
      port = port_info_from_props(info->props);

      // util::warning("call: " + std::to_string(port.node_id));
    }

    break;
  }
}

void on_link_info(void* object, const struct pw_link_info* info) {
  auto* ld = static_cast<link_data*>(object);

  auto link = link_info_from_props(info->props);

  for (auto& l : ld->pm->list_links) {
    if (l.id == link.id) {
      l = link;

      break;
    }
  }

  bool found_input = false;
  bool found_output = false;
  NodeInfo input_node;
  NodeInfo output_node;

  for (auto& node : ld->pm->list_nodes) {
    if (link.input_node_id == node.id) {
      found_input = true;

      input_node = node;
    }

    if (link.output_node_id == node.id) {
      found_output = true;

      output_node = node;
    }
  }

  if (found_input and found_output) {
    util::debug(ld->pm->log_tag + output_node.name + " port " + std::to_string(link.output_port_id) +
                " is connected to " + input_node.name + " port " + std::to_string(link.input_port_id));
  }
}

void on_removed_link_proxy(void* data) {
  auto* ld = static_cast<link_data*>(data);

  pw_proxy_destroy(ld->proxy);

  ld->pm->list_links.erase(
      std::remove_if(ld->pm->list_links.begin(), ld->pm->list_links.end(), [=](auto& n) { return n.id == ld->id; }),
      ld->pm->list_links.end());
}

void on_destroy_link_proxy(void* data) {
  auto* ld = static_cast<link_data*>(data);

  spa_hook_remove(&ld->proxy_listener);
  spa_hook_remove(&ld->object_listener);
}

auto on_metadata_property(void* data, uint32_t id, const char* key, const char* type, const char* value) -> int {
  auto* pm = static_cast<PipeManager*>(data);

  util::debug(pm->log_tag + "new metadata property: " + std::to_string(id) + ", " + key + ", " + type + ", " + value);

  return 0;
}

const struct pw_metadata_events metadata_events = {PW_VERSION_METADATA_EVENTS, .property = on_metadata_property};

const struct pw_proxy_events link_proxy_events = {PW_VERSION_PROXY_EVENTS, .destroy = on_destroy_link_proxy,
                                                  .removed = on_removed_link_proxy};

const struct pw_node_events node_events = {PW_VERSION_NODE_EVENTS, .info = on_node_info, .param = on_node_event_param};

const struct pw_port_events port_events = {PW_VERSION_PORT_EVENTS, .info = on_port_info};

const struct pw_link_events link_events = {
    PW_VERSION_PORT_EVENTS,
    .info = on_link_info,
};

void on_registry_global(void* data,
                        uint32_t id,
                        uint32_t permissions,
                        const char* type,
                        uint32_t version,
                        const struct spa_dict* props) {
  auto* pm = static_cast<PipeManager*>(data);

  if (strcmp(type, PW_TYPE_INTERFACE_Node) == 0) {
    const auto* key_media_class = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);

    if (key_media_class != nullptr) {
      std::string media_class = key_media_class;

      if (media_class == "Audio/Sink" || media_class == "Audio/Source" || media_class == "Stream/Output/Audio" ||
          media_class == "Stream/Input/Audio") {
        const auto* node_name = spa_dict_lookup(props, PW_KEY_NODE_NAME);
        const auto* node_description = spa_dict_lookup(props, PW_KEY_NODE_DESCRIPTION);
        const auto* prio_session = spa_dict_lookup(props, PW_KEY_PRIORITY_SESSION);

        if (std::find(std::begin(pm->blocklist_node_name), std::end(pm->blocklist_node_name), node_name) !=
            std::end(pm->blocklist_node_name)) {
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

        if (node_name != nullptr) {
          pd->nd_info.name = node_name;
        }

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

        if (media_class == "Audio/Source") {
          Glib::signal_idle().connect_once([pd] { pd->pm->source_added.emit(pd->nd_info); });
        } else if (media_class == "Audio/Sink" && pd->nd_info.name != "pulseeffects_sink") {
          Glib::signal_idle().connect_once([pd] { pd->pm->sink_added.emit(pd->nd_info); });
        } else if (media_class == "Stream/Output/Audio") {
          Glib::signal_idle().connect_once([pd] { pd->pm->stream_output_added.emit(pd->nd_info); });
        } else if (media_class == "Stream/Input/Audio") {
          Glib::signal_idle().connect_once([pd] { pd->pm->stream_input_added.emit(pd->nd_info); });
        }
      }
    }
  }

  if (strcmp(type, PW_TYPE_INTERFACE_Port) == 0) {
    const auto* node_id_char = spa_dict_lookup(props, PW_KEY_NODE_ID);

    if (node_id_char == nullptr) {
      return;
    }

    uint node_id = std::stoi(node_id_char);

    for (auto& node : pm->list_nodes) {
      if (node.id == node_id) {
        auto* proxy =
            static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_PORT, sizeof(port_data)));

        auto* pd = static_cast<port_data*>(pw_proxy_get_user_data(proxy));

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
    }
  }

  if (strcmp(type, PW_TYPE_INTERFACE_Link) == 0) {
    auto link_info = link_info_from_props(props);

    link_info.id = id;

    for (auto& node : pm->list_nodes) {
      if (link_info.input_node_id == node.id || link_info.output_node_id == node.id) {
        auto* proxy =
            static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_LINK, sizeof(link_data)));

        auto* pd = static_cast<link_data*>(pw_proxy_get_user_data(proxy));

        pd->proxy = proxy;
        pd->pm = pm;
        pd->id = id;

        pw_link_add_listener(proxy, &pd->object_listener, &link_events, pd);
        pw_proxy_add_listener(proxy, &pd->proxy_listener, &link_proxy_events, pd);

        pm->list_links.emplace_back(link_info);

        return;
      }
    }
  }

  if (strcmp(type, PW_TYPE_INTERFACE_Metadata) == 0) {
    const auto* name = spa_dict_lookup(props, PW_KEY_METADATA_NAME);

    if (name != nullptr) {
      util::debug(pm->log_tag + "found metadata: " + name);

      if (pm->metadata != nullptr) {
        return;
      }

      pm->metadata = static_cast<pw_metadata*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_METADATA, 0));
    }

    pw_metadata_add_listener(pm->metadata, &pm->metadata_listener, &metadata_events, pm);
  }
}

void on_core_error(void* data, uint32_t id, int seq, int res, const char* message) {
  auto* pm = static_cast<PipeManager*>(data);

  util::warning(pm->log_tag + "Remote error on id:" + std::to_string(id));
  util::warning(pm->log_tag + "Remote error message:" + message);
}

void on_core_info(void* data, const struct pw_core_info* info) {
  auto* pm = static_cast<PipeManager*>(data);

  util::debug(pm->log_tag + "core version: " + info->version);
  util::debug(pm->log_tag + "core name: " + info->name);
}

void on_core_done(void* data, uint32_t id, int seq) {
  auto* pm = static_cast<PipeManager*>(data);

  util::debug(pm->log_tag + "connected to the core");

  pw_thread_loop_signal(pm->thread_loop, false);
}

const struct pw_core_events core_events = {.version = PW_VERSION_CORE_EVENTS,
                                           .info = on_core_info,
                                           .done = on_core_done,
                                           .error = on_core_error};

const struct pw_registry_events registry_events = {
    PW_VERSION_REGISTRY_EVENTS,
    .global = on_registry_global,
};

}  // namespace

PipeManager::PipeManager() {
  pw_init(nullptr, nullptr);

  spa_zero(core_listener);
  spa_zero(registry_listener);

  util::debug(log_tag + "compiled with pipewire: " + pw_get_headers_version());
  util::debug(log_tag + "linked to pipewire: " + pw_get_library_version());

  thread_loop = pw_thread_loop_new("pipewire-thread", nullptr);

  if (thread_loop == nullptr) {
    util::error(log_tag + "could not create pipewire loop");
  }

  if (pw_thread_loop_start(thread_loop) != 0) {
    util::error(log_tag + "could not start the loop");
  }

  pw_thread_loop_lock(thread_loop);

  context = pw_context_new(pw_thread_loop_get_loop(thread_loop), nullptr, 0);

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

  pw_properties_set(props_sink, PW_KEY_NODE_NAME, "pulseeffects_sink");
  pw_properties_set(props_sink, PW_KEY_NODE_DESCRIPTION, "PulseEffects Sink");
  pw_properties_set(props_sink, "factory.name", "support.null-audio-sink");
  pw_properties_set(props_sink, PW_KEY_MEDIA_CLASS, "Audio/Sink");
  pw_properties_set(props_sink, "audio.position", "FL,FR");

  proxy_stream_output_sink = static_cast<pw_proxy*>(
      pw_core_create_object(core, "adapter", PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, &props_sink->dict, 0));

  // loading our source

  // pw_properties* props_source = pw_properties_new(nullptr, nullptr);

  // pw_properties_set(props_source, PW_KEY_NODE_NAME, "pulseeffects_source");
  // pw_properties_set(props_source, PW_KEY_NODE_DESCRIPTION, "PulseEffects Source");
  // pw_properties_set(props_source, "factory.name", "support.null-audio-sink");
  // pw_properties_set(props_source, PW_KEY_MEDIA_CLASS, "Audio/Source");
  // pw_properties_set(props_source, "audio.position", "FL,FR");

  // proxy_stream_input_source = static_cast<pw_proxy*>(
  //     pw_core_create_object(core, "adapter", PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, &props_source->dict, 0));

  // filter = new PipeFilter(core);

  pw_core_sync(core, PW_ID_CORE, 0);

  pw_thread_loop_wait(thread_loop);

  pw_thread_loop_unlock(thread_loop);

  for (const auto& node : list_nodes) {
    if (node.name == "pulseeffects_sink") {
      pe_sink_node = node;
    }

    if (node.name == "pulseeffects_source") {
      pe_source_node = node;
    }
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

  // delete filter;

  pw_proxy_destroy(proxy_stream_output_sink);
  // pw_proxy_destroy(proxy_stream_input_source);

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

auto PipeManager::get_default_source() -> NodeInfo {
  int priority = -1;
  NodeInfo default_source;

  for (const auto& n : list_nodes) {
    if (n.media_class != "Audio/Source") {
      continue;
    }

    if (n.priority > priority) {
      priority = n.priority;

      default_source = n;
    }
  }

  return default_source;
}

auto PipeManager::get_default_sink() -> NodeInfo {
  int priority = -1;
  NodeInfo default_sink;

  for (const auto& n : list_nodes) {
    if (n.media_class != "Audio/Sink") {
      continue;
    }

    if (n.priority > priority) {
      priority = n.priority;

      default_sink = n;
    }
  }

  return default_sink;
}

void PipeManager::connect_stream_output(const NodeInfo& nd_info) const {
  if (nd_info.media_class == "Stream/Output/Audio") {
    pw_metadata_set_property(metadata, nd_info.id, "target.node", "Spa:Id", std::to_string(pe_sink_node.id).c_str());
  }
}

void PipeManager::disconnect_stream_output(const NodeInfo& nd_info) {
  if (nd_info.media_class == "Stream/Output/Audio") {
    auto default_sink = get_default_sink();

    pw_metadata_set_property(metadata, nd_info.id, "target.node", "Spa:Id", std::to_string(default_sink.id).c_str());
  }
}

void PipeManager::set_node_volume(const NodeInfo& nd_info, const float& value) {
  float volumes[SPA_AUDIO_MAX_CHANNELS];

  for (int i = 0; i < nd_info.n_volume_channels; i++) {
    volumes[i] = value;
  }

  char buffer[1024];

  auto builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

  pw_node_set_param((struct pw_node*)nd_info.proxy, SPA_PARAM_Props, 0,
                    (spa_pod*)spa_pod_builder_add_object(
                        &builder, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props, SPA_PROP_channelVolumes,
                        SPA_POD_Array(sizeof(float), SPA_TYPE_Float, nd_info.n_volume_channels, volumes)));
}

void PipeManager::set_node_mute(const NodeInfo& nd_info, const bool& state) {
  char buffer[1024];

  auto builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

  pw_node_set_param((pw_node*)nd_info.proxy, SPA_PARAM_Props, 0,
                    (spa_pod*)spa_pod_builder_add_object(&builder, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props,
                                                         SPA_PROP_mute, SPA_POD_Bool(state)));
}
