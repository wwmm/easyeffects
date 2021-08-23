/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "pitch.hpp"

Pitch::Pitch(const std::string& tag,
             const std::string& schema,
             const std::string& schema_path,
             PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::pitch, schema, schema_path, pipe_manager) {
  input_gain = static_cast<float>(util::db_to_linear(settings->get_double("input-gain")));
  output_gain = static_cast<float>(util::db_to_linear(settings->get_double("output-gain")));

  formant_preserving = settings->get_boolean("formant-preserving");
  faster = settings->get_boolean("faster");

  crispness = settings->get_int("crispness");
  octaves = settings->get_int("octaves");
  semitones = settings->get_int("semitones");
  cents = settings->get_int("cents");

  settings->signal_changed("input-gain").connect([=, this](const auto& key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](const auto& key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("crispness").connect([=, this](const auto& key) {
    crispness = settings->get_int("crispness");

    std::scoped_lock<std::mutex> lock(data_mutex);

    update_crispness();
  });

  settings->signal_changed("formant-preserving").connect([=, this](const auto& key) {
    formant_preserving = settings->get_boolean(key);

    std::scoped_lock<std::mutex> lock(data_mutex);

    if (!rubberband_ready) {
      return;
    }

    stretcher->setFormantOption(formant_preserving ? RubberBand::RubberBandStretcher::OptionFormantPreserved
                                                   : RubberBand::RubberBandStretcher::OptionFormantShifted);
  });

  settings->signal_changed("faster").connect([=, this](const auto& key) {
    faster = settings->get_boolean(key);

    std::scoped_lock<std::mutex> lock(data_mutex);

    if (!rubberband_ready) {
      return;
    }

    stretcher->setPitchOption(faster ? RubberBand::RubberBandStretcher::OptionPitchHighSpeed
                                     : RubberBand::RubberBandStretcher::OptionPitchHighConsistency);
  });

  settings->signal_changed("octaves").connect([=, this](const auto& key) {
    octaves = settings->get_int(key);

    std::scoped_lock<std::mutex> lock(data_mutex);

    update_pitch_scale();
  });

  settings->signal_changed("semitones").connect([=, this](const auto& key) {
    semitones = settings->get_int(key);

    std::scoped_lock<std::mutex> lock(data_mutex);

    update_pitch_scale();
  });

  settings->signal_changed("cents").connect([=, this](const auto& key) {
    cents = settings->get_int(key);

    std::scoped_lock<std::mutex> lock(data_mutex);

    update_pitch_scale();
  });
}

Pitch::~Pitch() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  util::debug(log_tag + name + " destroyed");
}

void Pitch::setup() {
  rubberband_ready = false;

  latency_n_frames = 0U;

  deque_out_L.resize(0);
  deque_out_R.resize(0);

  /*
   RubberBand initialization is slow. It is better to do it outside of the plugin realtime thread
 */

  Glib::signal_idle().connect_once([&, this] {
    init_stretcher();

    std::scoped_lock<std::mutex> lock(data_mutex);

    rubberband_ready = true;
  });
}

void Pitch::process(std::span<float>& left_in,
                    std::span<float>& right_in,
                    std::span<float>& left_out,
                    std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass || !rubberband_ready) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  apply_gain(left_in, right_in, input_gain);

  stretcher_in[0] = left_in.data();
  stretcher_in[1] = right_in.data();

  stretcher->process(stretcher_in.data(), n_samples, false);

  if (const auto& n_available = stretcher->available(); n_available > 0) {
    // util::debug(log_tag + name + " available: " + std::to_string(n_available));

    data_L.resize(n_available);
    data_R.resize(n_available);

    stretcher_out[0] = data_L.data();
    stretcher_out[1] = data_R.data();

    stretcher->retrieve(stretcher_out.data(), n_available);

    for (int n = 0; n < n_available; n++) {
      deque_out_L.emplace_back(data_L[n]);
      deque_out_R.emplace_back(data_R[n]);
    }
  }

  if (deque_out_L.size() >= left_out.size()) {
    for (float& v : left_out) {
      v = deque_out_L.front();

      deque_out_L.pop_front();
    }

    for (float& v : right_out) {
      v = deque_out_R.front();

      deque_out_R.pop_front();
    }
  } else {
    const uint offset = left_out.size() - deque_out_L.size();

    if (offset != latency_n_frames) {
      latency_n_frames = offset;

      notify_latency = true;
    }

    for (uint n = 0U, m = left_out.size(); !deque_out_L.empty() && n < m; n++) {
      if (n < offset) {
        left_out[n] = 0.0F;
        right_out[n] = 0.0F;
      } else {
        left_out[n] = deque_out_L.front();
        right_out[n] = deque_out_R.front();

        deque_out_R.pop_front();
        deque_out_L.pop_front();
      }
    }
  }

  apply_gain(left_out, right_out, output_gain);

  if (notify_latency) {
    const float latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(log_tag + name + " latency: " + std::to_string(latency_value) + " s");

    Glib::signal_idle().connect_once([=, this] { latency.emit(latency_value); });

    spa_process_latency_info latency_info{};

    latency_info.ns = static_cast<uint64_t>(latency_value * 1000000000.0F);

    std::array<char, 1024> buffer{};

    spa_pod_builder b{};

    spa_pod_builder_init(&b, buffer.data(), sizeof(buffer));

    const spa_pod* param = spa_process_latency_build(&b, SPA_PARAM_ProcessLatency, &latency_info);

    pw_filter_update_params(filter, nullptr, &param, 1);

    notify_latency = false;
  }

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += sample_duration;

    if (notification_dt >= notification_time_window) {
      notify();

      notification_dt = 0.0F;
    }
  }
}

