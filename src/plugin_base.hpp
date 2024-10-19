/*
 *  Copyright © 2017-2024 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <pipewire/filter.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <spa/utils/hook.h>
#include <sys/types.h>
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <vector>
#include "lv2_wrapper.hpp"
#include "pipeline_type.hpp"
#include "pw_manager.hpp"
#include "util.hpp"

class PluginBase : public QObject {
  Q_OBJECT;

  Q_PROPERTY(float inputPeakLeft MEMBER input_peak_left NOTIFY inputPeakLeftChanged)
  Q_PROPERTY(float inputPeakRight MEMBER input_peak_right NOTIFY inputPeakRightChanged)
  Q_PROPERTY(float outputPeakLeft MEMBER output_peak_left NOTIFY outputPeakLeftChanged)
  Q_PROPERTY(float outputPeakRight MEMBER output_peak_right NOTIFY outputPeakRightChanged)

 public:
  PluginBase(std::string tag,
             QString plugin_name,
             QString plugin_package,
             QString instance_id,
             pw::Manager* pipe_manager,
             PipelineType pipe_type,
             const bool& enable_probe = false);
  PluginBase(const PluginBase&) = delete;
  auto operator=(const PluginBase&) -> PluginBase& = delete;
  PluginBase(const PluginBase&&) = delete;
  auto operator=(const PluginBase&&) -> PluginBase& = delete;
  ~PluginBase() override;

  struct data;

  struct port {
    struct data* data;
  };

  struct data {
    struct port* in_left = nullptr;
    struct port* in_right = nullptr;

    struct port* out_left = nullptr;
    struct port* out_right = nullptr;

    struct port* probe_left = nullptr;
    struct port* probe_right = nullptr;

    PluginBase* pb = nullptr;
  };

  const std::string log_tag;

  QString name, package, instance_id;

  PipelineType pipeline_type{};

  pw_filter* filter = nullptr;

  pw_filter_state state = PW_FILTER_STATE_UNCONNECTED;

  bool can_get_node_id = false;

  bool enable_probe = false;

  uint n_samples = 0U;

  uint rate = 0U;

  bool package_installed = true;

  std::atomic<bool> bypass = {false};
  static_assert(std::atomic<bool>::is_always_lock_free);

  bool connected_to_pw = false;

  float latency_value = 0.0F;  // seconds

  std::chrono::time_point<std::chrono::system_clock> clock_start;

  std::vector<float> dummy_left, dummy_right;

  [[nodiscard]] auto get_node_id() const -> uint;

  void set_active(const bool& state) const;

  auto connect_to_pw() -> bool;

  void disconnect_from_pw();

  void reset_settings();

  void show_native_ui();

  void close_native_ui();

  void set_native_ui_update_frequency(const uint& value);

  virtual void setup();

  virtual void process(std::span<float>& left_in,
                       std::span<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out);

  virtual void process(std::span<float>& left_in,
                       std::span<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out,
                       std::span<float>& probe_left,
                       std::span<float>& probe_right);

  virtual void update_probe_links();

  virtual auto get_latency_seconds() -> float;

 signals:
  void inputPeakLeftChanged();
  void inputPeakRightChanged();
  void outputPeakLeftChanged();
  void outputPeakRightChanged();

  void latency();

 protected:
  std::mutex data_mutex;

  pw::Manager* pm = nullptr;

  spa_hook listener{};

  data pf_data = {};

  uint n_ports = 4U;

  float input_gain = 1.0F;
  float output_gain = 1.0F;

  std::unique_ptr<lv2::Lv2Wrapper> lv2_wrapper;

  void setup_input_output_gain();

  void initialize_listener();

  void get_peaks(const std::span<float>& left_in,
                 const std::span<float>& right_in,
                 std::span<float>& left_out,
                 std::span<float>& right_out);

  static void apply_gain(std::span<float>& left, std::span<float>& right, const float& gain);

  void update_filter_params();

 private:
  uint node_id = 0U;

  float input_peak_left = util::minimum_linear_level, input_peak_right = util::minimum_linear_level;
  float output_peak_left = util::minimum_linear_level, output_peak_right = util::minimum_linear_level;
};