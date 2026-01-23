/**
 * Copyright © 2017-2026 Wellington Wallace
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

#include "pw_node_manager.hpp"

// For some reason our flatpak CI needs this header before <pipewire/node.h>
#include <pipewire/type.h>

#include <pipewire/core.h>
#include <pipewire/extensions/metadata.h>
#include <pipewire/keys.h>
#include <pipewire/node.h>
#include <pipewire/properties.h>
#include <pipewire/proxy.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <spa/param/audio/raw-types.h>
#include <spa/param/audio/raw.h>
#include <spa/param/format.h>
#include <spa/param/param.h>
#include <spa/param/props.h>
#include <spa/pod/builder.h>
#include <spa/pod/iter.h>
#include <spa/pod/pod.h>
#include <spa/utils/defs.h>
#include <spa/utils/dict.h>
#include <spa/utils/hook.h>
#include <spa/utils/result.h>
#include <spa/utils/type.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <format>
#include <pw_model_nodes.hpp>
#include <pw_objects.hpp>
#include <span>
#include <string>
#include <utility>
#include <vector>
#include "db_manager.hpp"
#include "pw_metadata_manager.hpp"
#include "tags_app.hpp"
#include "tags_pipewire.hpp"
#include "util.hpp"

namespace pw {

NodeManager::NodeManager(models::Nodes& model_nodes,
                         MetadataManager& metadata_manager,
                         NodeInfo& ee_sink_node,
                         NodeInfo& ee_source_node,
                         std::vector<LinkInfo>& list_links)
    : model_nodes(model_nodes),
      metadata_manager(metadata_manager),
      ee_sink_node(ee_sink_node),
      ee_source_node(ee_source_node),
      list_links(list_links) {}

void NodeManager::setNodeMute(uint64_t serial, bool state) {
  if (auto* proxy = model_nodes.get_proxy_by_serial(serial); proxy != nullptr) {
    std::array<char, 1024U> buffer{};

    auto builder = SPA_POD_BUILDER_INIT(buffer.data(), sizeof(buffer));  // NOLINT

    // NOLINTNEXTLINE
    pw_node_set_param((pw_node*)proxy, SPA_PARAM_Props, 0,
                      (spa_pod*)spa_pod_builder_add_object(&builder, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props,
                                                           SPA_PROP_mute, SPA_POD_Bool(state)));
  }
}

void NodeManager::setNodeVolume(uint64_t serial, uint n_vol_ch, float value) {
  if (auto* proxy = model_nodes.get_proxy_by_serial(serial); proxy != nullptr) {
    std::array<float, SPA_AUDIO_MAX_CHANNELS> volumes{};

    std::ranges::fill(volumes, 0.0F);
    std::fill_n(volumes.begin(), n_vol_ch, value);

    std::array<char, 1024U> buffer{};

    auto builder = SPA_POD_BUILDER_INIT(buffer.data(), sizeof(buffer));  // NOLINT

    // NOLINTNEXTLINE
    pw_node_set_param(
        (struct pw_node*)proxy, SPA_PARAM_Props, 0,
        (spa_pod*)spa_pod_builder_add_object(&builder, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props, SPA_PROP_channelVolumes,
                                             SPA_POD_Array(sizeof(float), SPA_TYPE_Float, n_vol_ch, volumes.data())));
  }
}

auto NodeManager::registerNode(pw_registry* registry, uint32_t id, const char* type, const spa_dict* props) -> bool {
  if (registry == nullptr || props == nullptr) {
    return false;
  }

  bool is_ee_filter = false;

  if (const auto* key_media_role = spa_dict_lookup(props, PW_KEY_MEDIA_ROLE)) {
    if (DbMain::ignoreSystemNotifications() &&
        std::ranges::find(blocklist_media_role, std::string(key_media_role)) != blocklist_media_role.end()) {
      return false;
    }

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
                     tags::pipewire::media_class::virtual_source, tags::pipewire::media_class::virtual_sink});

  if (!is_ee_filter && !std::ranges::any_of(class_array, [&](const auto& str) { return str == media_class; })) {
    return false;
  }

  QString node_name;

  util::spa_dict_get_string(props, PW_KEY_NODE_NAME, node_name);

  // At least for now I do not think there is a point in showing
  // the spectrum and the output level filters in menus

  if (node_name.contains("output_level") || node_name.contains("spectrum")) {
    return false;
  }

  // Exclude blocklisted node names

  if (std::ranges::find(blocklist_node_name, node_name) != blocklist_node_name.end()) {
    return false;
  }

  // Exclude blocklisted notification node names

  if (DbMain::ignoreSystemNotifications() &&
      std::ranges::find(blocklist_notification_nodes, node_name) != blocklist_notification_nodes.end()) {
    return false;
  }

  uint64_t serial = 0U;

  if (!util::spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, serial)) {
    util::warning("An error occurred while retrieving the object serial. The node cannot be handled by Easy Effects.");
    return false;
  }

  // New node can be added into the node map

  auto* proxy = static_cast<pw_proxy*>(pw_registry_bind(registry, id, type, PW_VERSION_NODE, sizeof(NodeData)));

  auto* nd = static_cast<NodeData*>(pw_proxy_get_user_data(proxy));

  nd->proxy = proxy;
  nd->nm = this;

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

  util::spa_dict_get_string(props, PW_KEY_NODE_DESCRIPTION, nd->nd_info->description);

  util::spa_dict_get_num(props, PW_KEY_PRIORITY_SESSION, nd->nd_info->priority);

  util::spa_dict_get_num(props, PW_KEY_DEVICE_ID, nd->nd_info->device_id);

  const auto user_blocklist = (media_class == tags::pipewire::media_class::output_stream) ? DbStreamOutputs::blocklist()
                                                                                          : DbStreamInputs::blocklist();

  nd->nd_info->is_blocklisted = std::ranges::find(user_blocklist, nd->nd_info->application_id) != user_blocklist.end();

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

  return true;
}

// Static callback implementations
void NodeManager::onNodeInfo(void* object, const pw_node_info* info) {
  auto* const nd = static_cast<NodeData*>(object);

  if (NodeManager::exiting || nd->nd_info == nullptr) {
    return;
  }

  auto* const nm = nd->nm;

  // Check if the node has to be removed

  bool ignore_node = false;

  // Exclude blocklisted App id.
  // To be checked here because PW_KEY_APP_ID is not set in on_registry_global.

  if (const auto* app_id = spa_dict_lookup(info->props, PW_KEY_APP_ID)) {
    if (std::ranges::find(nm->blocklist_app_id, app_id) != nm->blocklist_app_id.end()) {
      ignore_node = true;
    }
  }

  /**
   * Exclude capture streams.
   * Even PW_KEY_STREAM_CAPTURE_SINK is not set in on_registry_global.
   * Useful to exclude OBS recording streams.
   */

  if (const auto* is_capture_sink = spa_dict_lookup(info->props, PW_KEY_STREAM_CAPTURE_SINK)) {
    if (std::strcmp(is_capture_sink, "true") == 0 && DbMain::excludeMonitorStreams()) {
      ignore_node = true;
    }
  }

  if (nd->nd_info->media_class == tags::pipewire::media_class::input_stream) {
    if (const auto* target_object = spa_dict_lookup(info->props, PW_KEY_TARGET_OBJECT)) {
      // target.object can a name or serial number:
      // https://gitlab.freedesktop.org/pipewire/pipewire/-/blob/master/src/pipewire/keys.h#L334

      uint64_t serial = SPA_ID_INVALID;

      auto input_device = nm->model_nodes.get_node_by_name(DbStreamInputs::inputDevice());

      if (util::str_to_num(target_object, serial)) {
        if (serial != SPA_ID_INVALID && (serial != input_device.serial && serial != nm->ee_source_node.serial)) {
          ignore_node = true;
        }
      } else if (target_object != input_device.name && target_object != nm->ee_source_node.name) {
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

      auto output_device = nm->model_nodes.get_node_by_name(DbStreamOutputs::outputDevice());

      if (util::str_to_num(target_object, serial)) {
        if (serial != SPA_ID_INVALID && (serial != output_device.serial && serial != nm->ee_sink_node.serial)) {
          ignore_node = true;
        }
      } else if (target_object != output_device.name && target_object != nm->ee_sink_node.name) {
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
    // Just in case the previous tests returned false in the first callback iteration
    nm->model_nodes.remove_by_serial(nd->nd_info->serial);

    if (nd->proxy != nullptr) {
      pw_proxy_destroy(nd->proxy);
    }

    return;
  }

  // Check for node info updates

  nd->nd_info->state = info->state;
  nd->nd_info->n_input_ports = static_cast<int>(info->n_input_ports);
  nd->nd_info->n_output_ports = static_cast<int>(info->n_output_ports);

  util::spa_dict_get_num(info->props, PW_KEY_PRIORITY_SESSION, nd->nd_info->priority);

  if (const auto* app_id = spa_dict_lookup(info->props, PW_KEY_APP_ID)) {
    nd->nd_info->application_id = app_id;
  }

  // util::spa_dict_get_string(props, PW_KEY_APP_PROCESS_BINARY, app_process_binary);

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

  util::spa_dict_get_string(info->props, PW_KEY_DEVICE_ICON_NAME, nd->nd_info->device_icon_name);

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

  util::spa_dict_get_num(info->props, PW_KEY_DEVICE_ID, nd->nd_info->device_id);

  // Now that we know the media name we check the blocklist again

  bool blocklisted_media_name = false;

  const auto blocklist_uses_media_name = (nd->nd_info->media_class == tags::pipewire::media_class::output_stream)
                                             ? DbStreamOutputs::blocklistUsesMediaName()
                                             : DbStreamInputs::blocklistUsesMediaName();

  if (blocklist_uses_media_name) {
    const auto user_blocklist = (nd->nd_info->media_class == tags::pipewire::media_class::output_stream)
                                    ? DbStreamOutputs::blocklist()
                                    : DbStreamInputs::blocklist();

    blocklisted_media_name =
        std::ranges::find(user_blocklist, nd->nd_info->name + ":" + nd->nd_info->media_name) != user_blocklist.end();
  }

  if (nd->nd_info->connected != nm->stream_is_connected(info->id, nd->nd_info->media_class)) {
    nd->nd_info->connected = !nd->nd_info->connected;
  }

  if (nd->nd_info->media_class == tags::pipewire::media_class::source ||
      nd->nd_info->media_class == tags::pipewire::media_class::virtual_source) {
    const auto nd_info_copy = *nd->nd_info;

    if (nd_info_copy.serial == nm->ee_source_node.serial) {
      nm->ee_source_node = nd_info_copy;
    }

  } else if (nd->nd_info->media_class == tags::pipewire::media_class::sink ||
             nd->nd_info->media_class == tags::pipewire::media_class::virtual_sink) {
    const auto nd_info_copy = *nd->nd_info;

    if (nd_info_copy.serial == nm->ee_sink_node.serial) {
      nm->ee_sink_node = nd_info_copy;
    }
  }

  auto connect_to_ee_sink = [&]() {
    if (DbMain::processAllOutputs() && !nd->nd_info->is_blocklisted) {
      // target.node for backward compatibility with old PW session managers
      nm->metadata_manager.set_property(nd->nd_info->id, "target.node", "Spa:Id",
                                        util::to_string(nm->ee_sink_node.id).c_str());

      nm->metadata_manager.set_property(nd->nd_info->id, "target.object", "Spa:Id",
                                        util::to_string(nm->ee_sink_node.serial).c_str());
    }
  };

  auto connect_to_ee_source = [&]() {
    if (DbMain::processAllInputs() && !nd->nd_info->is_blocklisted) {
      // target.node for backward compatibility with old PW session managers
      nm->metadata_manager.set_property(nd->nd_info->id, "target.node", "Spa:Id",
                                        util::to_string(nm->ee_source_node.id).c_str());

      nm->metadata_manager.set_property(nd->nd_info->id, "target.object", "Spa:Id",
                                        util::to_string(nm->ee_source_node.serial).c_str());
    }
  };

  // update NodeInfo or add it if it is not in the model yet

  if (!nm->model_nodes.has_serial(nd->nd_info->serial)) {
    nm->model_nodes.append(*nd->nd_info);

    auto nd_info_copy = *nd->nd_info;

    if ((nd_info_copy.media_class == tags::pipewire::media_class::source ||
         nd_info_copy.media_class == tags::pipewire::media_class::virtual_source) &&
        nd_info_copy.name != tags::pipewire::ee_source_name) {
      Q_EMIT nm->sourceAdded(nd_info_copy);
    } else if ((nd_info_copy.media_class == tags::pipewire::media_class::sink ||
                nd_info_copy.media_class == tags::pipewire::media_class::virtual_sink) &&
               nd_info_copy.name != tags::pipewire::ee_sink_name) {
      Q_EMIT nm->sinkAdded(nd_info_copy);
    } else if (nd_info_copy.media_class == tags::pipewire::media_class::output_stream) {
      connect_to_ee_sink();
    } else if (nd_info_copy.media_class == tags::pipewire::media_class::input_stream) {
      connect_to_ee_source();
    }

  } else {
    nd->nd_info->is_blocklisted =
        blocklisted_media_name ? true : nm->model_nodes.get_node_by_id(nd->nd_info->id).is_blocklisted;

    /*
     * This information is updated elsewhere through the device object event
     */
    nd->nd_info->device_route_name = nm->model_nodes.get_node_by_id(nd->nd_info->id).device_route_name;
    nd->nd_info->device_route_description = nm->model_nodes.get_node_by_id(nd->nd_info->id).device_route_description;

    nm->model_nodes.update_info(*nd->nd_info);

    if (nd->nd_info->media_class == tags::pipewire::media_class::output_stream) {
      connect_to_ee_sink();
    } else if (nd->nd_info->media_class == tags::pipewire::media_class::input_stream) {
      connect_to_ee_source();
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

void NodeManager::onNodeParam(void* data,
                              [[maybe_unused]] int seq,
                              [[maybe_unused]] uint32_t id,
                              [[maybe_unused]] uint32_t index,
                              [[maybe_unused]] uint32_t next,
                              const spa_pod* param) {
  if (exiting || param == nullptr) {
    return;
  }

  auto* const nd = static_cast<NodeData*>(data);

  auto* const nm = nd->nm;

  const auto serial = nd->nd_info->serial;

  const auto rowIndex = nm->model_nodes.get_row_by_serial(serial);

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

          nm->model_nodes.update_field(rowIndex, pw::models::Nodes::Roles::Format, format_str);
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

        nm->model_nodes.update_field(rowIndex, pw::models::Nodes::Roles::Rate, rate);

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

        nm->model_nodes.update_field(rowIndex, pw::models::Nodes::Roles::Mute, v);

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

          nm->model_nodes.update_field(rowIndex, pw::models::Nodes::Roles::NvolumeChannels, n_volumes);
          nm->model_nodes.update_field(rowIndex, pw::models::Nodes::Roles::Volume, max);
        }

        break;
      }
      default:
        break;
    }
  }

  if (nd->nd_info->serial == nm->ee_source_node.serial) {
    nm->ee_source_node = *nd->nd_info;
  }

  if (nd->nd_info->serial == nm->ee_sink_node.serial) {
    nm->ee_sink_node = *nd->nd_info;
  }
}

void NodeManager::onDestroyNodeProxy(void* data) {
  auto* const nd = static_cast<NodeData*>(data);

  auto* const nm = nd->nm;

  spa_hook_remove(&nd->proxy_listener);

  nd->nd_info->proxy = nullptr;

  nm->model_nodes.remove_by_serial(nd->nd_info->serial);

  util::debug(std::format("{} {} {} has been removed", nd->nd_info->media_class.toStdString(), nd->nd_info->id,
                          nd->nd_info->name.toStdString()));

  delete nd->nd_info;

  nd->nd_info = nullptr;
}

void NodeManager::onRemovedNodeProxy(void* data) {
  auto* const nd = static_cast<NodeData*>(data);

  spa_hook_remove(&nd->object_listener);

  if (nd->proxy != nullptr) {
    pw_proxy_destroy(nd->proxy);
  }
}

auto NodeManager::load_virtual_devices(pw_core* core) -> std::pair<pw_proxy*, pw_proxy*> {
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

  auto proxy_stream_output_sink = static_cast<pw_proxy*>(
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

  auto proxy_stream_input_source = static_cast<pw_proxy*>(
      pw_core_create_object(core, "adapter", PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, &props_source->dict, 0));

  pw_properties_free(props_source);

  return std::make_pair(proxy_stream_input_source, proxy_stream_output_sink);
}

auto NodeManager::stream_is_connected(const uint& id, const QString& media_class) -> bool {
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

}  // namespace pw
