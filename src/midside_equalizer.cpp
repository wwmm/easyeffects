/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * Copyright © 2026 Alessio Attilio
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

#include "midside_equalizer.hpp"
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
#include "easyeffects_db_equalizer_channel.h"
#include "easyeffects_db_midside_equalizer.h"
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

MidSideEqualizer::MidSideEqualizer(const std::string& tag,
                                   pw::Manager* pipe_manager,
                                   PipelineType pipe_type,
                                   QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::midsideEqualizer,
                 tags::plugin_package::Package::lsp,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(db::Manager::self().get_plugin_db<DbMidSideEqualizer>(
          pipe_type, tags::plugin_name::BaseName::midsideEqualizer + "#" + instance_id)),
      settings_mid(db::Manager::self().get_plugin_db<DbEqualizerChannel>(
          pipe_type, tags::plugin_name::BaseName::midsideEqualizer + "#" + instance_id + "#mid")),
      settings_side(db::Manager::self().get_plugin_db<DbEqualizerChannel>(
          pipe_type,
          tags::plugin_name::BaseName::midsideEqualizer + "#" + instance_id + "#side")) {
  const auto lv2_plugin_uri = "http://lsp-plug.in/plugins/lv2/para_equalizer_x32_ms";

  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>(lv2_plugin_uri);

  packageInstalled = lv2_wrapper->found_plugin;

  if (!packageInstalled) {
    util::debug(std::format("{}{} is not installed", log_tag, lv2_plugin_uri));
  }

  init_common_controls<DbMidSideEqualizer>(settings);

  BIND_LV2_PORT("mode", mode, setMode, DbMidSideEqualizer::modeChanged);
  BIND_LV2_PORT("bal", balance, setBalance, DbMidSideEqualizer::balanceChanged);
  BIND_LV2_PORT("frqs_m", pitchMid, setPitchMid, DbMidSideEqualizer::pitchMidChanged);
  BIND_LV2_PORT("frqs_s", pitchSide, setPitchSide, DbMidSideEqualizer::pitchSideChanged);
  BIND_LV2_PORT("decramp", decramp, setDecramp, DbMidSideEqualizer::decrampChanged);

  BIND_LV2_PORT_INVERTED_BOOL("clink", splitChannels, setSplitChannels, DbMidSideEqualizer::splitChannelsChanged);

  bind_mid_bands();
  bind_side_bands();

  on_split_channels();

  connect(settings, &DbMidSideEqualizer::numBandsChanged, [&]() {
    for (int n = 0; n < max_bands; n++) {
      if (n >= settings->numBands()) {  // turn off unused bands
        settings_mid->setProperty(tags::equalizer::band_type[n].data(), 0);

        if (settings->splitChannels()) {
          settings_side->setProperty(tags::equalizer::band_type[n].data(), 0);
        }
      }
    }
  });

  connect(settings, &DbMidSideEqualizer::splitChannelsChanged, [&]() { on_split_channels(); });
}

MidSideEqualizer::~MidSideEqualizer() {
  stop_worker();

  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();
  settings_mid->disconnect();
  settings_side->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void MidSideEqualizer::reset() {
  settings->setDefaults();
  settings_mid->setDefaults();
  settings_side->setDefaults();
}

// NOLINTNEXTLINE(readability-function-size,hicpp-function-size)
void MidSideEqualizer::bind_mid_bands() {
  using namespace tags::equalizer;

  BIND_BANDS_PROPERTY(settings_mid, ftm, Type);
  BIND_BANDS_PROPERTY(settings_mid, fmm, Mode);
  BIND_BANDS_PROPERTY(settings_mid, sm, Slope);
  BIND_BANDS_PROPERTY(settings_mid, xsm, Solo);
  BIND_BANDS_PROPERTY(settings_mid, xmm, Mute);
  BIND_BANDS_PROPERTY(settings_mid, fm, Frequency);
  BIND_BANDS_PROPERTY(settings_mid, qm, Q);
  BIND_BANDS_PROPERTY(settings_mid, wm, Width);
  BIND_BANDS_PROPERTY_DB(settings_mid, gm, Gain, false);
}

// NOLINTNEXTLINE(readability-function-size,hicpp-function-size)
void MidSideEqualizer::bind_side_bands() {
  using namespace tags::equalizer;

  BIND_BANDS_PROPERTY(settings_side, fts, Type);
  BIND_BANDS_PROPERTY(settings_side, fms, Mode);
  BIND_BANDS_PROPERTY(settings_side, ss, Slope);
  BIND_BANDS_PROPERTY(settings_side, xss, Solo);
  BIND_BANDS_PROPERTY(settings_side, xms, Mute);
  BIND_BANDS_PROPERTY(settings_side, fs, Frequency);
  BIND_BANDS_PROPERTY(settings_side, qs, Q);
  BIND_BANDS_PROPERTY(settings_side, ws, Width);
  BIND_BANDS_PROPERTY_DB(settings_side, gs, Gain, false);
}

// NOLINTNEXTLINE(readability-function-size,hicpp-function-size)
void MidSideEqualizer::on_split_channels() {
  if (settings->splitChannels()) {
    for (const auto& conn : unified_mode_connections) {
      QObject::disconnect(conn);
    }

    unified_mode_connections.clear();

    return;
  }

  using namespace tags::equalizer;

  // When in unified mode we want settings applied to the mid channel to
  // be propagated to the side channel database

  UNIFIED_BANDS_PROPERTY_BIND(settings_side, settings_mid, Type);
  UNIFIED_BANDS_PROPERTY_BIND(settings_side, settings_mid, Mode);
  UNIFIED_BANDS_PROPERTY_BIND(settings_side, settings_mid, Slope);
  UNIFIED_BANDS_PROPERTY_BIND(settings_side, settings_mid, Solo);
  UNIFIED_BANDS_PROPERTY_BIND(settings_side, settings_mid, Mute);
  UNIFIED_BANDS_PROPERTY_BIND(settings_side, settings_mid, Frequency);
  UNIFIED_BANDS_PROPERTY_BIND(settings_side, settings_mid, Q);
  UNIFIED_BANDS_PROPERTY_BIND(settings_side, settings_mid, Width);
  UNIFIED_BANDS_PROPERTY_BIND(settings_side, settings_mid, Gain);
}

void MidSideEqualizer::clear_data() {
  if (lv2_wrapper == nullptr) {
    return;
  }

  {
    std::scoped_lock<std::mutex> lock(data_mutex);

    lv2_wrapper->destroy_instance_locked();
  }

  setup();
}

void MidSideEqualizer::setup() {
  if (rate == 0 || n_samples == 0) {
    // Some signals may be emitted before PipeWire calls our setup function
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);

  if (lv2_wrapper->has_instance() && rate == lv2_wrapper->get_rate()) {
    return;
  }

  ready = false;

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

void MidSideEqualizer::process(std::span<float>& left_in,
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

void MidSideEqualizer::process([[maybe_unused]] std::span<float>& left_in,
                                [[maybe_unused]] std::span<float>& right_in,
                                [[maybe_unused]] std::span<float>& left_out,
                                [[maybe_unused]] std::span<float>& right_out,
                                [[maybe_unused]] std::span<float>& probe_left,
                                [[maybe_unused]] std::span<float>& probe_right) {}

void MidSideEqualizer::sortBands() {
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

  std::vector<DbEqualizerChannel*> settings_channels{settings_mid};

  if (settings->splitChannels()) {
    settings_channels.push_back(settings_side);
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

auto MidSideEqualizer::get_latency_seconds() -> float {
  return latency_value;
}

void MidSideEqualizer::flatResponse() {
  RESET_BANDS_PROPERTY(settings_mid, Gain);
  RESET_BANDS_PROPERTY(settings_side, Gain);
}

void MidSideEqualizer::calculateFrequencies() {
  constexpr double min_freq = 20.0;
  constexpr double max_freq = 20000.0;

  double freq0 = min_freq;
  double freq1 = 0.0;

  const double step = std::pow(max_freq / min_freq, 1.0 / static_cast<double>(settings->numBands()));

  for (int n = 0; n < settings->numBands(); n++) {
    freq1 = freq0 * step;

    const double freq = freq0 + (0.5 * (freq1 - freq0));
    const double width = freq1 - freq0;
    const double q = freq / width;

    settings_mid->setProperty(tags::equalizer::band_frequency[n].data(), freq);
    settings_mid->setProperty(tags::equalizer::band_q[n].data(), q);

    settings_side->setProperty(tags::equalizer::band_frequency[n].data(), freq);
    settings_side->setProperty(tags::equalizer::band_q[n].data(), q);

    freq0 = freq1;
  }

  RESET_BANDS_PROPERTY(settings_mid, Width);
  RESET_BANDS_PROPERTY(settings_side, Width);
}

bool MidSideEqualizer::importApoPreset(const QList<QString>& url_list) {
  return std::ranges::any_of(url_list, [&](const auto& u) {
    const auto url = QUrl(u);

    if (url.isLocalFile()) {
      const auto path = std::filesystem::path{url.toLocalFile().toStdString()};

      if (apo::import_apo_preset(settings, settings_mid, settings_side, path.string())) {
        return true;
      }
    }

    return false;
  });
}

bool MidSideEqualizer::importApoGraphicEqPreset(const QList<QString>& url_list) {
  return std::ranges::any_of(url_list, [&](const auto& u) {
    const auto url = QUrl(u);

    if (url.isLocalFile()) {
      const auto path = std::filesystem::path{url.toLocalFile().toStdString()};

      if (apo::import_graphiceq_preset(settings, settings_mid, settings_side, path.string())) {
        return true;
      }
    }

    return false;
  });
}

bool MidSideEqualizer::exportApoPreset(const QString& url) {
  const auto u = QUrl(url);
  return apo::export_apo_preset(settings, settings_mid, settings_side, u.toLocalFile().toStdString());
}
