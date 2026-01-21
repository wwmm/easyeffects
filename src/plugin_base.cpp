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

#include "plugin_base.hpp"
#include <pipewire/filter.h>
#include <pipewire/keys.h>
#include <pipewire/loop.h>
#include <pipewire/port.h>
#include <pipewire/properties.h>
#include <pipewire/thread-loop.h>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <qthread.h>
#include <qtimer.h>
#include <spa/node/io.h>
#include <spa/param/latency-utils.h>
#include <spa/param/latency.h>
#include <spa/param/param.h>
#include <spa/pod/builder.h>
#include <spa/pod/pod.h>
#include <spa/support/loop.h>
#include <spa/utils/defs.h>
#include <spa/utils/hook.h>
#include <sys/types.h>
#include <KLocalizedString>
#include <QString>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <format>
#include <span>
#include <string>
#include <thread>
#include <utility>
#include "db_manager.hpp"
#include "pipeline_type.hpp"
#include "pw_manager.hpp"
#include "tags_app.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

namespace {

void on_process(void* userdata, spa_io_position* position) {
  auto* d = static_cast<PluginBase::data*>(userdata);

  const auto n_samples = position->clock.duration;
  const auto rate = position->clock.rate.denom;

  if (n_samples == 0 || rate == 0) {
    return;
  }

  // We had to add the following checks for dummy array sizes. See #4085
  if (d->pb->dummy_left.size() != n_samples) {
    d->pb->dummy_left.resize(n_samples);

    std::ranges::fill(d->pb->dummy_left, 0.0F);
  }

  if (d->pb->dummy_right.size() != n_samples) {
    d->pb->dummy_right.resize(n_samples);

    std::ranges::fill(d->pb->dummy_right, 0.0F);
  }

  if (DbMain::copyFilterInputBuffers() && d->pb->copy_left_in.size() != n_samples) {
    d->pb->copy_left_in.resize(n_samples);
    d->pb->copy_right_in.resize(n_samples);
  }

  if (rate != d->pb->rate || n_samples != d->pb->n_samples) {
    d->pb->rate = rate;
    d->pb->n_samples = n_samples;

    d->pb->got_null_left_in = false;
    d->pb->got_null_left_out = false;
    d->pb->got_null_right_in = false;
    d->pb->got_null_right_out = false;
    d->pb->got_null_probe = false;

    d->pb->setup();
  }

  // util::warning("Processing: " + util::to_string(n_samples));

  auto* in_left = static_cast<float*>(pw_filter_get_dsp_buffer(d->in_left, n_samples));
  auto* in_right = static_cast<float*>(pw_filter_get_dsp_buffer(d->in_right, n_samples));

  auto* out_left = static_cast<float*>(pw_filter_get_dsp_buffer(d->out_left, n_samples));
  auto* out_right = static_cast<float*>(pw_filter_get_dsp_buffer(d->out_right, n_samples));

  std::span<float> left_in;
  std::span<float> right_in;
  std::span<float> left_out;
  std::span<float> right_out;

  if (in_left != nullptr) {
    left_in = std::span(in_left, n_samples);

    if (DbMain::copyFilterInputBuffers()) {
      std::ranges::copy(left_in, d->pb->copy_left_in.begin());
    }

  } else {
    if (!d->pb->got_null_left_in) {
      util::warning("Processing: we received a null left_in pointer. Using the dummy array instead.");

      d->pb->got_null_left_in = true;
    }

    left_in = d->pb->dummy_left;
  }

  if (in_right != nullptr) {
    right_in = std::span(in_right, n_samples);

    if (DbMain::copyFilterInputBuffers()) {
      std::ranges::copy(right_in, d->pb->copy_right_in.begin());
    }

  } else {
    if (!d->pb->got_null_right_in) {
      util::warning("Processing: we received a null right_in pointer. Using the dummy array instead.");

      d->pb->got_null_right_in = true;
    }

    right_in = d->pb->dummy_right;
  }

  if (out_left != nullptr) {
    left_out = std::span(out_left, n_samples);
  } else {
    if (!d->pb->got_null_left_out) {
      util::warning("Processing: we received a null left_out pointer. Using the dummy array instead.");

      d->pb->got_null_left_out = true;
    }

    left_out = d->pb->dummy_left;
  }

  if (out_right != nullptr) {
    right_out = std::span(out_right, n_samples);
  } else {
    if (!d->pb->got_null_right_out) {
      util::warning("Processing: we received a null right_out pointer. Using the dummy array instead.");

      d->pb->got_null_right_out = true;
    }

    right_out = d->pb->dummy_right;
  }

  if (!d->pb->enable_probe) {
    if (DbMain::copyFilterInputBuffers()) {
      auto copy_left_in = std::span(d->pb->copy_left_in);
      auto copy_right_in = std::span(d->pb->copy_right_in);

      d->pb->process(copy_left_in, copy_right_in, left_out, right_out);
    } else {
      d->pb->process(left_in, right_in, left_out, right_out);
    }

  } else {
    auto* probe_left = static_cast<float*>(pw_filter_get_dsp_buffer(d->probe_left, n_samples));
    auto* probe_right = static_cast<float*>(pw_filter_get_dsp_buffer(d->probe_right, n_samples));

    if (probe_left == nullptr || probe_right == nullptr) {
      if (!d->pb->got_null_probe) {
        util::debug("Processing: we received a null pointer for probe left/right. Using the dummy array instead.");

        d->pb->got_null_probe = true;
      }

      std::span l(d->pb->dummy_left.data(), n_samples);
      std::span r(d->pb->dummy_right.data(), n_samples);

      if (DbMain::copyFilterInputBuffers()) {
        auto copy_left_in = std::span(d->pb->copy_left_in);
        auto copy_right_in = std::span(d->pb->copy_right_in);

        d->pb->process(copy_left_in, copy_right_in, left_out, right_out, l, r);
      } else {
        d->pb->process(left_in, right_in, left_out, right_out, l, r);
      }

    } else {
      std::span l(probe_left, n_samples);
      std::span r(probe_right, n_samples);

      if (DbMain::copyFilterInputBuffers()) {
        auto copy_left_in = std::span(d->pb->copy_left_in);
        auto copy_right_in = std::span(d->pb->copy_right_in);

        d->pb->process(copy_left_in, copy_right_in, left_out, right_out, l, r);
      } else {
        d->pb->process(left_in, right_in, left_out, right_out, l, r);
      }
    }
  }
}

auto update_filter([[maybe_unused]] struct spa_loop* loop,
                   [[maybe_unused]] bool async,
                   [[maybe_unused]] uint32_t seq,
                   [[maybe_unused]] const void* data,
                   [[maybe_unused]] size_t size,
                   void* user_data) -> int {
  auto* self = static_cast<PluginBase*>(user_data);

  spa_process_latency_info latency_info{};

  latency_info.ns = static_cast<uint64_t>(self->latency_value * 1000000000.0F);

  std::array<char, 1024U> buffer{};

  spa_pod_builder b{};

  spa_pod_builder_init(&b, buffer.data(), sizeof(buffer));

  const spa_pod* param = spa_process_latency_build(&b, SPA_PARAM_ProcessLatency, &latency_info);

  pw_filter_update_params(self->filter, nullptr, &param, 1);

  return 0;
}

void on_filter_state_changed(void* userdata,
                             [[maybe_unused]] pw_filter_state old,
                             pw_filter_state state,
                             [[maybe_unused]] const char* error) {
  auto* d = static_cast<PluginBase::data*>(userdata);

  d->pb->state = state;

  switch (state) {
    case PW_FILTER_STATE_ERROR:
      d->pb->can_get_node_id = false;
      break;
    case PW_FILTER_STATE_UNCONNECTED:
      d->pb->can_get_node_id = false;
      break;
    case PW_FILTER_STATE_CONNECTING:
      d->pb->can_get_node_id = false;
      break;
    case PW_FILTER_STATE_STREAMING:
      d->pb->can_get_node_id = true;
      break;
    case PW_FILTER_STATE_PAUSED:
      d->pb->can_get_node_id = true;
      break;
    default:
      break;
  }
}

const struct pw_filter_events filter_events = {.version = 0,
                                               .destroy = nullptr,
                                               .state_changed = on_filter_state_changed,
                                               .io_changed = nullptr,
                                               .param_changed = nullptr,
                                               .add_buffer = nullptr,
                                               .remove_buffer = nullptr,
                                               .process = on_process,
                                               .drained = nullptr,
                                               .command = nullptr};

}  // namespace

