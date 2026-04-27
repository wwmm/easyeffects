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

#include "autotune.hpp"
#include <lilv/lilv.h>
#include <lv2/atom/atom.h>
#include <lv2/atom/util.h>
#include <lv2/urid/urid.h>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <qtypes.h>
#include <QApplication>
#include <algorithm>
#include <format>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_autotune.h"
#include "lv2_macros.hpp"
#include "lv2_wrapper.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

Autotune::Autotune(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::autotune,
                 tags::plugin_package::Package::x42,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(db::Manager::self().get_plugin_db<DbAutotune>(
          pipe_type,
          tags::plugin_name::BaseName::autotune + "#" + instance_id)) {
  const auto lv2_plugin_uri = "http://gareus.org/oss/lv2/fat1";

  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>(lv2_plugin_uri);

  packageInstalled = lv2_wrapper->found_plugin;

  if (!packageInstalled) {
    util::debug(std::format("{}{} is not installed", log_tag, lv2_plugin_uri));
  }

  init_common_controls<DbAutotune>(settings);

  // Find the MIDI atom input port index
  for (const auto& port : lv2_wrapper->ports) {
    if (port.type == lv2::PortType::TYPE_ATOM && port.is_input) {
      midi_port_index = port.index;
      break;
    }
  }

  // UI mode: 0=Auto, 1=Manual. LV2 mode: 0=Auto, 1=MIDI, 2=Manual.
  {
    const float lv2_mode = settings->mode() == 0 ? 0.0F : 2.0F;
    lv2_wrapper->set_control_port_value("mode", lv2_mode);
    lv2_wrapper->sync_funcs.emplace_back([this]() {
      const float lv2_mode = lv2_wrapper->get_control_port_value("mode");
      settings->setMode(lv2_mode >= 1.5F ? 1 : 0);
    });
    connect(settings, &DbAutotune::modeChanged, [this]() {
      if (this == nullptr || settings == nullptr || lv2_wrapper == nullptr) {
        return;
      }
      lv2_wrapper->set_control_port_value("mode", settings->mode() == 0 ? 0.0F : 2.0F);
    });
  }
  BIND_LV2_PORT("channelf", channelFilter, setChannelFilter, DbAutotune::channelFilterChanged);
  BIND_LV2_PORT("tuning", tuning, setTuning, DbAutotune::tuningChanged);
  BIND_LV2_PORT("bias", bias, setBias, DbAutotune::biasChanged);
  BIND_LV2_PORT("filter", filter, setFilter, DbAutotune::filterChanged);
  BIND_LV2_PORT("corr", correction, setCorrection, DbAutotune::correctionChanged);
  BIND_LV2_PORT("offset", offset, setOffset, DbAutotune::offsetChanged);
  BIND_LV2_PORT("bendrange", bendRange, setBendRange, DbAutotune::bendRangeChanged);
  BIND_LV2_PORT("fastmode", fastMode, setFastMode, DbAutotune::fastModeChanged);

  BIND_LV2_PORT("m00", noteC, setNoteC, DbAutotune::noteCChanged);
  BIND_LV2_PORT("m01", noteCSharp, setNoteCSharp, DbAutotune::noteCSharpChanged);
  BIND_LV2_PORT("m02", noteD, setNoteD, DbAutotune::noteDChanged);
  BIND_LV2_PORT("m03", noteDSharp, setNoteDSharp, DbAutotune::noteDSharpChanged);
  BIND_LV2_PORT("m04", noteE, setNoteE, DbAutotune::noteEChanged);
  BIND_LV2_PORT("m05", noteF, setNoteF, DbAutotune::noteFChanged);
  BIND_LV2_PORT("m06", noteFSharp, setNoteFSharp, DbAutotune::noteFSharpChanged);
  BIND_LV2_PORT("m07", noteG, setNoteG, DbAutotune::noteGChanged);
  BIND_LV2_PORT("m08", noteGSharp, setNoteGSharp, DbAutotune::noteGSharpChanged);
  BIND_LV2_PORT("m09", noteA, setNoteA, DbAutotune::noteAChanged);
  BIND_LV2_PORT("m10", noteASharp, setNoteASharp, DbAutotune::noteASharpChanged);
  BIND_LV2_PORT("m11", noteB, setNoteB, DbAutotune::noteBChanged);
}

Autotune::~Autotune() {
  stop_worker();

  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void Autotune::reset() {
  settings->setDefaults();
}

void Autotune::clear_data() {
  if (lv2_wrapper == nullptr) {
    return;
  }

  {
    std::scoped_lock<std::mutex> lock(data_mutex);

    lv2_wrapper->destroy_instance();
  }

  setup();
}

void Autotune::setup() {
  if (rate == 0 || n_samples == 0) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);

  mono_buffer.resize(n_samples);

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

void Autotune::process(std::span<float>& left_in,
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

  // fat1 is mono — mix stereo to mono for processing
  for (size_t i = 0; i < left_in.size(); i++) {
    mono_buffer[i] = (left_in[i] + right_in[i]) * 0.5F;
  }

  auto mono_span = std::span<float>(mono_buffer);

  // Provide an empty MIDI atom sequence for the fat1 MIDI input port
  midi_in_buf.seq.atom.size = sizeof(LV2_Atom_Sequence_Body);
  midi_in_buf.seq.atom.type = lv2_wrapper->map_urid(LV2_ATOM__Sequence);
  midi_in_buf.seq.body.unit = 0;
  midi_in_buf.seq.body.pad = 0;
  lilv_instance_connect_port(lv2_wrapper->get_instance(), midi_port_index, &midi_in_buf);

  lv2_wrapper->connect_data_ports(mono_span, right_in, left_out, right_out);
  lv2_wrapper->run();

  // left_out now has the processed mono signal; copy to right_out
  std::ranges::copy(left_out, right_out.begin());

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  const auto lv = static_cast<uint>(lv2_wrapper->get_control_port_value("latency"));

  if (latency_n_frames != lv) {
    latency_n_frames = lv;

    latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(std::format("{}{} latency: {} s", log_tag, name.toStdString(), latency_value));

    update_filter_params();
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);

    pitch_error = lv2_wrapper->get_control_port_value("error");
  }
}

void Autotune::process([[maybe_unused]] std::span<float>& left_in,
                       [[maybe_unused]] std::span<float>& right_in,
                       [[maybe_unused]] std::span<float>& left_out,
                       [[maybe_unused]] std::span<float>& right_out,
                       [[maybe_unused]] std::span<float>& probe_left,
                       [[maybe_unused]] std::span<float>& probe_right) {}

auto Autotune::get_latency_seconds() -> float {
  return this->latency_value;
}

float Autotune::getPitchError() const {
  return pitch_error;
}