void Pitch::update_crispness() {
  if (stretcher == nullptr) {
    return;
  }

  switch (crispness) {
    case 0:
      stretcher->setTransientsOption(RubberBand::RubberBandStretcher::OptionTransientsSmooth);
      stretcher->setPhaseOption(RubberBand::RubberBandStretcher::OptionPhaseIndependent);
      stretcher->setDetectorOption(RubberBand::RubberBandStretcher::OptionDetectorCompound);

      break;
    case 1:
      stretcher->setTransientsOption(RubberBand::RubberBandStretcher::OptionTransientsCrisp);
      stretcher->setPhaseOption(RubberBand::RubberBandStretcher::OptionPhaseIndependent);
      stretcher->setDetectorOption(RubberBand::RubberBandStretcher::OptionDetectorSoft);

      break;
    case 2:
      stretcher->setTransientsOption(RubberBand::RubberBandStretcher::OptionTransientsSmooth);
      stretcher->setPhaseOption(RubberBand::RubberBandStretcher::OptionPhaseIndependent);
      stretcher->setDetectorOption(RubberBand::RubberBandStretcher::OptionDetectorCompound);

      break;
    case 3:
      stretcher->setTransientsOption(RubberBand::RubberBandStretcher::OptionTransientsSmooth);
      stretcher->setPhaseOption(RubberBand::RubberBandStretcher::OptionPhaseLaminar);
      stretcher->setDetectorOption(RubberBand::RubberBandStretcher::OptionDetectorCompound);

      break;
    case 4:
      stretcher->setTransientsOption(RubberBand::RubberBandStretcher::OptionTransientsMixed);
      stretcher->setPhaseOption(RubberBand::RubberBandStretcher::OptionPhaseLaminar);
      stretcher->setDetectorOption(RubberBand::RubberBandStretcher::OptionDetectorCompound);

      break;
    case 5:
      stretcher->setTransientsOption(RubberBand::RubberBandStretcher::OptionTransientsCrisp);
      stretcher->setPhaseOption(RubberBand::RubberBandStretcher::OptionPhaseLaminar);
      stretcher->setDetectorOption(RubberBand::RubberBandStretcher::OptionDetectorCompound);

      break;
    case 6:
      stretcher->setTransientsOption(RubberBand::RubberBandStretcher::OptionTransientsCrisp);
      stretcher->setPhaseOption(RubberBand::RubberBandStretcher::OptionPhaseIndependent);
      stretcher->setDetectorOption(RubberBand::RubberBandStretcher::OptionDetectorCompound);

      break;
  }
}

/*
  Code based on the RubberBand LADSPA plugin

  https://github.com/breakfastquay/rubberband/blob/cc937ebe655fc3c902ad0bc5cb63ce4e782720ee/ladspa/RubberBandPitchShifter.cpp#L377
*/

void Pitch::update_pitch_scale() {
  if (stretcher == nullptr) {
    return;
  }

  const double n_octaves = octaves + static_cast<double>(semitones) / 12.0 + static_cast<double>(cents) / 1200.0;

  const double ratio = std::pow(2.0, n_octaves);

  stretcher->setPitchScale(ratio);
}

void Pitch::init_stretcher() {
  delete stretcher;

  RubberBand::RubberBandStretcher::Options options = RubberBand::RubberBandStretcher::OptionProcessRealTime |
                                                     RubberBand::RubberBandStretcher::OptionPitchHighConsistency |
                                                     RubberBand::RubberBandStretcher::OptionChannelsTogether |
                                                     RubberBand::RubberBandStretcher::OptionPhaseIndependent;

  stretcher = new RubberBand::RubberBandStretcher(rate, 2, options);

  stretcher->setMaxProcessSize(n_samples);

  stretcher->setFormantOption(formant_preserving ? RubberBand::RubberBandStretcher::OptionFormantPreserved
                                                 : RubberBand::RubberBandStretcher::OptionFormantShifted);

  stretcher->setPitchOption(faster ? RubberBand::RubberBandStretcher::OptionPitchHighSpeed
                                   : RubberBand::RubberBandStretcher::OptionPitchHighConsistency);

  stretcher->setTimeRatio(time_ratio);

  update_crispness();
  update_pitch_scale();
}
