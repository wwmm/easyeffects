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

#include "ladspa_wrapper.hpp"
#include <dlfcn.h>
#include <ladspa.h>
#include <sys/types.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <format>
#include <limits>
#include <span>
#include <string>
#include <tuple>
#include <utility>
#include "config.h"
#include "util.hpp"

namespace ladspa {

static inline const char* get_ladspa_path() {
  const char* path = std::getenv("LADSPA_PATH");

  if (path == nullptr || path[0] == '\0') {
    path = "/usr/local/lib/ladspa/:" LIB_DIR "/ladspa/:/usr/lib/ladspa/:/usr/lib64/ladspa/";
  }

  return path;
}

struct dlhandle {
  dlhandle(void* handle) : dl_handle(handle) {}

  ~dlhandle() {
    if (dl_handle != nullptr) {
      dlclose(dl_handle);
    }
  }

  dlhandle(const dlhandle&) = delete;
  auto operator=(const dlhandle&) -> dlhandle& = delete;
  dlhandle(dlhandle&& other) noexcept : dl_handle(std::exchange(other.dl_handle, nullptr)) {}
  auto operator=(dlhandle&& other) noexcept -> dlhandle& {
    std::swap(dl_handle, other.dl_handle);
    return *this;
  }

  void disable() { dl_handle = nullptr; }
  void* dl_handle;
};

static inline bool validate_ports(const LADSPA_Descriptor* descriptor) {
  unsigned long count_input = 0UL;
  unsigned long count_output = 0UL;

  for (unsigned long i = 0UL; i < descriptor->PortCount; i++) {
    if (LADSPA_IS_PORT_CONTROL(descriptor->PortDescriptors[i])) {
      if (LADSPA_IS_PORT_AUDIO(descriptor->PortDescriptors[i])) {
        return false;
      }
    } else if (LADSPA_IS_PORT_AUDIO(descriptor->PortDescriptors[i])) {
      if (LADSPA_IS_PORT_INPUT(descriptor->PortDescriptors[i])) {
        if (++count_input > 4UL) {
          return false;
        }
      } else if (LADSPA_IS_PORT_OUTPUT(descriptor->PortDescriptors[i])) {
        if (++count_output > 2UL) {
          return false;
        }
      } else {
        return false;
      }
    }
  }

  return true;
}

LadspaWrapper::LadspaWrapper(const std::string& plugin_filename, const std::string& plugin_label)
    : plugin_name(plugin_label) {
  const char* ladspa_path = get_ladspa_path();
  const char* p = nullptr;

  do {
    p = std::strchr(ladspa_path, ':');

    if (!p) {
      p = std::strchr(ladspa_path, '\0');
    }

    std::string path(ladspa_path, p - ladspa_path);

    if (*p == ':') {
      p++;
    }

    if (path.empty() || path[path.length() - 1] != '/') {
      path.push_back('/');
    }

    path.append(plugin_filename);

    void* dl_handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);

    if (dl_handle == nullptr) {
      continue;
    }

    dlhandle h(dl_handle);

    auto func = (LADSPA_Descriptor_Function)dlsym(dl_handle, "ladspa_descriptor");

    if (func == nullptr) {
      continue;
    }

    unsigned long i = 0UL;

    const LADSPA_Descriptor* descriptor = nullptr;

    do {
      descriptor = func(i);
      if (descriptor == nullptr) {
        break;
      }

      if (std::strcmp(descriptor->Label, plugin_label.c_str()) == 0) {
        break;
      }
    } while (i++ < std::numeric_limits<unsigned long>::max());

    if (descriptor != nullptr) {
      if (descriptor->instantiate != nullptr && descriptor->connect_port != nullptr && descriptor->run != nullptr &&
          validate_ports(descriptor)) {
        unsigned long count = 0UL;

        for (unsigned long i = 0UL; i < descriptor->PortCount; i++) {
          if (LADSPA_IS_PORT_CONTROL(descriptor->PortDescriptors[i])) {
            count++;
          }
        }

        auto* control_ports = new LADSPA_Data[count]();
        auto* control_ports_initialized = new bool[count]();

        h.disable();

        this->dl_handle = dl_handle;
        this->descriptor = descriptor;
        this->found = true;
        this->control_ports = control_ports;
        this->control_ports_initialized = control_ports_initialized;

        for (unsigned long i = 0UL, j = 0UL; i < descriptor->PortCount; i++) {
          if (LADSPA_IS_PORT_CONTROL(descriptor->PortDescriptors[i])) {
            map_cp_name_to_idx.insert(std::make_pair(descriptor->PortNames[i], j++));
          }
        }
      }
      break;
    }
  } while (*(ladspa_path = p) != '\0');
}

