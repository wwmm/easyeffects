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

#include "equalizer.hpp"
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <sys/types.h>
#include <QApplication>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <format>
#include <map>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <utility>
#include <vector>
#include "db_manager.hpp"
#include "easyeffects_db_equalizer.h"
#include "easyeffects_db_equalizer_channel.h"
#include "equalizer_apo.hpp"
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
  const auto lv2_plugin_uri = "http://lsp-plug.in/plugins/lv2/para_equalizer_x32_lr";

  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>(lv2_plugin_uri);

  packageInstalled = lv2_wrapper->found_plugin;

  if (!packageInstalled) {
    util::debug(std::format("{}{} is not installed", log_tag, lv2_plugin_uri));
  }

  init_common_controls<db::Equalizer>(settings);

  BIND_LV2_PORT("mode", mode, setMode, db::Equalizer::modeChanged);
  BIND_LV2_PORT("bal", balance, setBalance, db::Equalizer::balanceChanged);
  BIND_LV2_PORT("frqs_l", pitchLeft, setPitchLeft, db::Equalizer::pitchLeftChanged);
  BIND_LV2_PORT("frqs_r", pitchRight, setPitchRight, db::Equalizer::pitchRightChanged);

  BIND_LV2_PORT_INVERTED_BOOL("clink", splitChannels, setSplitChannels, db::Equalizer::splitChannelsChanged);

  bind_left_bands();
  bind_right_bands();

  on_split_channels();

  /**
   * Using setProperty does not have the same performance as calling the proper
   * setter directly:
   * https://doc.qt.io/qt-6/properties.html
   * But it is the easiest thing to do in the case below.
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
  settings_left->disconnect();
  settings_right->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void Equalizer::reset() {
  settings->setDefaults();
  settings_left->setDefaults();
  settings_right->setDefaults();
}

// NOLINTNEXTLINE(readability-function-size,hicpp-function-size)
void Equalizer::bind_left_bands() {
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
}

// NOLINTNEXTLINE(readability-function-size,hicpp-function-size)
void Equalizer::bind_right_bands() {
  using namespace tags::equalizer;

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

  // When in unified mode we want settings applied to the left channel to
  // be propagated to the right channel database

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

void Equalizer::clear_data() {
  if (lv2_wrapper == nullptr) {
    return;
  }

  {
    std::scoped_lock<std::mutex> lock(data_mutex);

    lv2_wrapper->destroy_instance();
  }

  setup();
}

void Equalizer::setup() {
  if (rate == 0 || n_samples == 0) {
    // Some signals may be emitted before PipeWire calls our setup function
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  if (!lv2_wrapper->found_plugin) {
    return;
  }

  ready = false;

  lv2_wrapper->set_n_samples(n_samples);

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
  QMetaObject::invokeMethod(
      baseWorker,
      [this] {
        lv2_wrapper->create_instance(rate);

        std::scoped_lock<std::mutex> lock(data_mutex);

        ready = true;
      },
      Qt::QueuedConnection);
  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void Equalizer::process(std::span<float>& left_in,
                        std::span<float>& right_in,
                        std::span<float>& left_out,
                        std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    return;
  }

  if (!ready) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    if (output_gain != 1.0F) {
      apply_gain(left_out, right_out, output_gain);
    }

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

  // This plugin gives the latency in number of samples

  const auto lv = static_cast<uint>(lv2_wrapper->get_control_port_value("out_latency"));

  if (latency_n_frames != lv) {
    latency_n_frames = lv;

    latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(std::format("{}{} latency: {} s", log_tag, name.toStdString(), latency_value));

    update_filter_params();
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void Equalizer::process([[maybe_unused]] std::span<float>& left_in,
                        [[maybe_unused]] std::span<float>& right_in,
                        [[maybe_unused]] std::span<float>& left_out,
                        [[maybe_unused]] std::span<float>& right_out,
                        [[maybe_unused]] std::span<float>& probe_left,
                        [[maybe_unused]] std::span<float>& probe_right) {}

void Equalizer::sortBands() {
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

  std::vector<db::EqualizerChannel*> settings_channels{settings_left};

  if (settings->splitChannels()) {
    settings_channels.push_back(settings_right);
  }

  using namespace tags::equalizer;

  for (auto* channel : settings_channels) {
    std::multimap<double, struct EQ_Band> sorted_bands;

    for (int n = 0; n < used_bands; n++) {
      const auto f = channel->property(band_frequency[n].data()).value<double>();

      sorted_bands.emplace(
          std::pair<double, struct EQ_Band>(f, {.freq = f,
                                                .type = channel->property(band_type[n].data()).value<int>(),
                                                .mode = channel->property(band_mode[n].data()).value<int>(),
                                                .slope = channel->property(band_slope[n].data()).value<int>(),
                                                .gain = channel->property(band_gain[n].data()).value<double>(),
                                                .q = channel->property(band_q[n].data()).value<double>(),
                                                .width = channel->property(band_width[n].data()).value<double>(),
                                                .solo = channel->property(band_solo[n].data()).value<bool>(),
                                                .mute = channel->property(band_mute[n].data()).value<bool>()}));
    }

    for (int n = 0; const auto& p : sorted_bands) {
      channel->setProperty(band_frequency[n].data(), p.second.freq);
      channel->setProperty(band_type[n].data(), p.second.type);
      channel->setProperty(band_mode[n].data(), p.second.mode);
      channel->setProperty(band_slope[n].data(), p.second.slope);
      channel->setProperty(band_gain[n].data(), p.second.gain);
      channel->setProperty(band_q[n].data(), p.second.q);
      channel->setProperty(band_width[n].data(), p.second.width);
      channel->setProperty(band_solo[n].data(), p.second.solo);
      channel->setProperty(band_mute[n].data(), p.second.mute);

      n++;
    }
  }
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

    /**
     * Using setProperty does not have the same performance as calling the
     * proper setter directly:
     * https://doc.qt.io/qt-6/properties.html
     * But it is the easiest thing to do in the case below.
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

bool Equalizer::importApoPreset(const QList<QString>& url_list) {
  return std::ranges::any_of(url_list, [&](const auto& u) {
    const auto url = QUrl(u);

    if (url.isLocalFile()) {
      const auto path = std::filesystem::path{url.toLocalFile().toStdString()};

      if (apo::import_apo_preset(settings, settings_left, settings_right, path.string())) {
        return true;
      }
    }

    return false;
  });
}

bool Equalizer::importApoGraphicEqPreset(const QList<QString>& url_list) {
  return std::ranges::any_of(url_list, [&](const auto& u) {
    const auto url = QUrl(u);

    if (url.isLocalFile()) {
      const auto path = std::filesystem::path{url.toLocalFile().toStdString()};

      if (apo::import_graphiceq_preset(settings, settings_left, settings_right, path.string())) {
        return true;
      }
    }

    return false;
  });
}

bool Equalizer::exportApoPreset(const QString& url) {
  const auto u = QUrl(url);
  return apo::export_apo_preset(settings, settings_left, settings_right, u.toLocalFile().toStdString());
}
