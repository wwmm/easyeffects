/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef OUTPUT_STREAM_HPP
#define OUTPUT_STREAM_HPP

#include <ranges>
#include <span>
#include "pipe_manager.hpp"
#include "plugin_base.hpp"

class OutputStream {
 public:
  OutputStream(std::string tag, PipeManager* pipe_manager);
  OutputStream(const OutputStream&) = delete;
  auto operator=(const OutputStream&) -> OutputStream& = delete;
  OutputStream(const OutputStream&&) = delete;
  auto operator=(const OutputStream&&) -> OutputStream& = delete;
  ~OutputStream();

  struct data;

  struct port {
    struct data* data;
  };

  struct data {
    struct port* in_left = nullptr;
    struct port* in_right = nullptr;

    OutputStream* os = nullptr;
  };

  const std::string log_tag;

  std::string filter_name, stream_name;

  pw_filter* filter = nullptr;

  pw_stream* playback = nullptr;

  uint n_samples = 0U;

  uint rate = 0U;

  float sample_duration = 0.0F;

  bool bypass = false;

  bool filter_connected_to_pw = false;

  bool stream_connected_to_pw = false;

  bool post_messages = false;

  spa_hook playback_listener{};

  auto connect_filter_to_pw() -> bool;

  auto connect_stream_to_pw() -> bool;

  void disconnect_filter_from_pw();

  void disconnect_stream_from_pw();

  void setup();

  void process(std::span<float>& left_in, std::span<float>& right_in);

 private:
  uint node_id = 0U;

  float notification_time_window = 1.0F / 20.0F;  // seconds
  float notification_dt = 0.0F;

  PipeManager* pm = nullptr;

  spa_hook listener{};

  data pf_data = {};
};

#endif
