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

#include <pipewire/context.h>
#include <pipewire/filter.h>
#include <pipewire/proxy.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <spa/utils/hook.h>
#include <sys/types.h>
#include <random>
#include <vector>
#include "pw_manager.hpp"

enum class TestSignalType { sine_wave, gaussian, pink };

class TestSignals : public QObject {
  Q_OBJECT

 public:
  TestSignals(pw::Manager* pipe_manager);
  TestSignals(const TestSignals&) = delete;
  auto operator=(const TestSignals&) -> TestSignals& = delete;
  TestSignals(const TestSignals&&) = delete;
  auto operator=(const TestSignals&&) -> TestSignals& = delete;
  ~TestSignals() override;

  static TestSignals& self(pw::Manager* pm) {
    static TestSignals ts = TestSignals(pm);
    return ts;
  }

  struct data;

  struct port {
    struct data* data;
  };

  struct data {
    struct port* out_left = nullptr;
    struct port* out_right = nullptr;

    TestSignals* ts = nullptr;
  };

  pw_filter* filter = nullptr;

  pw_filter_state state = PW_FILTER_STATE_UNCONNECTED;

  uint n_samples = 0U;

  uint rate = 0U;

  bool create_left_channel = true;

  bool create_right_channel = true;

  bool can_get_node_id = false;

  float sine_phase = 0.0F;

  float sine_frequency = 1000.0F;

  float pink_b0 = 0.0F;
  float pink_b1 = 0.0F;
  float pink_b2 = 0.0F;

  TestSignalType signal_type = TestSignalType::sine_wave;

  void set_state(const bool& state);

  void set_frequency(const float& value);

  [[nodiscard]] auto get_node_id() const -> uint;

  void set_active(const bool& state) const;

  auto white_noise() -> float;

  auto pink_noise() -> float;

 private:
  pw::Manager* pm = nullptr;

  spa_hook listener{};

  data pf_data = {};

  uint node_id = 0U;

  std::vector<pw_proxy*> list_proxies;

  std::random_device rd;

  std::mt19937 random_generator;

  std::normal_distribution<float> normal_distribution{0.0F, 0.3F};

  void set_channel(const int& value);
};
