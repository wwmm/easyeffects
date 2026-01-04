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
#include <pipewire/device.h>
#include <pipewire/proxy.h>
#include <pipewire/thread-loop.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <spa/pod/pod.h>
#include <spa/utils/defs.h>
#include <spa/utils/hook.h>
#include <cstdint>
#include <vector>
#include "pw_objects.hpp"

namespace pw {

class DeviceManager : public QObject {
  Q_OBJECT

 public:
  explicit DeviceManager(std::vector<DeviceInfo>& list_devices);
  ~DeviceManager() override = default;

  DeviceManager(const DeviceManager&) = delete;
  auto operator=(const DeviceManager&) -> DeviceManager& = delete;
  DeviceManager(const DeviceManager&&) = delete;
  auto operator=(const DeviceManager&&) -> DeviceManager& = delete;

  auto register_device(pw_registry* registry, uint32_t id, const char* type, const spa_dict* props) -> bool;

 Q_SIGNALS:
  /**
   * IMPORTANT
   * Do not pass DeviceInfo by reference. Sometimes it dies before we use it
   * and a segmentation fault happens.
   */

  void inputRouteChanged(DeviceInfo device);
  void outputRouteChanged(DeviceInfo device);

 private:
  struct proxy_data {
    pw_proxy* proxy = nullptr;

    spa_hook proxy_listener{};

    spa_hook object_listener{};

    DeviceManager* dm = nullptr;

    uint id = SPA_ID_INVALID;

    uint64_t serial = SPA_ID_INVALID;
  };

  std::vector<DeviceInfo>& list_devices;

  const struct pw_proxy_events device_proxy_events = {.version = 0,
                                                      .destroy = on_destroy_device_proxy,
                                                      .bound = nullptr,
                                                      .removed = on_removed_proxy,
                                                      .done = nullptr,
                                                      .error = nullptr,
                                                      .bound_props = nullptr};

  const struct pw_device_events device_events = {.version = 0, .info = on_device_info, .param = on_device_event_param};

  static void on_device_info(void* object, const struct pw_device_info* info);

  static void on_device_event_param(void* object,
                                    int seq,
                                    uint32_t id,
                                    uint32_t index,
                                    uint32_t next,
                                    const struct spa_pod* param);

  static void on_removed_proxy(void* data);

  static void on_destroy_device_proxy(void* data);

  static auto device_info_from_props(const spa_dict* props) -> pw::DeviceInfo;
};

}  // namespace pw
