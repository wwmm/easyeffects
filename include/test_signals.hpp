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

#ifndef TEST_SIGNALS_HPP
#define TEST_SIGNALS_HPP

#include <pipewire/filter.h>
#include <numbers>
#include <random>
#include <span>
#include "pipe_manager.hpp"

enum class TestSignalType { sine_wave, gaussian, pink };

class TestSignals {
 public:
  TestSignals(PipeManager* pipe_manager);
  TestSignals(const TestSignals&) = delete;
  auto operator=(const TestSignals&) -> TestSignals& = delete;
  TestSignals(const TestSignals&&) = delete;
  auto operator=(const TestSignals&&) -> TestSignals& = delete;
  virtual ~TestSignals();

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

  uint n_samples = 0U;

  uint rate = 0U;

  bool create_left_channel = true;

  bool create_right_channel = true;

  float sine_phase = 0.0F;

  float sine_frequency = 1000.0F;

  TestSignalType signal_type = TestSignalType::sine_wave;

  void set_state(const bool& state);

  void set_frequency(const float& value);

  [[nodiscard]] auto get_node_id() const -> uint;

  void set_active(const bool& state) const;

  void set_signal_type(const TestSignalType& value);

  auto white_noise() -> float;

 private:
  inline static const std::string log_tag = "test signals: ";

  PipeManager* pm = nullptr;

  spa_hook listener{};

  data pf_data = {};

  uint node_id = 0U;

  std::vector<pw_proxy*> list_proxies;

  std::random_device rd{};

  std::mt19937 random_generator;

  std::normal_distribution<float> normal_distribution{0.0F, 0.3F};
};

#endif