LadspaWrapper::~LadspaWrapper() {
  if (active) {
    deactivate();
  }

  LADSPA_Handle instance = std::exchange(this->instance, nullptr);

  if (instance != nullptr && descriptor->cleanup != nullptr) {
    descriptor->cleanup(instance);
  }

  if (control_ports_initialized) {
    delete[] std::exchange(control_ports_initialized, nullptr);
  }

  if (control_ports) {
    delete[] std::exchange(control_ports, nullptr);
  }

  if (dl_handle != nullptr) {
    dlclose(std::exchange(dl_handle, nullptr));
  }
}

struct ladspahandle {
  ladspahandle(LADSPA_Handle instance, void (*cleanup)(LADSPA_Handle)) : instance(instance), cleanup(cleanup) {}

  ~ladspahandle() {
    if (instance != nullptr) {
      cleanup(instance);
    }
  }

  ladspahandle(const ladspahandle&) = delete;

  auto operator=(const ladspahandle&) -> ladspahandle& = delete;

  ladspahandle(ladspahandle&& other) noexcept
      : instance(std::exchange(other.instance, nullptr)), cleanup(other.cleanup) {}

  auto operator=(ladspahandle&& other) noexcept -> ladspahandle& {
    std::swap(instance, other.instance);
    std::swap(cleanup, other.cleanup);

    return *this;
  }

  void disable() { instance = nullptr; }

  void* instance;

  void (*cleanup)(LADSPA_Handle);
};

static inline void get_port_bounds(const LADSPA_Descriptor* descriptor,
                                   unsigned long port,
                                   uint rate,
                                   LADSPA_Data& min,
                                   LADSPA_Data& max) {
  min = -std::numeric_limits<LADSPA_Data>::infinity();
  max = std::numeric_limits<LADSPA_Data>::infinity();

  const LADSPA_PortRangeHint* hint = &descriptor->PortRangeHints[port];

  if (LADSPA_IS_HINT_BOUNDED_BELOW(hint->HintDescriptor)) {
    min = hint->LowerBound;

    if (LADSPA_IS_HINT_SAMPLE_RATE(hint->HintDescriptor)) {
      min *= rate;
    }
  }

  if (LADSPA_IS_HINT_BOUNDED_ABOVE(hint->HintDescriptor)) {
    max = hint->UpperBound;

    if (LADSPA_IS_HINT_SAMPLE_RATE(hint->HintDescriptor)) {
      min *= rate;
    }
  }

  if (LADSPA_IS_HINT_TOGGLED(hint->HintDescriptor)) {
    min = 0.0F;
    max = 1.0F;
  }
}

