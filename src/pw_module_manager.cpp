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

#include "pw_module_manager.hpp"

// For some reason our flatpak CI needs this header before <pipewire/module.h>
#include <pipewire/type.h>

#include <pipewire/context.h>
#include <pipewire/core.h>
#include <pipewire/keys.h>
#include <pipewire/module.h>
#include <pipewire/properties.h>
#include <pipewire/proxy.h>
#include <pipewire/thread-loop.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <spa/utils/hook.h>
#include <cstdint>
#include <string>
#include "pw_model_modules.hpp"
#include "pw_objects.hpp"
#include "util.hpp"

namespace pw {

ModuleManager::ModuleManager(pw_core*& core, pw_thread_loop*& thread_loop, models::Modules& model_modules)
    : core(core), thread_loop(thread_loop), model_modules(model_modules) {}

auto ModuleManager::register_module(pw_registry* registry, uint32_t id, const char* type, const spa_dict* props)
    -> bool {
  uint64_t serial = 0U;

  if (!util::spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, serial)) {
    util::warning(
        "An error occurred while retrieving the object serial. This module cannot be handled by Easy Effects.");
    return false;
  }

  auto* proxy = static_cast<pw_proxy*>(pw_registry_bind(registry, id, type, PW_VERSION_MODULE, sizeof(proxy_data)));

  auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

  pd->proxy = proxy;
  pd->mm = this;
  pd->id = id;
  pd->serial = serial;

  pw_proxy_add_object_listener(proxy, &pd->object_listener, &module_events, pd);  // NOLINT
  pw_proxy_add_listener(proxy, &pd->proxy_listener, &module_proxy_events, pd);

  pw::ModuleInfo m_info{.id = id, .serial = serial, .name = "", .description = "", .filename = "", .version = ""};

  util::spa_dict_get_string(props, PW_KEY_MODULE_NAME, m_info.name);

  model_modules.append(m_info);

  return true;
}

void ModuleManager::on_module_info(void* object, const struct pw_module_info* info) {
  auto* const md = static_cast<proxy_data*>(object);

  auto list = md->mm->model_modules.get_list();

  for (int n = 0; n < list.size(); n++) {
    if (list[n].id == info->id) {
      if (info->props != nullptr) {
        QString description;

        util::spa_dict_get_string(info->props, PW_KEY_MODULE_DESCRIPTION, description);

        md->mm->model_modules.update_field(n, pw::models::Modules::Roles::Description, description);

        QString version;

        util::spa_dict_get_string(info->props, PW_KEY_MODULE_VERSION, version);

        md->mm->model_modules.update_field(n, pw::models::Modules::Roles::Version, version);
      }

      if (info->filename != nullptr) {
        md->mm->model_modules.update_field(n, pw::models::Modules::Roles::Filename, info->filename);
      }

      break;
    }
  }
}

void ModuleManager::on_removed_proxy(void* data) {
  auto* const pd = static_cast<proxy_data*>(data);

  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  if (pd->object_listener.link.next != nullptr || pd->object_listener.link.prev != nullptr) {
    spa_hook_remove(&pd->object_listener);
  }

  if (pd->proxy != nullptr) {
    pw_proxy_destroy(pd->proxy);
  }
}

void ModuleManager::on_destroy_module_proxy(void* data) {
  auto* const md = static_cast<proxy_data*>(data);

  spa_hook_remove(&md->proxy_listener);

  md->mm->model_modules.remove_by_id(md->id);
}

}  // namespace pw
