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

#include <lilv/lilv.h>
#include <lv2/lv2plug.in/ns/ext/log/log.h>
#include <lv2/ui/ui.h>
#include <lv2/urid/urid.h>
#include <sys/types.h>
#include <mutex>

namespace lv2 {

class Lv2Wrapper;  // forward declaration

class NativeUi {
 public:
  explicit NativeUi(Lv2Wrapper* wrapper);
  ~NativeUi();

  void load();
  void notify();
  void update();
  void close();
  void port_event(uint port_index, float value);
  void sync_to_database();

  [[nodiscard]] bool has_ui() const;

 private:
  Lv2Wrapper* wrapper;

  LV2UI_Handle ui_handle = nullptr;

  LV2_Log_Log lv2_log;
  LV2_URID_Map lv2_map;
  LV2_URID_Unmap lv2_unmap;

  const LV2UI_Descriptor* ui_descriptor = nullptr;
  const LV2UI_Idle_Interface* idle_iface = nullptr;
  const LV2UI_Show_Interface* show_iface = nullptr;

  void* libhandle = nullptr;

  std::mutex ui_mutex;
};

}  // namespace lv2
