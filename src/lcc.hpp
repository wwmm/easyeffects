/**
 * Copyright © 2017-2026 Wellington Wallace
 * Crosstalk Canceller plugin developed by Antti S. Lankila <alankila@bel.fi>
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

#include <cmath>
#include <span>
#include <string>
#include <vector>
#include "easyeffects_db_lcc.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"

/**
 * Some standard filters from "Cookbook formulae for audio EQ biquad filter coefficients"
 * by Robert Bristow-Johnson  <rbj@audioimagination.com>, adapted for EasyEffects.
 */
class Biquad {
 private:
  float fa1 = 0;
  float fa2 = 0;
  float fb0 = 1;
  float fb1 = 0;
  float fb2 = 0;

  float x1 = 0;
  float x2 = 0;
  float y1 = 0;
  float y2 = 0;

  void set_coefficients(double a0, double a1, double a2, double b0, double b1, double b2) {
    fa1 = static_cast<float>(a1 / a0);
    fa2 = static_cast<float>(a2 / a0);
    fb0 = static_cast<float>(b0 / a0);
    fb1 = static_cast<float>(b1 / a0);
    fb2 = static_cast<float>(b2 / a0);
  }

 public:
  void set_allpass(double center_frequency, double sampling_frequency, double quality) {
    auto w0 = 2 * M_PI * center_frequency / sampling_frequency;
    auto alpha = std::sin(w0) / (2 * quality);
    auto b0 = 1 - alpha;
    auto b1 = -2 * std::cos(w0);
    auto b2 = 1 + alpha;
    auto a0 = 1 + alpha;
    auto a1 = -2 * std::cos(w0);
    auto a2 = 1 - alpha;
    set_coefficients(a0, a1, a2, b0, b1, b2);
  }

  void set_low_pass(double center_frequency, double sampling_frequency, double quality) {
    auto w0 = 2 * M_PI * center_frequency / sampling_frequency;
    auto alpha = std::sin(w0) / (2 * quality);

    auto b0 = (1 - std::cos(w0)) / 2;
    auto b1 = 1 - std::cos(w0);
    auto b2 = (1 - std::cos(w0)) / 2;
    auto a0 = 1 + alpha;
    auto a1 = -2 * std::cos(w0);
    auto a2 = 1 - alpha;
    set_coefficients(a0, a1, a2, b0, b1, b2);
  }

  void set_high_pass(double center_frequency, double sampling_frequency, double quality) {
    auto w0 = 2 * M_PI * center_frequency / sampling_frequency;
    auto alpha = std::sin(w0) / (2 * quality);
    auto b0 = (1 + std::cos(w0)) / 2;
    auto b1 = -(1 + std::cos(w0));
    auto b2 = (1 + std::cos(w0)) / 2;
    auto a0 = 1 + alpha;
    auto a1 = -2 * std::cos(w0);
    auto a2 = 1 - alpha;
    set_coefficients(a0, a1, a2, b0, b1, b2);
  }

  void set_high_shelf(double center_frequency, double sampling_frequency, double db_gain, double quality) {
    auto w0 = 2 * M_PI * center_frequency / sampling_frequency;
    auto A = std::pow(10, db_gain / 40);
    auto alpha = std::sin(w0) / (2 * quality);

    auto b0 = A * ((A + 1) + ((A - 1) * std::cos(w0)) + (2 * std::sqrt(A) * alpha));
    auto b1 = -2 * A * ((A - 1) + ((A + 1) * std::cos(w0)));
    auto b2 = A * ((A + 1) + ((A - 1) * std::cos(w0)) - (2 * std::sqrt(A) * alpha));
    auto a0 = (A + 1) - ((A - 1) * std::cos(w0)) + (2 * std::sqrt(A) * alpha);
    auto a1 = 2 * ((A - 1) - ((A + 1) * std::cos(w0)));
    auto a2 = (A + 1) - ((A - 1) * std::cos(w0)) - (2 * std::sqrt(A) * alpha);
    set_coefficients(a0, a1, a2, b0, b1, b2);
  }

