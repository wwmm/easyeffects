/*
 *  Copyright © 2017-2025 Wellington Wallace
 *
 *  This file is part of Easy Effects.
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

#include "equalizer.hpp"
#include <qobject.h>
#include <sys/types.h>
#include <algorithm>
#include <cmath>
#include <memory>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_equalizer.h"
#include "easyeffects_db_equalizer_channel.h"
#include "equalizer_macros.hpp"
#include "lv2_macros.hpp"
#include "lv2_wrapper.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_equalizer.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

using namespace std::string_literals;

Equalizer::Equalizer(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::equalizer,
                 tags::plugin_package::Package::lsp,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(
          db::Manager::self().get_plugin_db<db::Equalizer>(pipe_type,
                                                           tags::plugin_name::BaseName::equalizer + "#" + instance_id)),
      settings_left(db::Manager::self().get_plugin_db<db::EqualizerChannel>(
          pipe_type,
          tags::plugin_name::BaseName::equalizer + "#" + instance_id + "#left")),
      settings_right(db::Manager::self().get_plugin_db<db::EqualizerChannel>(
          pipe_type,
          tags::plugin_name::BaseName::equalizer + "#" + instance_id + "#right")) {
  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>("http://lsp-plug.in/plugins/lv2/para_equalizer_x32_lr");

  package_installed = lv2_wrapper->found_plugin;

  if (!package_installed) {
    util::debug(log_tag + "http://lsp-plug.in/plugins/lv2/para_equalizer_x32_lr is not installed");
  }

  init_common_controls<db::Equalizer>(settings);

  BIND_LV2_PORT("mode", mode, setMode, db::Equalizer::modeChanged);
  BIND_LV2_PORT("bal", balance, setBalance, db::Equalizer::balanceChanged);
  BIND_LV2_PORT("frqs_l", pitchLeft, setPitchLeft, db::Equalizer::pitchLeftChanged);
  BIND_LV2_PORT("frqs_r", pitchRight, setPitchRight, db::Equalizer::pitchRightChanged);

  bind_bands();

  on_split_channels();

  /*
    Using setProperty does not have the same performance as calling the proper setter directly:
    https://doc.qt.io/qt-6/properties.html
    . But it is the easiest thing to do in the case below.
  */

  connect(settings, &db::Equalizer::numBandsChanged, [&]() {
    for (int n = 0; n < max_bands; n++) {
      if (n >= settings->numBands()) {  // turn off unused bands
        settings_left->setProperty(tags::equalizer::band_type[n].data(), 0);

        if (settings->splitChannels()) {
          settings_right->setProperty(tags::equalizer::band_type[n].data(), 0);
        }
      }
    }
  });

  connect(settings, &db::Equalizer::splitChannelsChanged, [&]() { on_split_channels(); });
}

Equalizer::~Equalizer() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(log_tag + name.toStdString() + " destroyed");
}

void Equalizer::reset() {
  settings->setDefaults();
  settings_left->setDefaults();
  settings_right->setDefaults();
}

// NOLINTNEXTLINE(readability-function-size,hicpp-function-size)
void Equalizer::bind_bands() {
  using namespace tags::equalizer;

  BIND_BANDS_PROPERTY(settings_left, ftl, Type);
  BIND_BANDS_PROPERTY(settings_left, fml, Mode);
  BIND_BANDS_PROPERTY(settings_left, sl, Slope);
  BIND_BANDS_PROPERTY(settings_left, xsl, Solo);
  BIND_BANDS_PROPERTY(settings_left, xml, Mute);
  BIND_BANDS_PROPERTY(settings_left, fl, Frequency);
  BIND_BANDS_PROPERTY(settings_left, ql, Q);
  BIND_BANDS_PROPERTY(settings_left, wl, Width);
  BIND_BANDS_PROPERTY_DB(settings_left, gl, Gain, false);

  BIND_BANDS_PROPERTY(settings_right, ftr, Type);
  BIND_BANDS_PROPERTY(settings_right, fmr, Mode);
  BIND_BANDS_PROPERTY(settings_right, sr, Slope);
  BIND_BANDS_PROPERTY(settings_right, xsr, Solo);
  BIND_BANDS_PROPERTY(settings_right, xmr, Mute);
  BIND_BANDS_PROPERTY(settings_right, fr, Frequency);
  BIND_BANDS_PROPERTY(settings_right, qr, Q);
  BIND_BANDS_PROPERTY(settings_right, wr, Width);
  BIND_BANDS_PROPERTY_DB(settings_right, gr, Gain, false);
}

// NOLINTNEXTLINE(readability-function-size,hicpp-function-size)
void Equalizer::on_split_channels() {
  if (settings->splitChannels()) {
    for (const auto& conn : unified_mode_connections) {
      QObject::disconnect(conn);
    }

    unified_mode_connections.clear();

    return;
  }

  using namespace tags::equalizer;

  /*
    When in unified mode we want settings applied to the left channel to be propagated to the right channel
    database
  */

  UNIFIED_BANDS_PROPERTY_BIND(settings_right, settings_left, Type);
  UNIFIED_BANDS_PROPERTY_BIND(settings_right, settings_left, Mode);
  UNIFIED_BANDS_PROPERTY_BIND(settings_right, settings_left, Slope);
  UNIFIED_BANDS_PROPERTY_BIND(settings_right, settings_left, Solo);
  UNIFIED_BANDS_PROPERTY_BIND(settings_right, settings_left, Mute);
  UNIFIED_BANDS_PROPERTY_BIND(settings_right, settings_left, Frequency);
  UNIFIED_BANDS_PROPERTY_BIND(settings_right, settings_left, Q);
  UNIFIED_BANDS_PROPERTY_BIND(settings_right, settings_left, Width);
  UNIFIED_BANDS_PROPERTY_BIND(settings_right, settings_left, Gain);
}