static inline LADSPA_Data get_port_default_raw(const LADSPA_Descriptor* descriptor, unsigned long port, uint rate) {
  const LADSPA_PortRangeHint* hint = &descriptor->PortRangeHints[port];
  if (LADSPA_IS_HINT_HAS_DEFAULT(hint->HintDescriptor)) {
    if (LADSPA_IS_HINT_DEFAULT_0(hint->HintDescriptor)) {
      return 0.0F;

    } else if (LADSPA_IS_HINT_DEFAULT_1(hint->HintDescriptor)) {
      return 1.0F;

    } else if (LADSPA_IS_HINT_DEFAULT_100(hint->HintDescriptor)) {
      return 100.0F;

    } else if (LADSPA_IS_HINT_DEFAULT_440(hint->HintDescriptor)) {
      return 440.0F;

    } else {
      LADSPA_Data min = 0.0F;
      LADSPA_Data max = 0.0F;

      get_port_bounds(descriptor, port, rate, min, max);

      if (LADSPA_IS_HINT_DEFAULT_MINIMUM(hint->HintDescriptor)) {
        return min;
      } else if (LADSPA_IS_HINT_DEFAULT_MAXIMUM(hint->HintDescriptor)) {
        return max;
      } else if (LADSPA_IS_HINT_LOGARITHMIC(hint->HintDescriptor)) {
        if (LADSPA_IS_HINT_DEFAULT_LOW(hint->HintDescriptor)) {
          return std::exp((std::log(min) * 0.75) + (std::log(max) * 0.25));
        } else if (LADSPA_IS_HINT_DEFAULT_MIDDLE(hint->HintDescriptor)) {
          return std::exp((std::log(min) * 0.5) + (std::log(max) * 0.5));
        } else if (LADSPA_IS_HINT_DEFAULT_HIGH(hint->HintDescriptor)) {
          return std::exp((std::log(min) * 0.25) + (std::log(max) * 0.75));
        }
      } else {
        if (LADSPA_IS_HINT_DEFAULT_LOW(hint->HintDescriptor)) {
          return (min * 0.75F) + (max * 0.25F);
        } else if (LADSPA_IS_HINT_DEFAULT_MIDDLE(hint->HintDescriptor)) {
          return (min * 0.5F) + (max * 0.5F);
        } else if (LADSPA_IS_HINT_DEFAULT_HIGH(hint->HintDescriptor)) {
          return (min * 0.25F) + (max * 0.75F);
        }
      }
    }
  }

  return 0.0F;
}

static inline LADSPA_Data clamp_port_value(const LADSPA_Descriptor* descriptor,
                                           unsigned long port,
                                           uint rate,
                                           LADSPA_Data val) {
  LADSPA_Data min = 0.0F;
  LADSPA_Data max = 0.0F;

  get_port_bounds(descriptor, port, rate, min, max);

  return std::clamp(val, min, max);
}

static inline LADSPA_Data get_port_default(const LADSPA_Descriptor* descriptor, unsigned long port, uint rate) {
  LADSPA_Data val = get_port_default_raw(descriptor, port, rate);

  return clamp_port_value(descriptor, port, rate, val);
}

static inline void scale_control_ports(const LADSPA_Descriptor* descriptor,
                                       LADSPA_Data* control_ports,
                                       bool* control_ports_initialized,
                                       uint old_rate,
                                       uint rate) {
  for (unsigned long i = 0UL, j = 0UL; i < descriptor->PortCount; i++) {
    if (LADSPA_IS_PORT_CONTROL(descriptor->PortDescriptors[i])) {
      if (!control_ports_initialized[j]) {
        control_ports[j] = get_port_default(descriptor, i, rate);
        control_ports_initialized[j] = true;
      } else if (old_rate != 0U) {
        const LADSPA_PortRangeHint* hint = &descriptor->PortRangeHints[i];

        if (LADSPA_IS_HINT_SAMPLE_RATE(hint->HintDescriptor)) {
          LADSPA_Data val = control_ports[j] * rate / old_rate;
          control_ports[j] = clamp_port_value(descriptor, i, rate, val);
        }
      }

      j++;
    }
  }
}

auto LadspaWrapper::create_instance(uint rate) -> bool {
  LADSPA_Handle new_instance = descriptor->instantiate(descriptor, rate);

  if (new_instance == nullptr) {
    return false;
  }

  ladspahandle h(new_instance, descriptor->cleanup);

  scale_control_ports(descriptor, control_ports, control_ports_initialized, this->rate, rate);

  for (unsigned long i = 0UL, j = 0UL; i < descriptor->PortCount; i++) {
    if (LADSPA_IS_PORT_CONTROL(descriptor->PortDescriptors[i])) {
      descriptor->connect_port(new_instance, i, &control_ports[j++]);
    }
  }

  if (active) {
    deactivate();
  }

  if (instance != nullptr && descriptor->cleanup != nullptr) {
    descriptor->cleanup(instance);
  }

  h.disable();

  this->instance = new_instance;
  this->rate = rate;

  activate();

  return true;
}

