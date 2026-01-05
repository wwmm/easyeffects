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

#include "pw_device_manager.hpp"

// For some reason our flatpak CI needs this header before <pipewire/module.h>
#include <pipewire/type.h>

#include <pipewire/context.h>
#include <pipewire/core.h>
#include <pipewire/device.h>
#include <pipewire/keys.h>
#include <pipewire/properties.h>
#include <pipewire/proxy.h>
#include <pipewire/thread-loop.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <spa/monitor/device.h>
#include <spa/param/param.h>
#include <spa/param/profile.h>
#include <spa/param/route.h>
#include <spa/pod/parser.h>
#include <spa/pod/pod.h>
#include <spa/pod/vararg.h>
#include <spa/utils/defs.h>
#include <spa/utils/dict.h>
#include <spa/utils/hook.h>
#include <spa/utils/keys.h>
#include <spa/utils/type.h>
#include <algorithm>
#include <cstdint>
#include <format>
#include <span>
#include <vector>
#include "pw_objects.hpp"
#include "util.hpp"

namespace pw {

DeviceManager::DeviceManager(std::vector<DeviceInfo>& list_devices) : list_devices(list_devices) {}

auto DeviceManager::register_device(pw_registry* registry, uint32_t id, const char* type, const spa_dict* props)
    -> bool {
  QString media_class;

  if (const auto* key_media_class = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS)) {
    media_class = key_media_class;
  }

  uint64_t serial = 0U;

  if (!util::spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, serial)) {
    util::warning(
        "An error occurred while converting the object serial. This device cannot be handled by Easy Effects.");
    return false;
  }

  auto* proxy = static_cast<pw_proxy*>(pw_registry_bind(registry, id, type, PW_VERSION_DEVICE, sizeof(proxy_data)));

  auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

  pd->proxy = proxy;
  pd->dm = this;
  pd->id = id;
  pd->serial = serial;

  pw_proxy_add_object_listener(proxy, &pd->object_listener, &device_events, pd);
  pw_proxy_add_listener(proxy, &pd->proxy_listener, &device_proxy_events, pd);

  DeviceInfo d_info;

  d_info.id = id;
  d_info.serial = serial;
  d_info.media_class = media_class;

  list_devices.push_back(d_info);

  return true;
}

void DeviceManager::on_device_info(void* object, const struct pw_device_info* info) {
  auto* const dd = static_cast<proxy_data*>(object);

  for (auto& device : dd->dm->list_devices) {
    if (device.id != info->id) {
      continue;
    }

    util::spa_dict_get_string(info->props, PW_KEY_DEVICE_NAME, device.name);

    util::spa_dict_get_string(info->props, PW_KEY_DEVICE_NICK, device.nick);

    util::spa_dict_get_string(info->props, PW_KEY_DEVICE_DESCRIPTION, device.description);

    util::spa_dict_get_string(info->props, PW_KEY_DEVICE_API, device.api);

    if (util::spa_dict_get_string(info->props, SPA_KEY_DEVICE_BUS_ID, device.bus_id)) {
      std::ranges::replace(device.bus_id, ':', '_');
      std::ranges::replace(device.bus_id, '+', '_');
    }

    if (util::spa_dict_get_string(info->props, PW_KEY_DEVICE_BUS_PATH, device.bus_path)) {
      std::ranges::replace(device.bus_path, ':', '_');
      std::ranges::replace(device.bus_path, '+', '_');
    }

    /**
     * For some reason bluez5 devices do not define bus-path or bus-id.
     * So as a workaround we set SPA_KEY_API_BLUEZ5_ADDRESS as bus_path
     */

    if (device.api == "bluez5") {
      if (util::spa_dict_get_string(info->props, SPA_KEY_API_BLUEZ5_ADDRESS, device.bus_path)) {
        std::ranges::replace(device.bus_path, ':', '_');
      }
    }

    if ((info->change_mask & PW_DEVICE_CHANGE_MASK_PARAMS) != 0U) {
      auto params = std::span(info->params, info->n_params);

      for (auto param : params) {
        if ((param.flags & SPA_PARAM_INFO_READ) == 0U) {
          continue;
        }

        if (const auto id = param.id; id == SPA_PARAM_Route) {
          pw_device_enum_params((struct pw_device*)dd->proxy, 0, id, 0, -1, nullptr);
        }
      }
    }

    break;
  }
}

void DeviceManager::on_device_event_param(void* object,
                                          [[maybe_unused]] int seq,
                                          uint32_t id,
                                          [[maybe_unused]] uint32_t index,
                                          [[maybe_unused]] uint32_t next,
                                          const struct spa_pod* param) {
  if (id != SPA_PARAM_Route) {
    return;
  }

  auto* const dd = static_cast<proxy_data*>(object);

  const char* name = nullptr;
  const char* description = nullptr;

  enum spa_direction direction {};
  enum spa_param_availability available {};

  if (spa_pod_parse_object(param, SPA_TYPE_OBJECT_ParamRoute, nullptr, SPA_PARAM_ROUTE_direction,
                           SPA_POD_Id(&direction), SPA_PARAM_ROUTE_name, SPA_POD_String(&name),
                           SPA_PARAM_ROUTE_description, SPA_POD_String(&description), SPA_PARAM_ROUTE_available,
                           SPA_POD_Id(&available)) < 0) {
    return;
  }

  if (name == nullptr) {
    return;
  }

  if (description == nullptr) {
    description = name;
  }

  for (auto& device : dd->dm->list_devices) {
    if (device.id != dd->id) {
      continue;
    }

    if (direction == SPA_DIRECTION_INPUT) {
      if (name != device.input_route_name || available != device.input_route_available) {
        device.input_route_name = name;
        device.input_route_description = description;
        device.input_route_available = available;

        Q_EMIT dd->dm->inputRouteChanged(device);

        util::debug(std::format("new {} input route: {}", device.description.toStdString(),
                                device.input_route_description.toStdString()));
      }
    } else if (direction == SPA_DIRECTION_OUTPUT) {
      if (name != device.output_route_name || available != device.output_route_available) {
        device.output_route_name = name;
        device.output_route_description = description;
        device.output_route_available = available;

        Q_EMIT dd->dm->outputRouteChanged(device);

        util::debug(std::format("new {} output route: {}", device.description.toStdString(),
                                device.output_route_description.toStdString()));
      }
    }

    break;
  }
}

void DeviceManager::on_removed_proxy(void* data) {
  auto* const pd = static_cast<proxy_data*>(data);

  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  if (pd->object_listener.link.next != nullptr || pd->object_listener.link.prev != nullptr) {
    spa_hook_remove(&pd->object_listener);
  }

  if (pd->proxy != nullptr) {
    pw_proxy_destroy(pd->proxy);
  }
}

void DeviceManager::on_destroy_device_proxy(void* data) {
  auto* const dd = static_cast<proxy_data*>(data);

  spa_hook_remove(&dd->proxy_listener);

  auto it = std::ranges::remove_if(dd->dm->list_devices, [=](const auto& n) { return n.serial == dd->serial; });

  dd->dm->list_devices.erase(it.begin(), it.end());
}

}  // namespace pw
