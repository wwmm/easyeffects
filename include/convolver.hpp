/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects
 *
 *  EasyEffectsis free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffectsis distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CONVOLVER_HPP
#define CONVOLVER_HPP

#include <zita-convolver.h>
#include <algorithm>
#include <ranges>
#include <sndfile.hh>
#include <vector>
#include "plugin_base.hpp"
#include "resampler.hpp"

class Convolver : public PluginBase {
 public:
  Convolver(const std::string& tag,
            const std::string& schema,
            const std::string& schema_path,
            PipeManager* pipe_manager);
  Convolver(const Convolver&) = delete;
  auto operator=(const Convolver&) -> Convolver& = delete;
  Convolver(const Convolver&&) = delete;
  auto operator=(const Convolver&&) -> Convolver& = delete;
  ~Convolver() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  sigc::signal<void(double)> latency;

 private:
  bool kernel_is_initialized = false;
  bool n_samples_is_power_of_2 = true;
  bool zita_ready = false;
  bool ready = false;
  bool notify_latency = false;

  uint blocksize = 512U;
  uint ir_width = 100U;
  uint latency_n_frames = 0U;

  std::vector<float> kernel_L, kernel_R;
  std::vector<float> original_kernel_L, original_kernel_R;
  std::vector<float> data_L, data_R;

  std::deque<float> deque_out_L, deque_out_R;

  Convproc* conv = nullptr;

  void read_kernel_file();

  void apply_kernel_autogain();

  void set_kernel_stereo_width();

  void setup_zita();

  auto get_zita_buffer_size() -> uint;

  template <typename T1>
  void do_convolution(T1& data_left, T1& data_right) {
    std::span conv_left_in{conv->inpdata(0), conv->inpdata(0) + get_zita_buffer_size()};
    std::span conv_right_in{conv->inpdata(1), conv->inpdata(1) + get_zita_buffer_size()};

    std::span conv_left_out{conv->outdata(0), conv->outdata(0) + get_zita_buffer_size()};
    std::span conv_right_out{conv->outdata(1), conv->outdata(1) + get_zita_buffer_size()};

    std::copy(data_left.begin(), data_left.end(), conv_left_in.begin());
    std::copy(data_right.begin(), data_right.end(), conv_right_in.begin());

    if (zita_ready) {
      int ret = conv->process(true);  // thread sync mode set to true

      if (ret != 0) {
        util::debug(log_tag + "IR: process failed: " + std::to_string(ret));

        zita_ready = false;
      } else {
        std::copy(conv_left_out.begin(), conv_left_out.end(), data_left.begin());
        std::copy(conv_right_out.begin(), conv_right_out.end(), data_right.begin());
      }
    }
  }
};

#endif