static inline int stricmp(const char* str1, const char* str2) {
  char c1 = 0;
  char c2 = 0;

  do {
    c1 = *str1;
    c2 = *str2;

    if (c1 != c2) {
      if (c1 >= 'A' && c1 <= 'Z') {
        c1 -= 'A';
      }

      if (c2 >= 'A' && c2 <= 'Z') {
        c2 -= 'A';
      }

      if (c1 != c2) {
        break;
      }
    }
  } while (c1 != '\0');
  return (int)c1 - c2;
}

template <std::size_t N>
static inline bool striendswith(const char* str, const char (&end)[N]) {
  std::size_t len = std::strlen(str);
  return len < (N - 1) && stricmp(&str[len - (N - 1)], end) == 0;
}

void LadspaWrapper::connect_data_ports(const std::span<const float>& left_in,
                                       const std::span<const float>& right_in,
                                       const std::span<float>& left_out,
                                       const std::span<float>& right_out) {
  if (instance == nullptr) {
    return;
  }

  unsigned long left_in_idx = -1L;
  unsigned long right_in_idx = -1L;
  unsigned long first_in_idx = -1L;
  unsigned long second_in_idx = -1L;
  unsigned long left_out_idx = -1L;
  unsigned long right_out_idx = -1L;
  unsigned long first_out_idx = -1L;
  unsigned long second_out_idx = -1L;

  int count_input = 0;
  int count_output = 0;

  for (unsigned long i = 0UL; i < descriptor->PortCount; i++) {
    if (LADSPA_IS_PORT_AUDIO(descriptor->PortDescriptors[i])) {
      if (LADSPA_IS_PORT_INPUT(descriptor->PortDescriptors[i])) {
        if (striendswith(descriptor->PortNames[i], " L") || striendswith(descriptor->PortNames[i], " (L)")) {
          left_in_idx = i;
        } else if (striendswith(descriptor->PortNames[i], " R") || striendswith(descriptor->PortNames[i], " (R)")) {
          right_in_idx = i;
        }

        if (count_input == 0) {
          first_in_idx = i;
        } else if (count_input == 1) {
          second_in_idx = i;
        }

        count_input++;
      } else if (LADSPA_IS_PORT_OUTPUT(descriptor->PortDescriptors[i])) {
        if (striendswith(descriptor->PortNames[i], " L") || striendswith(descriptor->PortNames[i], " (L)")) {
          left_out_idx = i;
        } else if (striendswith(descriptor->PortNames[i], " R") || striendswith(descriptor->PortNames[i], " (R)")) {
          right_out_idx = i;
        }

        if (count_output == 0) {
          first_out_idx = i;
        } else if (count_output == 1) {
          second_out_idx = i;
        }

        count_output++;
      }
    }
  }

  if (left_in_idx == null_ul || right_in_idx == null_ul) {
    left_in_idx = first_in_idx;
    right_in_idx = second_in_idx;
  }
  if (left_in_idx != null_ul) {
    descriptor->connect_port(instance, left_in_idx, const_cast<LADSPA_Data*>(left_in.data()));
  }

  if (right_in_idx != null_ul) {
    descriptor->connect_port(instance, right_in_idx, const_cast<LADSPA_Data*>(right_in.data()));
  }

  if (left_out_idx == null_ul || right_out_idx == null_ul) {
    left_out_idx = first_out_idx;
    right_out_idx = second_out_idx;
  }
  if (left_out_idx != null_ul) {
    descriptor->connect_port(instance, left_out_idx, left_out.data());
  }

  if (right_out_idx != null_ul) {
    descriptor->connect_port(instance, right_out_idx, right_out.data());
  }
}

static inline char* stristr(const char* haystack, const char* needle) {
  size_t hlen = std::strlen(haystack);
  size_t nlen = std::strlen(needle);

  while (nlen <= hlen) {
    if (stricmp(haystack, needle) == 0) {
      return (char*)haystack;
    }

    haystack++;
    hlen--;
  }
  return nullptr;
}

