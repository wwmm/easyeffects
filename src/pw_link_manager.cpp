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

#include "pw_link_manager.hpp"

// For some reason our flatpak CI needs this header before <pipewire/link.h>
#include <pipewire/type.h>

#include <pipewire/context.h>
#include <pipewire/core.h>
#include <pipewire/keys.h>
#include <pipewire/link.h>
#include <pipewire/port.h>
#include <pipewire/properties.h>
#include <pipewire/proxy.h>
#include <pipewire/thread-loop.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <spa/utils/hook.h>
#include <algorithm>
#include <cstdint>
#include <format>
#include <stdexcept>
#include <utility>
#include <vector>
#include "pw_model_nodes.hpp"
#include "pw_objects.hpp"
#include "util.hpp"

namespace pw {

LinkManager::LinkManager(pw_core*& core,
                         pw_thread_loop*& thread_loop,
                         models::Nodes& model_nodes,
                         std::vector<LinkInfo>& list_links)
    : core(core), thread_loop(thread_loop), model_nodes(model_nodes), list_links(list_links) {}

auto LinkManager::link_info_from_props(const spa_dict* props) -> pw::LinkInfo {
  pw::LinkInfo info;

  util::spa_dict_get_num(props, PW_KEY_LINK_ID, info.id);

  util::spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, info.serial);

  util::spa_dict_get_string(props, PW_KEY_OBJECT_PATH, info.path);

  util::spa_dict_get_num(props, PW_KEY_LINK_INPUT_NODE, info.input_node_id);

  util::spa_dict_get_num(props, PW_KEY_LINK_INPUT_PORT, info.input_port_id);

  util::spa_dict_get_num(props, PW_KEY_LINK_OUTPUT_NODE, info.output_node_id);

  util::spa_dict_get_num(props, PW_KEY_LINK_OUTPUT_PORT, info.output_port_id);

  util::spa_dict_get_bool(props, PW_KEY_LINK_PASSIVE, info.passive);

  return info;
}

auto LinkManager::port_info_from_props(const spa_dict* props) -> pw::PortInfo {
  pw::PortInfo info;

  util::spa_dict_get_num(props, PW_KEY_PORT_ID, info.port_id);

  util::spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, info.serial);

  util::spa_dict_get_string(props, PW_KEY_PORT_NAME, info.name);

  util::spa_dict_get_num(props, PW_KEY_NODE_ID, info.node_id);

  util::spa_dict_get_string(props, PW_KEY_PORT_DIRECTION, info.direction);

  util::spa_dict_get_string(props, PW_KEY_AUDIO_CHANNEL, info.audio_channel);

  util::spa_dict_get_string(props, PW_KEY_AUDIO_FORMAT, info.format_dsp);

  util::spa_dict_get_bool(props, PW_KEY_PORT_PHYSICAL, info.physical);

  util::spa_dict_get_bool(props, PW_KEY_PORT_TERMINAL, info.terminal);

  util::spa_dict_get_bool(props, PW_KEY_PORT_MONITOR, info.monitor);

  return info;
}

auto LinkManager::register_link(pw_registry* registry, uint32_t id, const char* type, const spa_dict* props) -> bool {
  uint64_t serial = 0U;

  if (!util::spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, serial)) {
    util::warning("An error occurred while retrieving the object serial. This link cannot be handled by Easy Effects.");
    return false;
  }

  auto* proxy = static_cast<pw_proxy*>(pw_registry_bind(registry, id, type, PW_VERSION_LINK, sizeof(proxy_data)));

  auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

  pd->proxy = proxy;
  pd->lm = this;
  pd->id = id;
  pd->serial = serial;

  pw_proxy_add_object_listener(proxy, &pd->object_listener, &link_events, pd);  // NOLINT
  pw_proxy_add_listener(proxy, &pd->proxy_listener, &link_proxy_events, pd);

  auto link_info = link_info_from_props(props);

  link_info.id = id;
  link_info.serial = serial;

  list_links.push_back(link_info);

  try {
    const auto input_node = model_nodes.get_node_by_id(link_info.input_node_id);

    const auto output_node = model_nodes.get_node_by_id(link_info.output_node_id);

    util::debug(std::format("{} port {} is connected to {} port {}", output_node.name.toStdString(),
                            link_info.output_port_id, input_node.name.toStdString(), link_info.input_port_id));
  } catch (std::out_of_range& e) {
    util::debug(e.what());
  }

  return true;
}

