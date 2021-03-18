/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PLUGIN_BASE_HPP
#define PLUGIN_BASE_HPP

#include <giomm.h>
#include <pipewire/filter.h>
#include <span>
#include "pipe_manager.hpp"
#include "plugin_name.hpp"

class PluginBase {
 public:
  PluginBase(std::string tag,
             std::string plugin_name,
             const std::string& schema,
             const std::string& schema_path,
             PipeManager* pipe_manager);
  PluginBase(const PluginBase&) = delete;
  auto operator=(const PluginBase&) -> PluginBase& = delete;
  PluginBase(const PluginBase&&) = delete;
  auto operator=(const PluginBase&&) -> PluginBase& = delete;
  virtual ~PluginBase();

  struct data;

  struct port {
    struct data* data;
  };

  struct data {
    struct port *in_left = nullptr, *in_right = nullptr, *out_left = nullptr, *out_right = nullptr;

    PluginBase* pb = nullptr;
  };

  std::string log_tag, name;

  pw_filter* filter = nullptr;

  uint n_samples = 0;

  uint rate = 0;

  float sample_duration = 0.0F;

  bool bypass = false;

  bool post_messages = false;

  [[nodiscard]] auto get_node_id() const -> uint;

  void set_active(const bool& state) const;

  virtual void setup();

  virtual void process(std::span<float>& left_in,
                       std::span<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out);

  sigc::signal<void(float, float)> input_level;
  sigc::signal<void(float, float)> output_level;

 protected:
  Glib::RefPtr<Gio::Settings> settings;

  PipeManager* pm = nullptr;

  spa_hook listener{};

  float input_gain = 1.0F;
  float output_gain = 1.0F;

  float notification_time_window = 1.0F / 20.0F;  // seconds
  float notification_dt = 0.0F;

  void notify();

  void get_peaks(const std::span<float>& left_in,
                 const std::span<float>& right_in,
                 std::span<float>& left_out,
                 std::span<float>& right_out);

  static void apply_gain(std::span<float>& left, std::span<float>& right, const float& gain);

 private:
  data pf_data = {};

  uint node_id = 0;

  float input_peak_left = util::minimum_linear_level, input_peak_right = util::minimum_linear_level;
  float output_peak_left = util::minimum_linear_level, output_peak_right = util::minimum_linear_level;
};

#endif
