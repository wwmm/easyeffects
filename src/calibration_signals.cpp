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

#include "calibration_signals.hpp"
#include <glibmm/main.h>
#include <boost/math/interpolators/cardinal_cubic_b_spline.hpp>
#include "gst/gstelement.h"
#include "util.hpp"

namespace {

void on_message_state_changed(const GstBus* gst_bus, GstMessage* message, CalibrationSignals* cs) {
  if (GST_OBJECT_NAME(message->src) == std::string("pipeline")) {
    GstState old_state = GST_STATE_VOID_PENDING;
    GstState new_state = GST_STATE_VOID_PENDING;

    gst_message_parse_state_changed(message, &old_state, &new_state, nullptr);

    util::debug(cs->log_tag + gst_element_state_get_name(new_state));
  }
}

void on_message_element(const GstBus* gst_bus, GstMessage* message, CalibrationSignals* cs) {
  if (GST_OBJECT_NAME(message->src) == std::string("spectrum")) {
    const GstStructure* s = gst_message_get_structure(message);

    const GValue* magnitudes = nullptr;

    magnitudes = gst_structure_get_value(s, "magnitude");

    for (uint n = 0U; n < cs->spectrum_freqs.size(); n++) {
      cs->spectrum_mag_tmp[n] = g_value_get_float(gst_value_list_get_value(magnitudes, n));
    }

    boost::math::interpolators::cardinal_cubic_b_spline<float> spline(
        cs->spectrum_mag_tmp.begin(), cs->spectrum_mag_tmp.end(), cs->spline_f0, cs->spline_df);

    for (uint n = 0U; n < cs->spectrum_mag.size(); n++) {
      cs->spectrum_mag[n] = spline(cs->spectrum_x_axis[n]);
    }

    auto min_mag = *std::min_element(cs->spectrum_mag.begin(), cs->spectrum_mag.end());
    auto max_mag = *std::max_element(cs->spectrum_mag.begin(), cs->spectrum_mag.end());

    if (max_mag > min_mag) {
      for (float& v : cs->spectrum_mag) {
        v = (v - min_mag) / (max_mag - min_mag);
      }

      Glib::signal_idle().connect_once([=] { cs->new_spectrum.emit(cs->spectrum_mag); });
    }
  }
}

}  // namespace

CalibrationSignals::CalibrationSignals() {
  gst_init(nullptr, nullptr);

  pipeline = gst_pipeline_new("pipeline");

  bus = gst_element_get_bus(pipeline);

  gst_bus_add_signal_watch(bus);

  // bus callbacks

  g_signal_connect(bus, "message::state-changed", G_CALLBACK(on_message_state_changed), this);
  g_signal_connect(bus, "message::element", G_CALLBACK(on_message_element), this);

  // creating elements

  source = gst_element_factory_make("audiotestsrc", "source");
  sink = gst_element_factory_make("pipewiresink", "sink");
  spectrum = gst_element_factory_make("spectrum", "spectrum");

  auto* capsfilter = gst_element_factory_make("capsfilter", nullptr);
  auto* queue = gst_element_factory_make("queue", nullptr);

  // building the pipeline

  gst_bin_add_many(GST_BIN(pipeline), source, capsfilter, queue, spectrum, sink, nullptr);

  gst_element_link_many(source, capsfilter, queue, spectrum, sink, nullptr);

  // setting a few parameters

  auto* props = gst_structure_from_string("props,application.name=PulseEffectsCalibration", nullptr);

  auto* caps = gst_caps_from_string("audio/x-raw,format=F32LE,channels=2,rate=48000");

  g_object_set(source, "wave", 0, nullptr);  // sine
  g_object_set(capsfilter, "caps", caps, nullptr);
  g_object_set(queue, "silent", 1, nullptr);
  g_object_set(spectrum, "bands", spectrum_nbands, nullptr);
  g_object_set(spectrum, "threshold", spectrum_threshold, nullptr);
  g_object_set(sink, "stream-properties", props, nullptr);
  g_object_set(sink, "sync", 0, nullptr);

  gst_structure_free(props);
  gst_caps_unref(caps);

  // init spectrum

  for (uint n = 0U; n < spectrum_nbands; n++) {
    auto f = 48000.0 * (0.5 * n + 0.25) / spectrum_nbands;

    if (f > max_spectrum_freq) {
      break;
    }

    if (f > min_spectrum_freq) {
      spectrum_freqs.emplace_back(f);
    }
  }

  spectrum_mag_tmp.resize(spectrum_freqs.size());

  spectrum_x_axis = util::logspace(log10(static_cast<float>(min_spectrum_freq)),
                                   log10(static_cast<float>(max_spectrum_freq)), spectrum_npoints);

  spectrum_mag.resize(spectrum_npoints);

  spline_f0 = spectrum_freqs[0];
  spline_df = spectrum_freqs[1] - spectrum_freqs[0];
}

CalibrationSignals::~CalibrationSignals() {
  gst_element_set_state(pipeline, GST_STATE_NULL);

  gst_object_unref(bus);
  gst_object_unref(pipeline);

  util::debug(log_tag + "destroyed");
}

void CalibrationSignals::start() const {
  gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void CalibrationSignals::stop() const {
  gst_element_set_state(pipeline, GST_STATE_NULL);
}

void CalibrationSignals::set_freq(const double& value) const {
  g_object_set(source, "freq", value, nullptr);
}

void CalibrationSignals::set_volume(const double& value) const {
  g_object_set(source, "volume", value, nullptr);
}
