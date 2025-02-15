/*
 *  Copyright © 2017-2025 Wellington Wallace
 *
 *  This file is part of Easy Effects
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

#include <qtmetamacros.h>
#include <sys/types.h>
#include <zita-convolver.h>
#include <QString>
#include <deque>
#include <span>
#include <string>
#include <thread>
#include <tuple>
#include <vector>
#include "easyeffects_db_convolver.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "util.hpp"

class Convolver : public PluginBase {
  Q_OBJECT

  Q_PROPERTY(QString kernelRate MEMBER kernelRate NOTIFY kernelRateChanged)
  Q_PROPERTY(QString kernelSamples MEMBER kernelSamples NOTIFY kernelSamplesChanged)
  Q_PROPERTY(QString kernelDuration MEMBER kernelDuration NOTIFY kernelDurationChanged)

 public:
  Convolver(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  Convolver(const Convolver&) = delete;
  auto operator=(const Convolver&) -> Convolver& = delete;
  Convolver(const Convolver&&) = delete;
  auto operator=(const Convolver&&) -> Convolver& = delete;
  ~Convolver() override;

  void reset() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out,
               std::span<float>& probe_left,
               std::span<float>& probe_right) override;

  auto get_latency_seconds() -> float override;

  const std::string irs_ext = ".irs";

  auto search_irs_path(const std::string& name) -> std::string;

 Q_SIGNALS:

  void kernelRateChanged();
  void kernelSamplesChanged();
  void kernelDurationChanged();

 private:
  db::Convolver* settings = nullptr;

  std::string app_config_dir;
  std::string local_dir_irs;
  std::vector<std::string> system_data_dir_irs;

  bool kernel_is_initialized = false;
  bool n_samples_is_power_of_2 = true;
  bool zita_ready = false;
  bool ready = false;
  bool notify_latency = false;

  uint blocksize = 512U;
  uint latency_n_frames = 0U;

  std::vector<float> kernel_L, kernel_R;
  std::vector<float> original_kernel_L, original_kernel_R;
  std::vector<float> data_L, data_R;

  std::deque<float> deque_out_L, deque_out_R;

  Convproc* conv = nullptr;

  std::vector<std::thread> mythreads;

  QString kernelRate;
  QString kernelSamples;
  QString kernelDuration;

  void apply_kernel_autogain();

  void set_kernel_stereo_width();

  void setup_zita();

  auto get_zita_buffer_size() -> uint;

  void prepare_kernel();

  static void direct_conv(const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& c);

  auto read_kernel_file(const std::string& kernel_path) -> std::tuple<int, std::vector<float>, std::vector<float>>;

  void load_kernel_file();

  void combine_kernels(const std::string& kernel_1_name,
                       const std::string& kernel_2_name,
                       const std::string& output_file_name);

  template <typename T1>
  void do_convolution(T1& data_left, T1& data_right) {
    std::span conv_left_in(conv->inpdata(0), get_zita_buffer_size());
    std::span conv_right_in(conv->inpdata(1), get_zita_buffer_size());

    std::span conv_left_out(conv->outdata(0), get_zita_buffer_size());
    std::span conv_right_out(conv->outdata(1), get_zita_buffer_size());

    std::copy(data_left.begin(), data_left.end(), conv_left_in.begin());
    std::copy(data_right.begin(), data_right.end(), conv_right_in.begin());

    if (zita_ready) {
      const int& ret = conv->process(true);  // thread sync mode set to true

      if (ret != 0) {
        util::debug(log_tag + "IR: process failed: " + util::to_string(ret, ""));

        zita_ready = false;
      } else {
        std::copy(conv_left_out.begin(), conv_left_out.end(), data_left.begin());
        std::copy(conv_right_out.begin(), conv_right_out.end(), data_right.begin());
      }
    }
  }
};
