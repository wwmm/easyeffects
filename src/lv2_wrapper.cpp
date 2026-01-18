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

#include "lv2_wrapper.hpp"
#include <dlfcn.h>
#include <lilv/lilv.h>
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
#include <sys/types.h>
#include <array>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <format>
#include <functional>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "util.hpp"

namespace lv2 {

Lv2Wrapper::Lv2Wrapper(const std::string& plugin_uri)
    : plugin_uri(plugin_uri), world(lilv_world_new()), native_ui(this) {
  if (world == nullptr) {
    util::warning("Failed to initialized the world");

    return;
  }

  auto* const uri = lilv_new_uri(world, plugin_uri.c_str());

  if (uri == nullptr) {
    util::warning(std::format("Invalid plugin URI: {}", plugin_uri));

    return;
  }

  lilv_world_load_all(world);

  const LilvPlugins* plugins = lilv_world_get_all_plugins(world);

  plugin = lilv_plugins_get_by_uri(plugins, uri);

  lilv_node_free(uri);

  if (plugin == nullptr) {
    util::warning(std::format("Could not find the plugin: {}", plugin_uri));

    return;
  }

  found_plugin = true;

  check_required_features();

  create_ports();
}

Lv2Wrapper::~Lv2Wrapper() {
  if (instance != nullptr) {
    lilv_instance_deactivate(instance);
    lilv_instance_free(instance);

    instance = nullptr;
  }

  if (world != nullptr) {
    lilv_world_free(world);
  }
}

auto Lv2Wrapper::lv2_printf([[maybe_unused]] LV2_Log_Handle handle,
                            [[maybe_unused]] LV2_URID type,
                            const char* format,
                            ...) -> int {
  va_list args;

  va_start(args, format);

  int r = std::vprintf(format, args);

  va_end(args);

  return r;
}

void Lv2Wrapper::check_required_features() {
  LilvNodes* required_features = lilv_plugin_get_required_features(plugin);

  if (required_features != nullptr) {
    for (auto* i = lilv_nodes_begin(required_features); !lilv_nodes_is_end(required_features, i);
         i = lilv_nodes_next(required_features, i)) {
      const LilvNode* required_feature = lilv_nodes_get(required_features, i);

      const char* required_feature_uri = lilv_node_as_uri(required_feature);

      util::debug(std::format("{} requires feature: {}", plugin_uri, required_feature_uri));
    }

    lilv_nodes_free(required_features);
  }
}

void Lv2Wrapper::create_ports() {
  n_ports = lilv_plugin_get_num_ports(plugin);

  ports.resize(n_ports);

  // Get min, max and default values for all ports

  std::vector<float> values(n_ports);
  std::vector<float> minimum(n_ports);
  std::vector<float> maximum(n_ports);

  lilv_plugin_get_port_ranges_float(plugin, minimum.data(), maximum.data(), values.data());

  LilvNode* lv2_InputPort = lilv_new_uri(world, LV2_CORE__InputPort);
  LilvNode* lv2_OutputPort = lilv_new_uri(world, LV2_CORE__OutputPort);
  LilvNode* lv2_AudioPort = lilv_new_uri(world, LV2_CORE__AudioPort);
  LilvNode* lv2_ControlPort = lilv_new_uri(world, LV2_CORE__ControlPort);
  LilvNode* lv2_AtomPort = lilv_new_uri(world, LV2_ATOM__AtomPort);
  LilvNode* lv2_connectionOptional = lilv_new_uri(world, LV2_CORE__connectionOptional);

  data_ports.in.left = data_ports.in.right = UINT_MAX;
  data_ports.probe.left = data_ports.probe.right = UINT_MAX;
  data_ports.out.left = data_ports.out.right = UINT_MAX;

  for (uint n = 0U; n < n_ports; n++) {
    auto* port = &ports[n];

    const auto* lilv_port = lilv_plugin_get_port_by_index(plugin, n);

    auto* port_name = lilv_port_get_name(plugin, lilv_port);

    port->index = n;
    port->name = lilv_node_as_string(port_name);
    port->symbol = lilv_node_as_string(lilv_port_get_symbol(plugin, lilv_port));
    port->optional = lilv_port_has_property(plugin, lilv_port, lv2_connectionOptional);

    // Save port default value
    if (!std::isnan(values[n])) {
      port->value = values[n];
    }
    // Save minimum and maximum values
    if (!std::isnan(minimum[n])) {
      port->min = minimum[n];
    }
    if (!std::isnan(maximum[n])) {
      port->max = maximum[n];
    }

    // util::warning("Port name: " + port->name);
    // util::warning("Port symbol: " + port->symbol);

    if (lilv_port_is_a(plugin, lilv_port, lv2_InputPort)) {
      port->is_input = true;
    } else if (!lilv_port_is_a(plugin, lilv_port, lv2_OutputPort) && !port->optional) {
      util::warning(std::format("Port {} is neither input nor output!", port->name));
    }

    if (lilv_port_is_a(plugin, lilv_port, lv2_ControlPort)) {
      port->type = lv2::PortType::TYPE_CONTROL;
    } else if (lilv_port_is_a(plugin, lilv_port, lv2_AtomPort)) {
      port->type = lv2::PortType::TYPE_ATOM;

      // util::warning("Port name: " + port->name);
    } else if (lilv_port_is_a(plugin, lilv_port, lv2_AudioPort)) {
      port->type = lv2::PortType::TYPE_AUDIO;

      if (port->is_input) {
        if (n_audio_in == 0) {
          data_ports.in.left = port->index;
        } else if (n_audio_in == 1) {
          data_ports.in.right = port->index;
        } else if (n_audio_in == 2) {
          data_ports.probe.left = port->index;
        } else if (n_audio_in == 3) {
          data_ports.probe.right = port->index;
        }

        n_audio_in++;
      } else {
        if (n_audio_out == 0) {
          data_ports.out.left = port->index;
        } else if (n_audio_out == 1) {
          data_ports.out.right = port->index;
        }

        n_audio_out++;
      }
    } else if (!port->optional) {
      util::warning(std::format("Port {} has un unsupported type!", port->name));
    }

    lilv_node_free(port_name);
  }

  // util::warning("n audio_in ports: " + util::to_string(n_audio_in));
  // util::warning("n audio_out ports: " + util::to_string(n_audio_out));

  lilv_node_free(lv2_connectionOptional);
  lilv_node_free(lv2_ControlPort);
  lilv_node_free(lv2_AtomPort);
  lilv_node_free(lv2_AudioPort);
  lilv_node_free(lv2_OutputPort);
  lilv_node_free(lv2_InputPort);
}

auto Lv2Wrapper::create_instance(const uint& rate) -> bool {
  if (instance != nullptr && this->rate == rate) {
    return true;
  }

  this->rate = rate;

  if (instance != nullptr) {
    deactivate();

    lilv_instance_free(instance);

    instance = nullptr;
  }

  LV2_Log_Log lv2_log = {this, &lv2_printf,
                         []([[maybe_unused]] LV2_Log_Handle handle, [[maybe_unused]] LV2_URID type, const char* fmt,
                            va_list ap) { return std::vprintf(fmt, ap); }};

  LV2_URID_Map lv2_map = {this, [](LV2_URID_Map_Handle handle, const char* uri) {
                            auto* lw = static_cast<Lv2Wrapper*>(handle);

                            return lw->map_urid(uri);
                          }};

  LV2_URID_Unmap lv2_unmap = {this, [](LV2_URID_Unmap_Handle handle, LV2_URID urid) {
                                auto* lw = static_cast<Lv2Wrapper*>(handle);

                                return lw->map_urid_to_uri[urid].c_str();
                              }};

  const LV2_Feature lv2_log_feature = {LV2_LOG__log, &lv2_log};

  const LV2_Feature lv2_map_feature = {LV2_URID__map, &lv2_map};

  const LV2_Feature lv2_unmap_feature = {LV2_URID__unmap, &lv2_unmap};

  auto options = std::to_array<LV2_Options_Option>(
      {{.context = LV2_OPTIONS_INSTANCE,
        .subject = 0,
        .key = map_urid(LV2_PARAMETERS__sampleRate),
        .size = sizeof(float),
        .type = map_urid(LV2_ATOM__Float),
        .value = &rate},
       {.context = LV2_OPTIONS_INSTANCE,
        .subject = 0,
        .key = map_urid(LV2_BUF_SIZE__minBlockLength),
        .size = sizeof(int32_t),
        .type = map_urid(LV2_ATOM__Int),
        .value = &min_quantum},
       {.context = LV2_OPTIONS_INSTANCE,
        .subject = 0,
        .key = map_urid(LV2_BUF_SIZE__maxBlockLength),
        .size = sizeof(int32_t),
        .type = map_urid(LV2_ATOM__Int),
        .value = &max_quantum},
       {.context = LV2_OPTIONS_INSTANCE,
        .subject = 0,
        .key = map_urid(LV2_BUF_SIZE__nominalBlockLength),
        .size = sizeof(int32_t),
        .type = map_urid(LV2_ATOM__Int),
        .value = &n_samples},
       {.context = LV2_OPTIONS_INSTANCE, .subject = 0, .key = 0, .size = 0, .type = 0, .value = nullptr}});

  LV2_Feature feature_options = {.URI = LV2_OPTIONS__options, .data = options.data()};

  const auto features = std::to_array<const LV2_Feature*>(
      {&lv2_log_feature, &lv2_map_feature, &lv2_unmap_feature, &feature_options, static_features.data(), nullptr});

  instance = lilv_plugin_instantiate(plugin, rate, features.data());

  if (instance == nullptr) {
    util::warning(std::format("Failed to instantiate {}", plugin_uri));

    return false;
  }

  connect_control_ports();

  activate();

  return true;
}

auto Lv2Wrapper::get_instance() -> LilvInstance* {
  return instance;
}

void Lv2Wrapper::connect_control_ports() {
  for (auto& p : ports) {
    if (p.type == PortType::TYPE_CONTROL) {
      lilv_instance_connect_port(instance, p.index, &p.value);
    }
  }
}

void Lv2Wrapper::connect_data_ports(std::span<float>& left_in,
                                    std::span<float>& right_in,
                                    std::span<float>& left_out,
                                    std::span<float>& right_out) {
  if (instance == nullptr) {
    return;
  }

  if (data_ports.in.left != UINT_MAX) {
    lilv_instance_connect_port(instance, data_ports.in.left, left_in.data());
  }
  if (data_ports.in.right != UINT_MAX) {
    lilv_instance_connect_port(instance, data_ports.in.right, right_in.data());
  }
  if (data_ports.out.left != UINT_MAX) {
    lilv_instance_connect_port(instance, data_ports.out.left, left_out.data());
  }
  if (data_ports.out.right != UINT_MAX) {
    lilv_instance_connect_port(instance, data_ports.out.right, right_out.data());
  }
}

void Lv2Wrapper::connect_data_ports(std::span<float>& left_in,
                                    std::span<float>& right_in,
                                    std::span<float>& left_out,
                                    std::span<float>& right_out,
                                    std::span<float>& probe_left,
                                    std::span<float>& probe_right) {
  if (instance == nullptr) {
    return;
  }

  if (data_ports.in.left != UINT_MAX) {
    lilv_instance_connect_port(instance, data_ports.in.left, left_in.data());
  }
  if (data_ports.in.right != UINT_MAX) {
    lilv_instance_connect_port(instance, data_ports.in.right, right_in.data());
  }
  if (data_ports.probe.left != UINT_MAX) {
    lilv_instance_connect_port(instance, data_ports.probe.left, probe_left.data());
  }
  if (data_ports.probe.right != UINT_MAX) {
    lilv_instance_connect_port(instance, data_ports.probe.right, probe_right.data());
  }
  if (data_ports.out.left != UINT_MAX) {
    lilv_instance_connect_port(instance, data_ports.out.left, left_out.data());
  }
  if (data_ports.out.right != UINT_MAX) {
    lilv_instance_connect_port(instance, data_ports.out.right, right_out.data());
  }
}

void Lv2Wrapper::set_n_samples(const uint& value) {
  this->n_samples = value;
}

auto Lv2Wrapper::get_n_samples() const -> uint {
  return this->n_samples;
}

auto Lv2Wrapper::get_rate() const -> uint {
  return this->rate;
}

void Lv2Wrapper::activate() {
  lilv_instance_activate(instance);
}

void Lv2Wrapper::run() const {
  if (instance != nullptr) {
    lilv_instance_run(instance, n_samples);
  }
}

void Lv2Wrapper::deactivate() {
  lilv_instance_deactivate(instance);
}

void Lv2Wrapper::set_control_port_value(const std::string& symbol, const float& value) {
  auto found = false;

  for (auto& p : ports) {
    if (p.type == PortType::TYPE_CONTROL && p.symbol == symbol) {
      if (!p.is_input) {
        util::warning(std::format("{} port {} is not an input!", plugin_uri, symbol));

        return;
      }

      ui_port_event(p.index, value);

      // Check port bounds
      if (value < p.min) {
        // util::warning(plugin_uri + ": value " + util::to_string(value) + "
        // is out of minimum limit for port " + p.symbol + " (" + p.name + ")");

        p.value = p.min;
      } else if (value > p.max) {
        // util::warning(plugin_uri + ": value " + util::to_string(value) + "
        // is out of maximum limit for port " + p.symbol + " (" + p.name + ")");

        p.value = p.max;
      } else {
        p.value = value;
      }

      found = true;

      break;
    }
  }

  if (!found) {
    util::warning(std::format("{} port symbol not found: {}", plugin_uri, symbol));
  }
}
auto Lv2Wrapper::get_control_port_value(const std::string& symbol) -> float {
  size_t hash = std::hash<std::string>{}(symbol);

  for (const auto& slot : control_ports_cache) {
    // We use hash == 0 to mean the cache slot and all following are empty.
    // We might get collisions with hash == 0, which is fine.
    if (slot.first == 0) {
      break;
    }

    if (slot.first != hash) {
      continue;
    }

    // Ignore false positives.
    const Port& p = ports[slot.second];
    if (p.type == PortType::TYPE_CONTROL && p.symbol == symbol) {
      return p.value;
    }
  }

  for (const auto& p : ports) {
    if (p.type == PortType::TYPE_CONTROL && p.symbol == symbol) {
      // Add to cache.
      for (auto& i : control_ports_cache) {
        if (i.first == 0) {
          i = std::pair<size_t, uint>(hash, p.index);
          break;
        }
      }

      return p.value;
    }
  }

  util::warning(std::format("{} port symbol not found: {}", plugin_uri, symbol));

  return 0.0F;
}

auto Lv2Wrapper::has_instance() -> bool {
  return instance != nullptr;
}

auto Lv2Wrapper::map_urid(const std::string& uri) -> LV2_URID {
  if (map_uri_to_urid.contains(uri)) {
    return map_uri_to_urid[uri];
  }

  const auto hash = std::hash<std::string>{}(uri);

  map_uri_to_urid[uri] = hash;

  map_urid_to_uri[hash] = uri;

  return static_cast<LV2_URID>(hash);
}

void Lv2Wrapper::load_ui() {
  native_ui.load();
}
void Lv2Wrapper::notify_ui() {
  native_ui.notify();
}
void Lv2Wrapper::update_ui() {
  native_ui.update();
}
bool Lv2Wrapper::has_ui() {
  return native_ui.has_ui();
}
void Lv2Wrapper::close_ui() {
  native_ui.close();
}
void Lv2Wrapper::ui_port_event(const uint& port_index, const float& value) {
  native_ui.port_event(port_index, value);
}
void Lv2Wrapper::native_ui_to_database() {
  native_ui.sync_to_database();
}

auto Lv2Wrapper::get_plugin_uri() -> std::string {
  return plugin_uri;
}

auto Lv2Wrapper::get_lilv_plugin() -> const LilvPlugin* {
  return plugin;
}

}  // namespace lv2
