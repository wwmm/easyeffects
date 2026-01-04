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

#include "lv2_ui.hpp"
#include <dlfcn.h>
#include <lv2/atom/atom.h>
#include <lv2/buf-size/buf-size.h>
#include <lv2/core/lv2.h>
#include <lv2/data-access/data-access.h>
#include <lv2/instance-access/instance-access.h>
#include <lv2/lv2plug.in/ns/ext/log/log.h>
#include <lv2/options/options.h>
#include <lv2/parameters/parameters.h>
#include <lv2/ui/ui.h>
#include <lv2/urid/urid.h>
#include <qtypes.h>
#include <array>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <format>
#include <mutex>
#include <string>
#include "lilv/lilv.h"
#include "lv2_wrapper.hpp"
#include "util.hpp"

namespace lv2 {

NativeUi::NativeUi(Lv2Wrapper* wrapper) : wrapper(wrapper) {
  lv2_log = {.handle = wrapper,
             .printf = &Lv2Wrapper::lv2_printf,
             .vprintf = []([[maybe_unused]] LV2_Log_Handle handle, [[maybe_unused]] LV2_URID type, const char* fmt,
                           va_list ap) { return std::vprintf(fmt, ap); }};

  lv2_map = {.handle = wrapper, .map = [](LV2_URID_Map_Handle handle, const char* uri) {
               auto* lw = static_cast<Lv2Wrapper*>(handle);

               return lw->map_urid(uri);
             }};

  lv2_unmap = {.handle = wrapper, .unmap = [](LV2_URID_Unmap_Handle handle, LV2_URID urid) {
                 auto* lw = static_cast<Lv2Wrapper*>(handle);

                 return lw->map_urid_to_uri[urid].c_str();
               }};
}

NativeUi::~NativeUi() {
  close();
}

void NativeUi::load() {
  std::scoped_lock<std::mutex> lku(ui_mutex);

  if (!wrapper->has_instance()) {
    return;
  }

  LilvUIs* uis = lilv_plugin_get_uis(wrapper->get_lilv_plugin());

  if (!uis) {
    return;
  }

  LILV_FOREACH(uis, u, uis) {
    const LilvUI* ui = lilv_uis_get(uis, u);

    std::string ui_uri = lilv_node_as_uri(lilv_ui_get_uri(ui));

    util::debug(std::format("{} UI uri: {}", wrapper->get_plugin_uri(), ui_uri));

    const LilvNode* binary_node = lilv_ui_get_binary_uri(ui);
    const LilvNode* bundle_node = lilv_ui_get_bundle_uri(ui);

    auto path = lilv_file_uri_parse(lilv_node_as_uri(binary_node), nullptr);

    libhandle = dlopen(path, RTLD_NOW);

    lilv_free(path);

    if (!libhandle) {
      continue;
    }

    auto descfn = reinterpret_cast<LV2UI_DescriptorFunction>(dlsym(libhandle, "lv2ui_descriptor"));

    if (!descfn) {
      dlclose(libhandle);
      libhandle = nullptr;
      continue;
    }

    uint32_t index = 0U;

    while ((ui_descriptor = descfn(index++)) != nullptr) {
      if (ui_descriptor->URI != ui_uri) {
        continue;
      }

      idle_iface = static_cast<const LV2UI_Idle_Interface*>(ui_descriptor->extension_data(LV2_UI__idleInterface));
      show_iface = static_cast<const LV2UI_Show_Interface*>(ui_descriptor->extension_data(LV2_UI__showInterface));

      if (!idle_iface) {
        continue;
      }

      const LV2_Feature lv2_log_feature = {LV2_LOG__log, &lv2_log};

      const LV2_Feature lv2_map_feature = {LV2_URID__map, &lv2_map};

      const LV2_Feature lv2_unmap_feature = {LV2_URID__unmap, &lv2_unmap};

      const auto rate = wrapper->get_rate();
      const auto n_samples = wrapper->get_n_samples();

      auto options = std::to_array<LV2_Options_Option>(
          {{.context = LV2_OPTIONS_INSTANCE,
            .subject = 0,
            .key = wrapper->map_urid(LV2_PARAMETERS__sampleRate),
            .size = sizeof(float),
            .type = wrapper->map_urid(LV2_ATOM__Float),
            .value = &rate},
           {.context = LV2_OPTIONS_INSTANCE,
            .subject = 0,
            .key = wrapper->map_urid(LV2_BUF_SIZE__minBlockLength),
            .size = sizeof(int32_t),
            .type = wrapper->map_urid(LV2_ATOM__Int),
            .value = &lv2::Lv2Wrapper::min_quantum},
           {.context = LV2_OPTIONS_INSTANCE,
            .subject = 0,
            .key = wrapper->map_urid(LV2_BUF_SIZE__maxBlockLength),
            .size = sizeof(int32_t),
            .type = wrapper->map_urid(LV2_ATOM__Int),
            .value = &lv2::Lv2Wrapper::max_quantum},
           {.context = LV2_OPTIONS_INSTANCE,
            .subject = 0,
            .key = wrapper->map_urid(LV2_BUF_SIZE__nominalBlockLength),
            .size = sizeof(int32_t),
            .type = wrapper->map_urid(LV2_ATOM__Int),
            .value = &n_samples},
           {.context = LV2_OPTIONS_INSTANCE, .subject = 0, .key = 0, .size = 0, .type = 0, .value = nullptr}});

      LV2_Feature feature_options = {.URI = LV2_OPTIONS__options, .data = options.data()};

      LV2_Extension_Data_Feature extension_data = {wrapper->get_instance()->lv2_descriptor->extension_data};

      const LV2_Feature feature_dataAccess = {LV2_DATA_ACCESS_URI, &extension_data};

      const LV2_Feature feature_instAccess = {LV2_INSTANCE_ACCESS_URI, wrapper->get_instance()->lv2_handle};

      const LV2_Feature idle_feature = {LV2_UI__idleInterface, nullptr};

      const LV2_Feature parent_feature = {LV2_UI__parent, nullptr};

      const LV2_Feature fixed_size_feature = {LV2_UI__fixedSize, nullptr};

      const LV2_Feature no_user_resize_feature = {LV2_UI__noUserResize, nullptr};

      const LV2_Feature make_resident_feature = {LV2_UI_makeResident, nullptr};

      const LV2_Feature make_soname_resident_feature = {LV2_UI_makeSONameResident, nullptr};

      const auto features = std::to_array<const LV2_Feature*>(
          {&lv2_log_feature, &lv2_map_feature, &lv2_unmap_feature, &feature_options, wrapper->static_features.data(),
           &feature_dataAccess, &feature_instAccess, &idle_feature, &parent_feature, &fixed_size_feature,
           &no_user_resize_feature, &make_resident_feature, &make_soname_resident_feature, nullptr});

      LV2UI_Widget widget = nullptr;

      auto bundle_path = lilv_file_uri_parse(lilv_node_as_uri(bundle_node), nullptr);

      ui_handle = ui_descriptor->instantiate(
          ui_descriptor, wrapper->get_plugin_uri().c_str(), bundle_path,
          +[](LV2UI_Controller controller, uint32_t port_index, uint32_t, uint32_t port_protocol, const void* buffer) {
            auto wrapper = static_cast<Lv2Wrapper*>(controller);

            for (auto& p : wrapper->ports) {
              if (p.index == port_index && port_protocol == 0) {
                p.value = *static_cast<const float*>(buffer);
              }
            }
          },
          wrapper, &widget, features.data());

      lilv_free(bundle_path);

      if (ui_handle && show_iface && show_iface->show(ui_handle) != 0) {
        util::warning(std::format("Failed to show UI for {}", ui_uri));
      }

      break;
    }
  }

  lilv_uis_free(uis);

  // Initialize UI with current control values
  if (ui_descriptor && ui_handle) {
    for (const auto& p : wrapper->ports) {
      if (p.type == PortType::TYPE_CONTROL) {
        ui_descriptor->port_event(ui_handle, p.index, sizeof(float), 0, &p.value);
      }
    }
  }
}

void NativeUi::notify() {
  if (!ui_descriptor || !ui_handle || !ui_descriptor->port_event) {
    return;
  }

  std::scoped_lock<std::mutex> lk(ui_mutex);

  for (const auto& p : wrapper->ports) {
    if (p.type == PortType::TYPE_CONTROL && !p.is_input) {
      ui_descriptor->port_event(ui_handle, p.index, sizeof(float), 0, &p.value);
    }
  }
}

void NativeUi::update() {
  if (!idle_iface || !ui_handle) {
    return;
  }

  std::scoped_lock<std::mutex> lk(ui_mutex);

  idle_iface->idle(ui_handle);
}

void NativeUi::close() {
  std::scoped_lock<std::mutex> lk(ui_mutex);

  if (ui_descriptor && ui_handle) {
    if (show_iface) {
      show_iface->hide(ui_handle);
    }

    ui_descriptor->cleanup(ui_handle);
  }

  if (libhandle) {
    dlclose(libhandle);
  }

  ui_handle = nullptr;
  ui_descriptor = nullptr;
  libhandle = nullptr;
  show_iface = nullptr;
}

void NativeUi::port_event(uint port_index, float value) {
  if (!ui_descriptor || !ui_handle) {
    return;
  }

  ui_descriptor->port_event(ui_handle, port_index, sizeof(float), 0, &value);
}

void NativeUi::sync_to_database() {
  if (!ui_descriptor || !ui_handle) {
    return;
  }

  for (const auto& func : wrapper->sync_funcs) {
    func();
  }
}

bool NativeUi::has_ui() const {
  return ui_handle != nullptr;
}

}  // namespace lv2