  void set_peaking_band(double center_frequency, double sampling_frequency, double db_gain, double quality) {
    auto w0 = 2 * M_PI * center_frequency / sampling_frequency;
    auto A = std::pow(10, db_gain / 40);
    auto alpha = std::sin(w0) / (2 * quality);

    auto b0 = 1 + (alpha * A);
    auto b1 = -2 * std::cos(w0);
    auto b2 = 1 - (alpha * A);
    auto a0 = 1 + (alpha / A);
    auto a1 = -2 * std::cos(w0);
    auto a2 = 1 - (alpha / A);
    set_coefficients(a0, a1, a2, b0, b1, b2);
  }

  float process(float x0) {
    auto y0 = (fb0 * x0) + (fb1 * x1) + (fb2 * x2) - (y1 * fa1) - (y2 * fa2);

    y2 = y1;
    y1 = y0;

    x2 = x1;
    x1 = x0;

    return y0;
  }
};

class FilterState {
 private:
  std::vector<float> data;
  size_t data_index = 0;

  /* Feedback correction filters*/
  Biquad f1;
  Biquad f2;
  Biquad f3;
  Biquad f4;
  Biquad f5;
  Biquad f6;

 public:
  /**
   * Set up filtering line for specific delay and configure filters with sample rate.
   */
  void configure(double delay_us, double rate) {
    /* Configure delay line for the appropriate length (full sample precision only) */
    if (auto samples = static_cast<size_t>(std::round(delay_us / 1.0e6 * rate)); data.size() != samples) {
      data.resize(samples);
      data_index = 0;
    }

    /*
     * Standard kemar head fixture, digitized
     * from picture at https://www.intechopen.com/chapters/45612
     *
     * The contralateral HRTF is what is perceived of the sound traversing around the head,
     * but the correction audio is received with sensitivity of the ipsilateral HRTF. Thus,
     * the magnitudes required must be subtracted: contra - ipsi, and an equalization must
     * reproduce that shape.
     *
     * This EQ was fitted in REW with -1 dB fixed offset between ipsi and contra.
     *
     * Filter  1: ON  LP Q     Fc    3245 Hz  Q  0.710
     * Filter  2: ON  PK       Fc   688.0 Hz  Gain  -2.90 dB  Q  1.000
     * Filter  3: ON  PK       Fc    1066 Hz  Gain  -6.40 dB  Q  3.352
     * Filter  4: ON  PK       Fc    2190 Hz  Gain  -5.60 dB  Q  2.037
     * Filter  5: ON  PK       Fc    3792 Hz  Gain  -4.50 dB  Q  3.232
     */
    f1.set_high_pass(250, rate, 0.710);
    f2.set_low_pass(3245, rate, 0.710);
    f3.set_peaking_band(688, rate, -2.9, 1.000);
    f4.set_peaking_band(1066, rate, -6.4, 3.352);
    f5.set_peaking_band(2190, rate, -5.6, 2.037);
    f6.set_peaking_band(3792, rate, -4.5, 3.232);
  }

  /**
   * Retrieve sample from the delay line that was set
   * some number of samples ago.
   *
   * @return historical sample from delay line
   */
  float get_sample() { return data[data_index]; }

  /**
   * Put a new sample into the delay line
   *
   * @param sample the sample to store
   */
  void put_sample(float sample) {
    sample = f1.process(sample);
    sample = f2.process(sample);
    sample = f3.process(sample);
    sample = f4.process(sample);
    sample = f5.process(sample);
    sample = f6.process(sample);
    data[data_index] = sample;
    data_index = (data_index + 1) % data.size();
  }
};

class Lcc : public PluginBase {
  Q_OBJECT

 public:
  Lcc(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  Lcc(const Lcc&) = delete;
  auto operator=(const Lcc&) -> Lcc& = delete;
  Lcc(const Lcc&&) = delete;
  auto operator=(const Lcc&&) -> Lcc& = delete;
  ~Lcc() override;

  void reset() override;

  void clear_data() override;

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

 private:
  std::vector<float> data;

  FilterState a;
  FilterState b;

  db::Lcc* settings = nullptr;
};
