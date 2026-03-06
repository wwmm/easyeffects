#include "immersive_surround.hpp"
#include <algorithm>
#include <cmath>
#include <format>
#include <mutex>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_immersive_surround.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

ImmersiveSurround::ImmersiveSurround(const std::string& tag,
                                     pw::Manager* pipe_manager,
                                     PipelineType pipe_type,
                                     QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::immersiveSurround,
                 tags::plugin_package::Package::ee,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(db::Manager::self().get_plugin_db<db::ImmersiveSurround>(
          pipe_type,
          tags::plugin_name::BaseName::immersiveSurround + "#" + instance_id)) {
  init_common_controls<db::ImmersiveSurround>(settings);

  surround_amount = static_cast<float>(settings->surroundAmount() * 0.01);
  stereo_width = static_cast<float>(settings->stereoWidth());
  rear_mix = static_cast<float>(settings->rearMix());
  center_gain = static_cast<float>(util::db_to_linear(settings->centerLevel()));
  damping = static_cast<float>(settings->damping());
  rear_phase_invert = settings->rearPhaseInvert();

  connect(settings, &db::ImmersiveSurround::surroundAmountChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    surround_amount = std::clamp(static_cast<float>(settings->surroundAmount() * 0.01), 0.0F, 1.0F);
  });

  connect(settings, &db::ImmersiveSurround::stereoWidthChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    stereo_width = std::clamp(static_cast<float>(settings->stereoWidth()), 0.0F, 3.0F);
  });

  connect(settings, &db::ImmersiveSurround::rearMixChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    rear_mix = std::clamp(static_cast<float>(settings->rearMix()), 0.0F, 1.0F);
  });

  connect(settings, &db::ImmersiveSurround::centerLevelChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    center_gain = static_cast<float>(util::db_to_linear(settings->centerLevel()));
  });

  connect(settings, &db::ImmersiveSurround::dampingChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    damping = std::clamp(static_cast<float>(settings->damping()), 0.0F, 0.98F);
  });

  connect(settings, &db::ImmersiveSurround::rearPhaseInvertChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    rear_phase_invert = settings->rearPhaseInvert();
  });

  connect(settings, &db::ImmersiveSurround::rearDelayChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    update_delay_samples();
  });
}

ImmersiveSurround::~ImmersiveSurround() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void ImmersiveSurround::reset() {
  settings->setDefaults();
}

void ImmersiveSurround::clear_data() {
  setup();
}

void ImmersiveSurround::update_delay_samples() {
  if (rate == 0 || max_delay_samples <= 1U) {
    current_delay_samples = 1U;

    return;
  }

  const auto requested =
      static_cast<uint>(std::round(std::max(1.0, settings->rearDelay()) * 0.001 * static_cast<double>(rate)));

  current_delay_samples = std::clamp(requested, 1U, max_delay_samples - 1U);
}

void ImmersiveSurround::setup() {
  if (rate == 0 || n_samples == 0) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  constexpr auto max_delay_seconds = 0.04F;

  max_delay_samples = std::max(2U, static_cast<uint>(std::ceil(static_cast<float>(rate) * max_delay_seconds)) + 1U);

  delay_l.assign(max_delay_samples, 0.0F);
  delay_r.assign(max_delay_samples, 0.0F);

  delay_index = 0U;

  rear_lp_l = 0.0F;
  rear_lp_r = 0.0F;

  update_delay_samples();

  ready = true;
}

void ImmersiveSurround::process(std::span<float>& left_in,
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

  for (size_t n = 0U; n < left_in.size(); n++) {
    const auto in_l = left_in[n];
    const auto in_r = right_in[n];

    const auto mid = 0.5F * (in_l + in_r);
    const auto side = 0.5F * (in_l - in_r);

    const auto read_idx = (delay_index + max_delay_samples - current_delay_samples) % max_delay_samples;

    const auto delayed_l = delay_l[read_idx];
    const auto delayed_r = delay_r[read_idx];

    delay_l[delay_index] = side;
    delay_r[delay_index] = -side;

    delay_index = (delay_index + 1U) % max_delay_samples;

    rear_lp_l = (damping * rear_lp_l) + ((1.0F - damping) * delayed_l);
    rear_lp_r = (damping * rear_lp_r) + ((1.0F - damping) * delayed_r);

    auto rear_l = rear_mix * rear_lp_r;
    auto rear_r = rear_mix * rear_lp_l;

    if (rear_phase_invert) {
      rear_l = -rear_l;
      rear_r = -rear_r;
    }

    const auto front_l = (center_gain * mid) + (stereo_width * side);
    const auto front_r = (center_gain * mid) - (stereo_width * side);

    left_out[n] = front_l + (surround_amount * rear_l);
    right_out[n] = front_r + (surround_amount * rear_r);
  }

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void ImmersiveSurround::process([[maybe_unused]] std::span<float>& left_in,
                                [[maybe_unused]] std::span<float>& right_in,
                                [[maybe_unused]] std::span<float>& left_out,
                                [[maybe_unused]] std::span<float>& right_out,
                                [[maybe_unused]] std::span<float>& probe_left,
                                [[maybe_unused]] std::span<float>& probe_right) {}

auto ImmersiveSurround::get_latency_seconds() -> float {
  return 0.0F;
}
