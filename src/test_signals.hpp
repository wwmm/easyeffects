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
#include <qjsengine.h>
#include <qobject.h>
#include <qqmlengine.h>
#include <qqmlintegration.h>
#include <qtimer.h>
#include <qtmetamacros.h>
#include <spa/utils/hook.h>
#include <sys/types.h>
#include <atomic>
#include <random>
#include <vector>
#include "pw_manager.hpp"

enum class TestSignalType { sine_wave, gaussian, pink, silence };

class TestSignals : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(TestTone)
  QML_SINGLETON
  Q_PROPERTY(bool testing READ testing NOTIFY testingChanged)

 public:
  explicit TestSignals(pw::Manager* pipe_manager);
  TestSignals(const TestSignals&) = delete;
  auto operator=(const TestSignals&) -> TestSignals& = delete;
  TestSignals(const TestSignals&&) = delete;
  auto operator=(const TestSignals&&) -> TestSignals& = delete;
  ~TestSignals() override;

  static TestSignals& self(pw::Manager* pm) {
    static TestSignals ts = TestSignals(pm);
    return ts;
  }

  static TestSignals* create(QQmlEngine* engine, QJSEngine* js_engine) {
    Q_UNUSED(js_engine)
    auto& instance = self(&pw::Manager::self());
    Q_ASSERT(engine->thread() == instance.thread());
    QJSEngine::setObjectOwnership(&instance, QJSEngine::CppOwnership);
    return &instance;
  }

  [[nodiscard]] bool testing() const { return hearing_test; }

  Q_INVOKABLE bool beginTest();
  Q_INVOKABLE bool playTone(int frequency, int channel, double level);
  Q_INVOKABLE void stopTone();
  Q_INVOKABLE void endTest();

 Q_SIGNALS:
  void testingChanged();

 public:
  std::atomic<bool> hearing_test{false};
  std::atomic<float> hearing_amplitude{0.0F};
  std::atomic<bool> reset_phase{false};
  float current_amplitude = 0.0F;

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

  std::atomic<bool> create_left_channel{true};

  std::atomic<bool> create_right_channel{true};

  bool can_get_node_id = false;

  float sine_phase = 0.0F;

  std::atomic<float> sine_frequency{1000.0F};

  float pink_b0 = 0.0F;
  float pink_b1 = 0.0F;
  float pink_b2 = 0.0F;

  std::atomic<TestSignalType> signal_type{TestSignalType::sine_wave};

  void set_state(const bool& state);

  void set_frequency(const float& value);

  [[nodiscard]] auto get_node_id() const -> uint;

  void set_active(const bool& state) const;

  auto white_noise() -> float;

  auto pink_noise() -> float;

 private:
  pw::Manager* pm = nullptr;

  QTimer tone_timer;
  std::vector<pw_proxy*> hearing_links;

  spa_hook listener{};

  data pf_data = {};

  uint node_id = 0U;

  std::vector<pw_proxy*> list_proxies;

  std::random_device rd;

  std::mt19937 random_generator;

  std::normal_distribution<float> normal_distribution{0.0F, 0.3F};

  void set_channel(const int& value);
};