void LadspaWrapper::connect_data_ports(const std::span<const float>& left_in,
                                       const std::span<const float>& right_in,
                                       const std::span<float>& left_out,
                                       const std::span<float>& right_out,
                                       const std::span<const float>& probe_left,
                                       const std::span<const float>& probe_right) {
  if (instance == nullptr) {
    return;
  }

  unsigned long left_in_idx = -1L;
  unsigned long right_in_idx = -1L;
  unsigned long first_in_idx = -1L;
  unsigned long second_in_idx = -1L;
  unsigned long probe_left_idx = -1L;
  unsigned long probe_right_idx = -1L;
  unsigned long third_in_idx = -1L;
  unsigned long fourth_in_idx = -1L;
  unsigned long left_out_idx = -1L;
  unsigned long right_out_idx = -1L;
  unsigned long first_out_idx = -1L;
  unsigned long second_out_idx = -1L;

  int count_input = 0;
  int count_output = 0;

  for (unsigned long i = 0UL; i < descriptor->PortCount; i++) {
    if (LADSPA_IS_PORT_AUDIO(descriptor->PortDescriptors[i])) {
      if (LADSPA_IS_PORT_INPUT(descriptor->PortDescriptors[i])) {
        bool sc = stristr(descriptor->PortNames[i], "Probe") != nullptr ||
                  stristr(descriptor->PortNames[i], "Sidechain") != nullptr;
        if (striendswith(descriptor->PortNames[i], " L") || striendswith(descriptor->PortNames[i], " (L)")) {
          if (sc) {
            probe_left_idx = i;
          } else {
            left_in_idx = i;
          }
        } else if (striendswith(descriptor->PortNames[i], " R") || striendswith(descriptor->PortNames[i], " (R)")) {
          if (sc) {
            probe_right_idx = i;
          } else {
            right_in_idx = i;
          }
        }

        if (count_input == 0) {
          first_in_idx = i;
        } else if (count_input == 1) {
          second_in_idx = i;
        } else if (count_input == 2) {
          third_in_idx = i;
        } else if (count_input == 3) {
          fourth_in_idx = i;
        }

        count_input++;
      } else if (LADSPA_IS_PORT_OUTPUT(descriptor->PortDescriptors[i])) {
        if (striendswith(descriptor->PortNames[i], " L") || striendswith(descriptor->PortNames[i], " (L)")) {
          left_out_idx = i;
        } else if (striendswith(descriptor->PortNames[i], " R") || striendswith(descriptor->PortNames[i], " (R)")) {
          right_out_idx = i;
        }

        if (count_output == 0) {
          first_out_idx = i;
        } else if (count_output == 1) {
          second_out_idx = i;
        }

        count_output++;
      }
    }
  }

  if (left_in_idx == null_ul || right_in_idx == null_ul) {
    left_in_idx = first_in_idx;
    right_in_idx = second_in_idx;
  }
  if (left_in_idx != null_ul) {
    descriptor->connect_port(instance, left_in_idx, const_cast<LADSPA_Data*>(left_in.data()));
  }

  if (right_in_idx != null_ul) {
    descriptor->connect_port(instance, right_in_idx, const_cast<LADSPA_Data*>(right_in.data()));
  }

  if (probe_left_idx == null_ul || probe_right_idx == null_ul) {
    if (left_in_idx == first_in_idx) {
      if (right_in_idx == second_in_idx) {
        probe_left_idx = third_in_idx;
      } else {
        probe_left_idx = second_in_idx;
      }
    } else if (right_in_idx == first_in_idx) {
      if (left_in_idx == second_in_idx) {
        probe_left_idx = third_in_idx;
      } else {
        probe_left_idx = second_in_idx;
      }
    } else {
      probe_left_idx = first_in_idx;
    }
    if (left_in_idx == second_in_idx) {
      if (right_in_idx == third_in_idx) {
        probe_right_idx = fourth_in_idx;
      } else {
        probe_right_idx = third_in_idx;
      }
    } else if (right_in_idx == second_in_idx) {
      if (left_in_idx == third_in_idx) {
        probe_right_idx = fourth_in_idx;
      } else {
        probe_right_idx = third_in_idx;
      }
    } else {
      probe_right_idx = second_in_idx;
    }
  }
  if (probe_left_idx != null_ul) {
    descriptor->connect_port(instance, probe_left_idx, const_cast<LADSPA_Data*>(probe_left.data()));
  }

  if (probe_right_idx != null_ul) {
    descriptor->connect_port(instance, probe_right_idx, const_cast<LADSPA_Data*>(probe_right.data()));
  }

  if (left_out_idx == null_ul || right_out_idx == null_ul) {
    left_out_idx = first_out_idx;
    right_out_idx = second_out_idx;
  }
  if (left_out_idx != null_ul) {
    descriptor->connect_port(instance, left_out_idx, left_out.data());
  }

  if (right_out_idx != null_ul) {
    descriptor->connect_port(instance, right_out_idx, right_out.data());
  }
}

