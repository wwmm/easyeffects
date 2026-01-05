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

#include "pw_client_manager.hpp"

// For some reason our flatpak CI needs this header before <pipewire/client.h>
#include <pipewire/type.h>

#include <pipewire/client.h>
#include <pipewire/context.h>
#include <pipewire/core.h>
#include <pipewire/keys.h>
#include <pipewire/properties.h>
#include <pipewire/proxy.h>
#include <pipewire/thread-loop.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <spa/utils/hook.h>
#include <cstdint>
#include <string>
#include "pw_model_clients.hpp"
#include "pw_objects.hpp"
#include "util.hpp"

namespace pw {

ClientManager::ClientManager(pw_core*& core, pw_thread_loop*& thread_loop, models::Clients& model_clients)
    : core(core), thread_loop(thread_loop), model_clients(model_clients) {}

auto ClientManager::register_client(pw_registry* registry, uint32_t id, const char* type, const spa_dict* props)
    -> bool {
  uint64_t serial = 0U;

  if (!util::spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, serial)) {
    util::warning(
        "An error occurred while retrieving the object serial. This client cannot be handled by Easy Effects.");
    return false;
  }

  auto* proxy = static_cast<pw_proxy*>(pw_registry_bind(registry, id, type, PW_VERSION_CLIENT, sizeof(proxy_data)));

  auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

  pd->proxy = proxy;
  pd->cm = this;
  pd->id = id;
  pd->serial = serial;

  pw_proxy_add_object_listener(proxy, &pd->object_listener, &client_events, pd);  // NOLINT
  pw_proxy_add_listener(proxy, &pd->proxy_listener, &client_proxy_events, pd);

  pw::ClientInfo c_info{.id = id, .serial = serial, .name = "", .access = "", .api = ""};

  model_clients.append(c_info);

  return true;
}

void ClientManager::on_client_info(void* object, const struct pw_client_info* info) {
  auto* const cd = static_cast<proxy_data*>(object);

  auto list = cd->cm->model_clients.get_list();

  for (int n = 0; n < list.size(); n++) {
    if (list[n].id == info->id && info->props != nullptr) {
      QString name;
      QString access;
      QString api;

      util::spa_dict_get_string(info->props, PW_KEY_APP_NAME, name);

      util::spa_dict_get_string(info->props, PW_KEY_ACCESS, access);

      util::spa_dict_get_string(info->props, PW_KEY_CLIENT_API, api);

      cd->cm->model_clients.update_field(n, pw::models::Clients::Roles::Name, name);
      cd->cm->model_clients.update_field(n, pw::models::Clients::Roles::Access, access);
      cd->cm->model_clients.update_field(n, pw::models::Clients::Roles::Api, api);

      break;
    }
  }
}

void ClientManager::on_removed_proxy(void* data) {
  auto* const pd = static_cast<proxy_data*>(data);

  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  if (pd->object_listener.link.next != nullptr || pd->object_listener.link.prev != nullptr) {
    spa_hook_remove(&pd->object_listener);
  }

  if (pd->proxy != nullptr) {
    pw_proxy_destroy(pd->proxy);
  }
}

void ClientManager::on_destroy_client_proxy(void* data) {
  auto* const cd = static_cast<proxy_data*>(data);

  spa_hook_remove(&cd->proxy_listener);

  cd->cm->model_clients.remove_by_id(cd->id);
}

}  // namespace pw
