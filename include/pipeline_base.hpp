/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PIPELINE_BASE_HPP
#define PIPELINE_BASE_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <memory>
#include <vector>
#include "compressor.hpp"
#include "deesser.hpp"
#include "equalizer.hpp"
#include "filter.hpp"
#include "gate.hpp"
#include "limiter.hpp"
#include "maximizer.hpp"
#include "pipe_manager.hpp"
#include "pitch.hpp"
#include "realtime_kit.hpp"
#include "reverb.hpp"
#include "rnnoise.hpp"
#include "stereo_tools.hpp"

class PipelineBase {
 public:
  PipelineBase(const std::string& tag, PipeManager* pipe_manager);
  PipelineBase(const PipelineBase&) = delete;
  auto operator=(const PipelineBase&) -> PipelineBase& = delete;
  PipelineBase(const PipelineBase&&) = delete;
  auto operator=(const PipelineBase&&) -> PipelineBase& = delete;
  virtual ~PipelineBase();

  std::string log_tag;

  bool playing = false;

  PipeManager* pm = nullptr;

  GstElement *pipeline = nullptr, *source = nullptr, *queue_src = nullptr, *sink = nullptr, *src_type = nullptr,
             *effects_bin = nullptr, *identity_in = nullptr, *identity_out = nullptr, *spectrum = nullptr,
             *spectrum_bin = nullptr, *spectrum_identity_in = nullptr, *spectrum_identity_out = nullptr,
             *global_level_meter = nullptr, *global_level_meter_bin = nullptr, *level_meter_identity_in = nullptr,
             *level_meter_identity_out = nullptr;

  GstBus* bus = nullptr;

  GSettings *settings = nullptr, *child_settings = nullptr, *spectrum_settings = nullptr;

  std::vector<std::string> plugins_order, plugins_order_old;
  std::map<std::string, GstElement*> plugins;

  std::unique_ptr<Limiter> limiter;
  std::unique_ptr<Compressor> compressor;
  std::unique_ptr<Filter> filter;
  std::unique_ptr<Equalizer> equalizer;
  std::unique_ptr<Reverb> reverb;
  std::unique_ptr<Gate> gate;
  std::unique_ptr<Deesser> deesser;
  std::unique_ptr<Pitch> pitch;
  std::unique_ptr<StereoTools> stereo_tools;
  std::unique_ptr<Maximizer> maximizer;
  std::unique_ptr<RNNoise> rnnoise;

  std::unique_ptr<RealtimeKit> rtkit;

  GstClockTime state_check_timeout = 5 * GST_SECOND;

  uint sampling_rate = 0U;

  bool resizing_spectrum = false;
  uint min_spectrum_freq = 20U;     // Hz
  uint max_spectrum_freq = 20000U;  // Hz
  int spectrum_threshold = -120;    // dB
  uint spectrum_nbands = 1600U, spectrum_nfreqs = 0U;
  float spline_f0 = 0.0F, spline_df = 0.0F;
  uint spectrum_start_index = 0U;
  std::vector<float> spectrum_freqs, spectrum_x_axis;
  std::vector<float> spectrum_mag_tmp, spectrum_mag;

  void do_bypass(const bool& value);
  auto bypass_state() -> bool;

  void enable_spectrum();
  void disable_spectrum();

  static auto get_peak(GstMessage* message) -> std::array<double, 2>;

  void set_input_node_id(const uint& id) const;
  auto get_input_node_id() -> uint;
  void set_output_node_id(const uint& id) const;
  auto get_output_node_id() -> uint;
  void set_null_pipeline();
  void update_pipeline_state();
  void get_latency();
  void init_spectrum();
  void update_spectrum_interval(const double& value) const;
  void set_latency();

  sigc::signal<void, std::vector<float>> new_spectrum;
  sigc::signal<void, int> new_latency;
  sigc::signal<void, std::array<double, 2>> global_output_level;
  sigc::signal<void, std::array<double, 2>> equalizer_input_level;
  sigc::signal<void, std::array<double, 2>> equalizer_output_level;
  sigc::signal<void, std::array<double, 2>> pitch_input_level;
  sigc::signal<void, std::array<double, 2>> pitch_output_level;
  sigc::signal<void, std::array<double, 2>> gate_input_level;
  sigc::signal<void, std::array<double, 2>> gate_output_level;
  sigc::signal<void, std::array<double, 2>> deesser_input_level;
  sigc::signal<void, std::array<double, 2>> deesser_output_level;
  sigc::signal<void, std::array<double, 2>> maximizer_input_level;
  sigc::signal<void, std::array<double, 2>> maximizer_output_level;
  sigc::signal<void, std::array<double, 2>> rnnoise_input_level;
  sigc::signal<void, std::array<double, 2>> rnnoise_output_level;

 protected:
  bool apps_want_to_play = false;
  std::string pipe_props = "node.group=1";

  void set_pipewiresrc_stream_props(const std::string& props) const;
  void set_pipewiresink_stream_props(const std::string& props) const;
  void set_sampling_rate(const uint& sampling_rate);

 private:
  GstElement* capsfilter = nullptr;

  sigc::connection timeout_connection;

  void init_spectrum_bin();
  void init_global_level_meter_bin();
  void init_effects_bin();

  auto get_required_plugin(const gchar* factoryname, const gchar* name) const -> GstElement*;
};

#endif
