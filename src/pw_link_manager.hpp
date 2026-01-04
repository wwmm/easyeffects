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

#pragma once

#include <pipewire/core.h>
#include <pipewire/link.h>
#include <pipewire/proxy.h>
#include <pipewire/thread-loop.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <spa/utils/defs.h>
#include <spa/utils/hook.h>
#include <cstdint>
#include <utility>
#include <vector>
#include "pw_model_nodes.hpp"
#include "pw_objects.hpp"

namespace pw {

class LinkManager : public QObject {
  Q_OBJECT

 public:
  explicit LinkManager(pw_core*& core,
                       pw_thread_loop*& thread_loop,
                       models::Nodes& model_nodes,
                       std::vector<LinkInfo>& list_links);
  ~LinkManager() override = default;

  LinkManager(const LinkManager&) = delete;
  auto operator=(const LinkManager&) -> LinkManager& = delete;
  LinkManager(const LinkManager&&) = delete;
  auto operator=(const LinkManager&&) -> LinkManager& = delete;

  auto register_link(pw_registry* registry, uint32_t id, const char* type, const spa_dict* props) -> bool;

  auto register_port(pw_registry* registry, uint32_t id, const char* type, const spa_dict* props) -> bool;

  auto link_nodes(const uint& output_node_id,
                  const uint& input_node_id,
                  const bool& probe_link = false,
                  const bool& link_passive = true) -> std::vector<pw_proxy*>;

  static void destroy_links(const std::vector<pw_proxy*>& list);

  [[nodiscard]] auto get_links() const -> const std::vector<LinkInfo>&;

  [[nodiscard]] auto get_ports() const -> const std::vector<PortInfo>&;

  [[nodiscard]] auto count_node_ports(const uint& node_id) const -> uint;

  [[nodiscard]] auto get_node_ports(const uint& node_id, const QString& direction = "") const -> std::vector<PortInfo>;

  void update_link_state(uint64_t serial, pw_link_state state);

 Q_SIGNALS:
  void linkChanged(LinkInfo link);

 private:
  struct proxy_data {
    pw_proxy* proxy = nullptr;

    spa_hook proxy_listener{};

    spa_hook object_listener{};

    LinkManager* lm = nullptr;

    uint id = SPA_ID_INVALID;

    uint64_t serial = SPA_ID_INVALID;
  };

  pw_core*& core;

  pw_thread_loop*& thread_loop;

  models::Nodes& model_nodes;

  std::vector<LinkInfo>& list_links;

  std::vector<PortInfo> list_ports;

  const struct pw_proxy_events link_proxy_events = {.version = 0,
                                                    .destroy = on_destroy_link_proxy,
                                                    .bound = nullptr,
                                                    .removed = on_removed_proxy,
                                                    .done = nullptr,
                                                    .error = nullptr,
                                                    .bound_props = nullptr};

  const struct pw_link_events link_events = {
      .version = 0,
      .info = on_link_info,
  };

  const struct pw_proxy_events port_proxy_events = {.version = 0,
                                                    .destroy = on_destroy_port_proxy,
                                                    .bound = nullptr,
                                                    .removed = on_removed_proxy,
                                                    .done = nullptr,
                                                    .error = nullptr,
                                                    .bound_props = nullptr};

  static auto link_info_from_props(const spa_dict* props) -> pw::LinkInfo;

  static auto port_info_from_props(const spa_dict* props) -> pw::PortInfo;

  static void on_link_info(void* object, const struct pw_link_info* info);

  static void on_removed_proxy(void* data);

  static void on_destroy_link_proxy(void* data);

  static void on_destroy_port_proxy(void* data);

  [[nodiscard]] static auto find_matching_ports(const std::vector<PortInfo>& output_ports,
                                                const std::vector<PortInfo>& input_ports,
                                                const bool& probe_link) -> std::vector<std::pair<PortInfo, PortInfo>>;
};

}  // namespace pw
