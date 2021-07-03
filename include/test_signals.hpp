/*
 *  Copyright © 2017-2020 Wellington Wallace
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
#include <span>
#include "pipe_manager.hpp"

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
    struct port* out = nullptr;

    TestSignals* ts = nullptr;
  };

  std::string log_tag = "test signals: ";

  pw_filter* filter = nullptr;

  uint n_samples = 0;

  uint rate = 0;

  [[nodiscard]] auto get_node_id() const -> uint;

  void set_active(const bool& state) const;

 private:
  PipeManager* pm = nullptr;

  spa_hook listener{};

  data pf_data = {};

  uint node_id = 0;
};

#endif