void Equalizer::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);

  if (lv2_wrapper->get_rate() != rate) {
    lv2_wrapper->create_instance(rate);
  }
}

void Equalizer::process(std::span<float>& left_in,
                        std::span<float>& right_in,
                        std::span<float>& left_out,
                        std::span<float>& right_out) {
  if (!lv2_wrapper->found_plugin || !lv2_wrapper->has_instance() || bypass) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  lv2_wrapper->connect_data_ports(left_in, right_in, left_out, right_out);
  lv2_wrapper->run();

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  /*
    This plugin gives the latency in number of samples
  */

  const auto lv = static_cast<uint>(lv2_wrapper->get_control_port_value("out_latency"));

  if (latency_n_frames != lv) {
    latency_n_frames = lv;

    latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(log_tag + name.toStdString() + " latency: " + util::to_string(latency_value, "") + " s");

    update_filter_params();
  }

  get_peaks(left_in, right_in, left_out, right_out);
}

void Equalizer::process([[maybe_unused]] std::span<float>& left_in,
                        [[maybe_unused]] std::span<float>& right_in,
                        [[maybe_unused]] std::span<float>& left_out,
                        [[maybe_unused]] std::span<float>& right_out,
                        [[maybe_unused]] std::span<float>& probe_left,
                        [[maybe_unused]] std::span<float>& probe_right) {}

void Equalizer::sort_bands() {
  struct EQ_Band {
    double freq;
    int type;
    int mode;
    int slope;
    double gain;
    double q;
    double width;
    bool solo;
    bool mute;
  };

  const auto used_bands = settings->numBands();
  if (used_bands < 1 || used_bands > max_bands) {
    return;
  }

  // std::vector<GSettings*> settings_channels{settings_left};

  // if (settings->splitChannels()) {
  //   settings_channels.push_back(settings_right);
  // }

  // using namespace tags::equalizer;

  // for (auto* channel : settings_channels) {
  //   std::multimap<double, struct EQ_Band> sorted_bands;

  //   for (int n = 0; n < used_bands; n++) {
  //     const auto f = g_settings_get_double(channel, band_frequency[n].data());

  //     sorted_bands.emplace(
  //         std::pair<double, struct EQ_Band>(f, {.freq = f,
  //                                               .type = g_settings_get_enum(channel, band_type[n].data()),
  //                                               .mode = g_settings_get_enum(channel, band_mode[n].data()),
  //                                               .slope = g_settings_get_enum(channel, band_slope[n].data()),
  //                                               .gain = g_settings_get_double(channel, band_gain[n].data()),
  //                                               .q = g_settings_get_double(channel, band_q[n].data()),
  //                                               .width = g_settings_get_double(channel, band_width[n].data()),
  //                                               .solo = g_settings_get_boolean(channel, band_solo[n].data()),
  //                                               .mute = g_settings_get_boolean(channel, band_mute[n].data())}));
  //   }

  //   for (int n = 0; const auto& p : sorted_bands) {
  //     g_settings_set_double(channel, band_frequency[n].data(), p.second.freq);
  //     g_settings_set_enum(channel, band_type[n].data(), p.second.type);
  //     g_settings_set_enum(channel, band_mode[n].data(), p.second.mode);
  //     g_settings_set_enum(channel, band_slope[n].data(), p.second.slope);
  //     g_settings_set_double(channel, band_gain[n].data(), p.second.gain);
  //     g_settings_set_double(channel, band_q[n].data(), p.second.q);
  //     g_settings_set_double(channel, band_width[n].data(), p.second.width);
  //     g_settings_set_boolean(channel, band_solo[n].data(), p.second.solo);
  //     g_settings_set_boolean(channel, band_mute[n].data(), p.second.mute);
  //     n++;
  //   }
  // }
}

auto Equalizer::get_latency_seconds() -> float {
  return latency_value;
}

void Equalizer::flatResponse() {
  RESET_BANDS_PROPERTY(settings_left, Gain);
  RESET_BANDS_PROPERTY(settings_right, Gain);
}

void Equalizer::calculateFrequencies() {
  constexpr double min_freq = 20.0;
  constexpr double max_freq = 20000.0;

  double freq0 = min_freq;
  double freq1 = 0.0;

  // code taken from gstreamer equalizer sources: gstiirequalizer.c
  // function: gst_iir_equalizer_compute_frequencies

  const double step = std::pow(max_freq / min_freq, 1.0 / static_cast<double>(settings->numBands()));

  for (int n = 0; n < settings->numBands(); n++) {
    freq1 = freq0 * step;

    const double freq = freq0 + (0.5 * (freq1 - freq0));
    const double width = freq1 - freq0;
    const double q = freq / width;

    // std::cout << n << "\t" << freq << "\t" << width << std::endl;

    /*
      Using setProperty does not have the same performance as calling the proper setter directly:
      https://doc.qt.io/qt-6/properties.html
      . But it is the easiest thing to do in the case below.
    */

    settings_left->setProperty(tags::equalizer::band_frequency[n].data(), freq);
    settings_left->setProperty(tags::equalizer::band_q[n].data(), q);

    settings_right->setProperty(tags::equalizer::band_frequency[n].data(), freq);
    settings_right->setProperty(tags::equalizer::band_q[n].data(), q);

    freq0 = freq1;
  }

  RESET_BANDS_PROPERTY(settings_left, Width);
  RESET_BANDS_PROPERTY(settings_right, Width);
}