void LadspaWrapper::activate() {
  if (!active && descriptor->activate != nullptr) {
    descriptor->activate(instance);
  }

  active = true;
}

void LadspaWrapper::deactivate() {
  if (active && descriptor->activate != nullptr && descriptor->deactivate != nullptr) {
    descriptor->deactivate(instance);
  }

  active = false;
}

void LadspaWrapper::run() const {
  assert(active);
  assert(instance);

  descriptor->run(instance, n_samples);
}

auto LadspaWrapper::get_control_port_count() const -> uint {
  uint count = 0U;

  for (unsigned long i = 0UL; i < descriptor->PortCount; i++) {
    if (LADSPA_IS_PORT_CONTROL(descriptor->PortDescriptors[i])) {
      count++;
    }
  }

  return count;
}

static inline unsigned long cp_to_port_idx(const LADSPA_Descriptor* descriptor, uint index) {
  for (unsigned long i = 0UL; i < descriptor->PortCount; i++) {
    if (LADSPA_IS_PORT_CONTROL(descriptor->PortDescriptors[i])) {
      if (index-- == 0U) {
        return i;
      }
    }
  }

  return (unsigned long)-1L;
}

auto LadspaWrapper::get_control_port_name(uint index) const -> std::string {
  const unsigned long i = cp_to_port_idx(descriptor, index);

  if (i == null_ul) {
    return {};
  }

  return descriptor->PortNames[i];
}

auto LadspaWrapper::is_control_port_output(uint index) const -> bool {
  const unsigned long i = cp_to_port_idx(descriptor, index);

  if (i == null_ul) {
    return false;
  }

  return LADSPA_IS_PORT_OUTPUT(descriptor->PortDescriptors[i]);
}

auto LadspaWrapper::get_control_port_range(uint index) const -> std::tuple<float, float> {
  const unsigned long i = cp_to_port_idx(descriptor, index);

  if (i == null_ul) {
    return std::make_tuple(0.0F, 0.0F);
  }

  LADSPA_Data min = 0.0F;
  LADSPA_Data max = 0.0F;

  get_port_bounds(descriptor, i, rate, min, max);

  return std::make_tuple(min, max);
}

auto LadspaWrapper::get_control_port_default(uint index) const -> float {
  const unsigned long i = cp_to_port_idx(descriptor, index);

  if (i == null_ul) {
    return 0.0F;
  }

  return get_port_default(descriptor, i, rate);
}

auto LadspaWrapper::get_control_port_value(uint index) const -> float {
  assert(cp_to_port_idx(descriptor, index) != null_ul);
  assert(control_ports_initialized[index]);

  return control_ports[index];
}

auto LadspaWrapper::get_control_port_value(const std::string& symbol) const -> float {
  auto iter = map_cp_name_to_idx.find(symbol);

  if (iter == map_cp_name_to_idx.end()) {
    util::warning(std::format("{} port symbol not found: {}", plugin_name, symbol));

    return 0.0F;
  }

  return get_control_port_value(static_cast<uint>(iter->second));
}

auto LadspaWrapper::set_control_port_value_clamp(uint index, float value) -> float {
  const unsigned long i = cp_to_port_idx(descriptor, index);

  assert(i != null_ul);

  // If the value is out of bounds, get a new clamped one in LADSPA_Data (float)
  value = clamp_port_value(descriptor, i, rate, value);

  control_ports[index] = value;
  control_ports_initialized[index] = true;

  return value;
}

auto LadspaWrapper::set_control_port_value_clamp(const std::string& symbol, float value) -> float {
  auto iter = map_cp_name_to_idx.find(symbol);

  if (iter == map_cp_name_to_idx.end()) {
    util::warning(std::format("{} port symbol not found: {}", plugin_name, symbol));

    return value;
  }

  return set_control_port_value_clamp(static_cast<uint>(iter->second), value);
}

}  // namespace ladspa