PluginBase::PluginBase(std::string tag,
                       QString plugin_name,
                       QString package,
                       QString instance_id,
                       pw::Manager* pipe_manager,
                       PipelineType pipe_type,
                       const bool& enable_probe)
    : log_tag(std::move(tag)),
      name(std::move(plugin_name)),
      package(std::move(package)),
      instance_id(std::move(instance_id)),
      pipeline_type(pipe_type),
      enable_probe(enable_probe),
      pm(pipe_manager),
      baseWorker(new PluginBaseWorker),
      native_ui_timer(new QTimer(this)) {
  QString description;
  QString description_pipeline;

  switch (pipeline_type) {
    case PipelineType::input:
      description_pipeline = i18n("(Mic)");
      break;
    case PipelineType::output:
      description_pipeline = i18n("(Speakers)");
      break;
  }

  if (name != "output_level" && name != "spectrum") {
    description = tags::plugin_name::Model::self().translate(name) + " " + description_pipeline;
  } else if (name == "output_level") {
    description = i18n("Output Level Meter");
  } else if (name == "spectrum") {
    description = i18n("Spectrum");
  }

  pf_data.pb = this;

  const auto filter_name = "ee_" + log_tag.substr(0U, log_tag.size() - 2U) + "_" + name.toStdString();

  pm->lock();

  auto* props_filter = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_filter, PW_KEY_APP_ID, tags::app::id);
  pw_properties_set(props_filter, PW_KEY_NODE_NAME, filter_name.c_str());
  pw_properties_set(props_filter, PW_KEY_NODE_NICK, name.toStdString().c_str());
  pw_properties_set(props_filter, PW_KEY_NODE_DESCRIPTION, description.toStdString().c_str());
  pw_properties_set(props_filter, PW_KEY_MEDIA_TYPE, "Audio");
  pw_properties_set(props_filter, PW_KEY_MEDIA_CATEGORY, "Filter");
  pw_properties_set(props_filter, PW_KEY_MEDIA_ROLE, "DSP");
  pw_properties_set(props_filter, PW_KEY_NODE_PASSIVE, "true");

  filter = pw_filter_new(pm->core, filter_name.c_str(), props_filter);

  // left channel input

  auto* props_in_left = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_in_left, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_in_left, PW_KEY_PORT_NAME, "input_FL");
  pw_properties_set(props_in_left, "audio.channel", "FL");

  pf_data.in_left = static_cast<port*>(pw_filter_add_port(filter, PW_DIRECTION_INPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                                                          sizeof(port), props_in_left, nullptr, 0));

  // right channel input

  auto* props_in_right = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_in_right, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_in_right, PW_KEY_PORT_NAME, "input_FR");
  pw_properties_set(props_in_right, "audio.channel", "FR");

  pf_data.in_right = static_cast<port*>(pw_filter_add_port(filter, PW_DIRECTION_INPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                                                           sizeof(port), props_in_right, nullptr, 0));

  // left channel output

  auto* props_out_left = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_out_left, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_out_left, PW_KEY_PORT_NAME, "output_FL");
  pw_properties_set(props_out_left, "audio.channel", "FL");

  pf_data.out_left = static_cast<port*>(pw_filter_add_port(filter, PW_DIRECTION_OUTPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                                                           sizeof(port), props_out_left, nullptr, 0));

  // right channel output

  auto* props_out_right = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_out_right, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_out_right, PW_KEY_PORT_NAME, "output_FR");
  pw_properties_set(props_out_right, "audio.channel", "FR");

  pf_data.out_right = static_cast<port*>(pw_filter_add_port(
      filter, PW_DIRECTION_OUTPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS, sizeof(port), props_out_right, nullptr, 0));

  if (enable_probe) {
    n_ports += 2;

    // probe left input

    auto* props_left = pw_properties_new(nullptr, nullptr);

    pw_properties_set(props_left, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
    pw_properties_set(props_left, PW_KEY_PORT_NAME, "probe_FL");
    pw_properties_set(props_left, "audio.channel", "PROBE_FL");

    pf_data.probe_left = static_cast<port*>(pw_filter_add_port(
        filter, PW_DIRECTION_INPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS, sizeof(port), props_left, nullptr, 0));

    // probe right input

    auto* props_right = pw_properties_new(nullptr, nullptr);

    pw_properties_set(props_right, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
    pw_properties_set(props_right, PW_KEY_PORT_NAME, "probe_FR");
    pw_properties_set(props_right, "audio.channel", "PROBE_FR");

    pf_data.probe_right = static_cast<port*>(pw_filter_add_port(
        filter, PW_DIRECTION_INPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS, sizeof(port), props_right, nullptr, 0));
  }

  pm->sync_wait_unlock();

  native_ui_timer->setInterval(static_cast<long>(1000.0 / DbMain::lv2uiUpdateFrequency()));

  connect(native_ui_timer, &QTimer::timeout, this, [&]() {
    if (lv2_wrapper == nullptr || !lv2_wrapper->has_ui()) {
      return;
    }

    lv2_wrapper->notify_ui();
    lv2_wrapper->update_ui();
  });

  // worker thread for the native ui and maybe also other things

  baseWorker->moveToThread(&workerThread);

  workerThread.start();

  connect(&workerThread, &QThread::finished, baseWorker, &QObject::deleteLater);
}

PluginBase::~PluginBase() {
  pm->lock();

  if (listener.link.next != nullptr || listener.link.prev != nullptr) {
    spa_hook_remove(&listener);
  }

  pw_filter_destroy(filter);

  pm->sync_wait_unlock();

  workerThread.quit();
  workerThread.wait();
}

void PluginBase::reset() {}

auto PluginBase::connect_to_pw() -> bool {
  connected_to_pw = false;
  can_get_node_id = false;
  state = PW_FILTER_STATE_UNCONNECTED;

  pm->lock();

  if (pw_filter_connect(filter, PW_FILTER_FLAG_RT_PROCESS, nullptr, 0) != 0) {
    pm->unlock();

    util::warning(std::format("{}{} cannot connect the filter to PipeWire!", log_tag, name.toStdString()));

    return false;
  }

  pw_filter_add_listener(filter, &listener, &filter_events, &pf_data);

  pm->sync_wait_unlock();

  while (!can_get_node_id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    if (state == PW_FILTER_STATE_ERROR) {
      util::warning(std::format("{}{} is in an error", log_tag, name.toStdString()));

      return false;
    }
  }

  pm->lock();

  node_id = pw_filter_get_node_id(filter);

  pm->sync_wait_unlock();

  /**
   * The filter we link in our pipeline have at least 4 ports. Some have six.
   * Before we try to link filters we have to wait until the information about
   * their ports is available in PipeManager's list_ports vector.
   */

  while (pm->count_node_ports(node_id) != n_ports) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  connected_to_pw = true;

  util::debug(std::format("{}{} successfully connected to PipeWire graph", log_tag, name.toStdString()));

  return true;
}

auto PluginBase::get_node_id() const -> uint {
  return node_id;
}

void PluginBase::set_active(const bool& state) const {
  pw_filter_set_active(filter, state);
}

void PluginBase::disconnect_from_pw() {
  pm->lock();

  set_active(false);

  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  if (listener.link.next != nullptr || listener.link.prev != nullptr) {
    spa_hook_remove(&listener);
  }

  pw_filter_disconnect(filter);

  connected_to_pw = false;

  pm->sync_wait_unlock();

  node_id = SPA_ID_INVALID;

  while (pw_filter_get_state(filter, nullptr) != PW_FILTER_STATE_UNCONNECTED) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  util::debug(std::format("{}{} is disconnected", log_tag, name.toStdString()));
}

void PluginBase::clear_data() {}

void PluginBase::setup() {}

void PluginBase::process([[maybe_unused]] std::span<float>& left_in,
                         [[maybe_unused]] std::span<float>& right_in,
                         [[maybe_unused]] std::span<float>& left_out,
                         [[maybe_unused]] std::span<float>& right_out) {}

void PluginBase::process([[maybe_unused]] std::span<float>& left_in,
                         [[maybe_unused]] std::span<float>& right_in,
                         [[maybe_unused]] std::span<float>& left_out,
                         [[maybe_unused]] std::span<float>& right_out,
                         [[maybe_unused]] std::span<float>& probe_left,
                         [[maybe_unused]] std::span<float>& probe_right) {}

auto PluginBase::get_latency_seconds() -> float {
  return 0.0F;
}

void PluginBase::showNativeUi() {
  native_ui_timer->start();

  // using invokeMethod to force this code to run in the base class worker thread and
  // avoid load in QML or main thread.

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
  QMetaObject::invokeMethod(
      baseWorker,
      [this] {
        if (lv2_wrapper != nullptr && !lv2_wrapper->has_ui()) {
          lv2_wrapper->load_ui();
        }
      },
      Qt::QueuedConnection);

  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void PluginBase::closeNativeUi() {
  native_ui_timer->stop();

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
  QMetaObject::invokeMethod(
      baseWorker,
      [this] {
        if (lv2_wrapper != nullptr) {
          lv2_wrapper->native_ui_to_database();
          lv2_wrapper->close_ui();
        }
      },
      Qt::QueuedConnection);

  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void PluginBase::set_native_ui_update_frequency(const uint& value) {
  if (lv2_wrapper == nullptr) {
    return;
  }

  native_ui_timer->setInterval(static_cast<long>(1000.0 / value));
}

void PluginBase::get_peaks(const std::span<float>& left_in,
                           const std::span<float>& right_in,
                           std::span<float>& left_out,
                           std::span<float>& right_out) {
  float in_left_max = 0.0F;
  float in_right_max = 0.0F;
  float out_left_max = 0.0F;
  float out_right_max = 0.0F;

  for (size_t i = 0; i < n_samples; ++i) {
    in_left_max = std::max(in_left_max, std::fabs(left_in[i]));
    in_right_max = std::max(in_right_max, std::fabs(right_in[i]));
    out_left_max = std::max(out_left_max, std::fabs(left_out[i]));
    out_right_max = std::max(out_right_max, std::fabs(right_out[i]));
  }

  input_peak_left = util::linear_to_db(in_left_max);
  input_peak_right = util::linear_to_db(in_right_max);
  output_peak_left = util::linear_to_db(out_left_max);
  output_peak_right = util::linear_to_db(out_right_max);
}

void PluginBase::apply_gain(std::span<float>& left, std::span<float>& right, const float& gain) const {
  if (left.empty() || right.empty()) {
    return;
  }

  float* __restrict l_ptr = left.data();
  float* __restrict r_ptr = right.data();

  for (uint i = 0; i < n_samples; i++) {
    l_ptr[i] *= gain;
    r_ptr[i] *= gain;
  }
}

void PluginBase::update_probe_links() {}

void PluginBase::update_filter_params() {
  pw_loop_invoke(pw_thread_loop_get_loop(pm->thread_loop), update_filter, 1, nullptr, 0, false, this);  // NOLINT
}

float PluginBase::getInputLevelLeft() const {
  return input_peak_left;
}

float PluginBase::getInputLevelRight() const {
  return input_peak_right;
}

float PluginBase::getOutputLevelLeft() const {
  return output_peak_left;
}

float PluginBase::getOutputLevelRight() const {
  return output_peak_right;
}

bool PluginBase::hasNativeUi() {
  return lv2_wrapper->has_ui();
}