auto LinkManager::register_port(pw_registry* registry, uint32_t id, const char* type, const spa_dict* props) -> bool {
  uint64_t serial = 0U;

  if (!util::spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, serial)) {
    util::warning("An error occurred while retrieving the object serial. This port cannot be handled by Easy Effects.");
    return false;
  }

  auto* proxy = static_cast<pw_proxy*>(pw_registry_bind(registry, id, type, PW_VERSION_PORT, sizeof(proxy_data)));

  auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

  pd->proxy = proxy;
  pd->lm = this;
  pd->id = id;
  pd->serial = serial;

  pw_proxy_add_listener(proxy, &pd->proxy_listener, &port_proxy_events, pd);

  auto port_info = port_info_from_props(props);

  port_info.id = id;
  port_info.serial = serial;

  // std::cout << port_info.name << "\t" << port_info.audio_channel << "\t" << port_info.direction << "\t"
  //           << port_info.format_dsp << "\t" << port_info.port_id << "\t" << port_info.node_id << std::endl;

  list_ports.push_back(port_info);

  return true;
}

void LinkManager::on_link_info(void* object, const struct pw_link_info* info) {
  auto* const ld = static_cast<proxy_data*>(object);
  auto* const lm = ld->lm;

  pw::LinkInfo link_copy;

  for (auto& l : ld->lm->list_links) {
    if (l.serial == ld->serial) {
      l.state = info->state;

      link_copy = l;

      Q_EMIT lm->linkChanged(link_copy);

      // util::warning(pw_link_state_as_string(l.state));

      break;
    }
  }

  // const struct spa_dict_item* item = nullptr;
  // spa_dict_for_each(item, info->props) printf("\t\t%s: \"%s\"\n", item->key, item->value);
}

void LinkManager::on_removed_proxy(void* data) {
  auto* const pd = static_cast<proxy_data*>(data);

  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  if (pd->object_listener.link.next != nullptr || pd->object_listener.link.prev != nullptr) {
    spa_hook_remove(&pd->object_listener);
  }

  if (pd->proxy != nullptr) {
    pw_proxy_destroy(pd->proxy);
  }
}

void LinkManager::on_destroy_link_proxy(void* data) {
  auto* const ld = static_cast<proxy_data*>(data);

  spa_hook_remove(&ld->proxy_listener);

  auto it = std::ranges::remove_if(ld->lm->list_links, [=](const auto& n) { return n.serial == ld->serial; });

  ld->lm->list_links.erase(it.begin(), it.end());
}

void LinkManager::on_destroy_port_proxy(void* data) {
  auto* const pd = static_cast<proxy_data*>(data);

  spa_hook_remove(&pd->proxy_listener);

  auto it = std::ranges::remove_if(pd->lm->list_ports, [=](const auto& n) { return n.serial == pd->serial; });

  pd->lm->list_ports.erase(it.begin(), it.end());
}

auto LinkManager::get_links() const -> const std::vector<LinkInfo>& {
  return list_links;
}

auto LinkManager::get_ports() const -> const std::vector<PortInfo>& {
  return list_ports;
}

