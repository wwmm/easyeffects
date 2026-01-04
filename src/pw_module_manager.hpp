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
#include <pipewire/module.h>
#include <pipewire/proxy.h>
#include <pipewire/thread-loop.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <spa/utils/defs.h>
#include <spa/utils/hook.h>
#include <cstdint>
#include "pw_model_modules.hpp"
#include "pw_objects.hpp"

namespace pw {

class ModuleManager : public QObject {
  Q_OBJECT

 public:
  explicit ModuleManager(pw_core*& core, pw_thread_loop*& thread_loop, models::Modules& model_modules);
  ~ModuleManager() override = default;

  ModuleManager(const ModuleManager&) = delete;
  auto operator=(const ModuleManager&) -> ModuleManager& = delete;
  ModuleManager(const ModuleManager&&) = delete;
  auto operator=(const ModuleManager&&) -> ModuleManager& = delete;

  auto register_module(pw_registry* registry, uint32_t id, const char* type, const spa_dict* props) -> bool;

 private:
  struct proxy_data {
    pw_proxy* proxy = nullptr;

    spa_hook proxy_listener{};

    spa_hook object_listener{};

    ModuleManager* mm = nullptr;

    uint id = SPA_ID_INVALID;

    uint64_t serial = SPA_ID_INVALID;
  };

  pw_core*& core;
  pw_thread_loop*& thread_loop;

  models::Modules& model_modules;

  const struct pw_proxy_events module_proxy_events = {.version = 0,
                                                      .destroy = on_destroy_module_proxy,
                                                      .bound = nullptr,
                                                      .removed = on_removed_proxy,
                                                      .done = nullptr,
                                                      .error = nullptr,
                                                      .bound_props = nullptr};

  const struct pw_module_events module_events = {
      .version = 0,
      .info = on_module_info,
  };

  static void on_module_info(void* object, const struct pw_module_info* info);

  static void on_removed_proxy(void* data);

  static void on_destroy_module_proxy(void* data);

  static auto module_info_from_props(const spa_dict* props) -> pw::ModuleInfo;
};

}  // namespace pw
