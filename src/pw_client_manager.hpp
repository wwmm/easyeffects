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

#include <pipewire/client.h>
#include <pipewire/core.h>
#include <pipewire/proxy.h>
#include <pipewire/thread-loop.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <spa/utils/defs.h>
#include <spa/utils/hook.h>
#include <cstdint>
#include "pw_model_clients.hpp"

namespace pw {

class ClientManager : public QObject {
  Q_OBJECT

 public:
  explicit ClientManager(pw_core*& core, pw_thread_loop*& thread_loop, models::Clients& model_clients);
  ~ClientManager() override = default;

  ClientManager(const ClientManager&) = delete;
  auto operator=(const ClientManager&) -> ClientManager& = delete;
  ClientManager(const ClientManager&&) = delete;
  auto operator=(const ClientManager&&) -> ClientManager& = delete;

  auto register_client(pw_registry* registry, uint32_t id, const char* type, const spa_dict* props) -> bool;

 private:
  struct proxy_data {
    pw_proxy* proxy = nullptr;

    spa_hook proxy_listener{};

    spa_hook object_listener{};

    ClientManager* cm = nullptr;

    uint id = SPA_ID_INVALID;

    uint64_t serial = SPA_ID_INVALID;
  };

  pw_core*& core;
  pw_thread_loop*& thread_loop;

  models::Clients& model_clients;

  const struct pw_proxy_events client_proxy_events = {.version = 0,
                                                      .destroy = on_destroy_client_proxy,
                                                      .bound = nullptr,
                                                      .removed = on_removed_proxy,
                                                      .done = nullptr,
                                                      .error = nullptr,
                                                      .bound_props = nullptr};

  const struct pw_client_events client_events = {.version = 0, .info = on_client_info, .permissions = nullptr};

  static void on_client_info(void* object, const struct pw_client_info* info);

  static void on_removed_proxy(void* data);

  static void on_destroy_client_proxy(void* data);
};

}  // namespace pw