auto LinkManager::link_nodes(const uint& output_node_id,
                             const uint& input_node_id,
                             const bool& probe_link,
                             const bool& link_passive) -> std::vector<pw_proxy*> {
  std::vector<pw_proxy*> list;

  auto output_ports = get_node_ports(output_node_id, "out");
  auto input_ports = get_node_ports(input_node_id, "in");

  if (input_ports.empty()) {
    util::debug(std::format("node {} has no input ports yet. Aborting the link", input_node_id));

    return list;
  }

  if (output_ports.empty()) {
    util::debug(std::format("node {} has no output ports yet. Aborting the link", output_node_id));

    return list;
  }

  auto matching_port_pairs = find_matching_ports(output_ports, input_ports, probe_link);

  for (const auto& [outp, inp] : matching_port_pairs) {
    pw_properties* props = pw_properties_new(nullptr, nullptr);

    pw_properties_set(props, PW_KEY_LINK_PASSIVE, (link_passive) ? "true" : "false");
    pw_properties_set(props, PW_KEY_OBJECT_LINGER, "false");
    pw_properties_set(props, PW_KEY_LINK_OUTPUT_NODE, util::to_string(output_node_id).c_str());
    pw_properties_set(props, PW_KEY_LINK_OUTPUT_PORT, util::to_string(outp.id).c_str());
    pw_properties_set(props, PW_KEY_LINK_INPUT_NODE, util::to_string(input_node_id).c_str());
    pw_properties_set(props, PW_KEY_LINK_INPUT_PORT, util::to_string(inp.id).c_str());

    pw_thread_loop_lock(thread_loop);

    auto* proxy = static_cast<pw_proxy*>(
        pw_core_create_object(core, "link-factory", PW_TYPE_INTERFACE_Link, PW_VERSION_LINK, &props->dict, 0));

    pw_properties_free(props);

    if (proxy == nullptr) {
      pw_thread_loop_unlock(thread_loop);

      util::warning(std::format("Failed to link the node {} to {}", output_node_id, input_node_id));

      return list;
    }

    pw_core_sync(core, PW_ID_CORE, 0);  // NOLINT

    pw_thread_loop_wait(thread_loop);

    pw_thread_loop_unlock(thread_loop);

    list.push_back(proxy);
  }

  return list;
}

void LinkManager::destroy_links(const std::vector<pw_proxy*>& list) {
  for (auto* proxy : list) {
    if (proxy != nullptr) {
      pw_proxy_destroy(proxy);
    }
  }
}

auto LinkManager::count_node_ports(const uint& node_id) const -> uint {
  return std::count_if(list_ports.begin(), list_ports.end(),
                       [node_id](const auto& port) { return port.node_id == node_id; });
}

auto LinkManager::get_node_ports(const uint& node_id, const QString& direction) const -> std::vector<PortInfo> {
  std::vector<PortInfo> result;
  for (const auto& port : list_ports) {
    if (port.node_id == node_id && (direction.isEmpty() || port.direction == direction)) {
      result.push_back(port);
    }
  }
  return result;
}

void LinkManager::update_link_state(uint64_t serial, pw_link_state state) {
  for (auto& link : list_links) {
    if (link.serial == serial) {
      link.state = state;

      Q_EMIT linkChanged(link);

      break;
    }
  }
}

auto LinkManager::find_matching_ports(const std::vector<PortInfo>& output_ports,
                                      const std::vector<PortInfo>& input_ports,
                                      const bool& probe_link) -> std::vector<std::pair<PortInfo, PortInfo>> {
  std::vector<std::pair<PortInfo, PortInfo>> matches;
  bool use_audio_channel = true;

  // Determine if we should use audio channel matching
  if (!probe_link) {
    for (const auto& port : output_ports) {
      if (port.audio_channel != "FL" && port.audio_channel != "FR") {
        use_audio_channel = false;
        break;
      }
    }
    for (const auto& port : input_ports) {
      if (port.audio_channel != "FL" && port.audio_channel != "FR") {
        use_audio_channel = false;
        break;
      }
    }
  }

  for (const auto& outp : output_ports) {
    for (const auto& inp : input_ports) {
      bool ports_match = false;

      if (!probe_link) {
        if (use_audio_channel) {
          ports_match = outp.audio_channel == inp.audio_channel;
        } else {
          ports_match = outp.port_id == inp.port_id;
        }
      } else {
        if ((outp.audio_channel == "FL" && inp.audio_channel == "PROBE_FL") ||
            (outp.audio_channel == "FR" && inp.audio_channel == "PROBE_FR")) {
          ports_match = true;
        }
      }

      if (ports_match) {
        matches.emplace_back(outp, inp);
      }
    }
  }

  return matches;
}

}  // namespace pw
