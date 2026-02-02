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

#include <pipewire/filter.h>
#include <qobject.h>
#include <qthread.h>
#include <qtmetamacros.h>
#include <spa/utils/hook.h>
#include <sys/types.h>
#include <QTimer>
#include <atomic>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <vector>
#include "lv2_wrapper.hpp"
#include "pipeline_type.hpp"
#include "pw_manager.hpp"
#include "util.hpp"

class PluginBaseWorker : public QObject {
  Q_OBJECT
};

class PluginBase : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool updateLevelMeters MEMBER updateLevelMeters NOTIFY updateLevelMetersChanged)
  Q_PROPERTY(bool packageInstalled MEMBER packageInstalled NOTIFY packageInstalledChanged)

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

  bool packageInstalled = true;

  std::atomic<bool> bypass = {false};

  bool connected_to_pw = false;

  float latency_value = 0.0F;  // seconds

  /**
   * Even if it would be reasonable to initialize the peaks to
   * `util::minimum_db_level`, we want the plugins UI and the output level to
   * report 0 db in the initial stage (when nothing is playing), so 0.0F is
   * more suitable for this purpose.
   */
  float input_peak_left = 0.0F, input_peak_right = 0.0F;
  float output_peak_left = 0.0F, output_peak_right = 0.0F;

  bool got_null_left_in = false;
  bool got_null_left_out = false;
  bool got_null_right_in = false;
  bool got_null_right_out = false;
  bool got_null_probe = false;

  bool updateLevelMeters = false;

  std::vector<float> dummy_left, dummy_right, copy_left_in, copy_right_in;

  [[nodiscard]] auto get_node_id() const -> uint;

  void set_active(const bool& state) const;

  auto connect_to_pw() -> bool;

  void disconnect_from_pw();

  void set_native_ui_update_frequency(const uint& value);

  virtual void clear_data();

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

  Q_INVOKABLE virtual void reset() = 0;

  Q_INVOKABLE [[nodiscard]] float getInputLevelLeft() const;

  Q_INVOKABLE [[nodiscard]] float getInputLevelRight() const;

  Q_INVOKABLE [[nodiscard]] float getOutputLevelLeft() const;

  Q_INVOKABLE [[nodiscard]] float getOutputLevelRight() const;

  Q_INVOKABLE void showNativeUi();

  Q_INVOKABLE void closeNativeUi();

  Q_INVOKABLE bool hasNativeUi();

 Q_SIGNALS:

  void updateLevelMetersChanged();
  void packageInstalledChanged();

 protected:
  std::mutex data_mutex;

  pw::Manager* pm = nullptr;

  spa_hook listener{};

  data pf_data = {};

  uint n_ports = 4U;

  float input_gain = 1.0F;
  float output_gain = 1.0F;

  std::unique_ptr<lv2::Lv2Wrapper> lv2_wrapper;

  PluginBaseWorker* baseWorker;

  QThread workerThread;

  void get_peaks(const std::span<float>& left_in,
                 const std::span<float>& right_in,
                 std::span<float>& left_out,
                 std::span<float>& right_out);

  void apply_gain(std::span<float>& left, std::span<float>& right, const float& gain) const;

  void update_filter_params();

  void stop_worker();

  template <typename dbClass>
  void init_common_controls(dbClass* settings) {
    bypass = settings->bypass();
    input_gain = util::db_to_linear(settings->inputGain());
    output_gain = util::db_to_linear(settings->outputGain());

    connect(settings, &dbClass::bypassChanged, [&, settings]() { bypass = settings->bypass(); });
    connect(settings, &dbClass::inputGainChanged,
            [&, settings]() { input_gain = util::db_to_linear(settings->inputGain()); });
    connect(settings, &dbClass::outputGainChanged,
            [&, settings]() { output_gain = util::db_to_linear(settings->outputGain()); });
  }

 private:
  uint node_id = 0U;

  QTimer* native_ui_timer = nullptr;
};
