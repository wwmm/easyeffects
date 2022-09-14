/*
 *  Copyright © 2017-2023 Wellington Wallace
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

#include "pitch.hpp"

Pitch::Pitch(const std::string& tag,
             const std::string& schema,
             const std::string& schema_path,
             PipeManager* pipe_manager)
    : PluginBase(tag, tags::plugin_name::pitch, tags::plugin_package::rubber, schema, schema_path, pipe_manager) {
  mode = parse_mode_key(util::gsettings_get_string(settings, "mode"));
  formant = parse_formant_key(util::gsettings_get_string(settings, "formant"));
  transients = parse_transients_key(util::gsettings_get_string(settings, "transients"));
  detector = parse_detector_key(util::gsettings_get_string(settings, "detector"));
  phase = parse_phase_key(util::gsettings_get_string(settings, "phase"));

  octaves = g_settings_get_int(settings, "octaves");
  semitones = g_settings_get_int(settings, "semitones");
  cents = g_settings_get_int(settings, "cents");

  gconnections.push_back(g_signal_connect(settings, "changed::mode",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<Pitch*>(user_data);

                                            self->mode = parse_mode_key(util::gsettings_get_string(settings, key));

                                            if (!self->rubberband_ready) {
                                              return;
                                            }

                                            self->set_mode();
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(
      settings, "changed::formant", G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
        auto self = static_cast<Pitch*>(user_data);

        self->formant = parse_formant_key(util::gsettings_get_string(settings, key));

        if (!self->rubberband_ready) {
          return;
        }

        self->set_formant();
      }),
      this));

  gconnections.push_back(g_signal_connect(
      settings, "changed::transients", G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
        auto self = static_cast<Pitch*>(user_data);

        self->transients = parse_transients_key(util::gsettings_get_string(settings, key));

        if (!self->rubberband_ready) {
          return;
        }

        self->set_transients();
      }),
      this));

  gconnections.push_back(g_signal_connect(
      settings, "changed::detector", G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
        auto self = static_cast<Pitch*>(user_data);

        self->detector = parse_detector_key(util::gsettings_get_string(settings, key));

        if (!self->rubberband_ready) {
          return;
        }

        self->set_detector();
      }),
      this));

  gconnections.push_back(g_signal_connect(settings, "changed::phase",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<Pitch*>(user_data);

                                            self->phase = parse_phase_key(util::gsettings_get_string(settings, key));

                                            if (!self->rubberband_ready) {
                                              return;
                                            }

                                            self->set_phase();
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::octaves",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<Pitch*>(user_data);

                                            self->octaves = g_settings_get_int(settings, key);

                                            self->set_pitch_scale();
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::semitones",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<Pitch*>(user_data);

                                            self->semitones = g_settings_get_int(settings, key);

                                            self->set_pitch_scale();
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::cents",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<Pitch*>(user_data);

                                            self->cents = g_settings_get_int(settings, key);

                                            self->set_pitch_scale();
                                          }),
                                          this));

  setup_input_output_gain();
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

  deque_out_L.resize(0U);
  deque_out_R.resize(0U);

  /*
   RubberBand initialization is slow. It is better to do it outside of the plugin realtime thread
 */

  util::idle_add([&, this] {
    if (rubberband_ready) {
      return;
    }

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

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  stretcher_in[0] = left_in.data();
  stretcher_in[1] = right_in.data();

  stretcher->process(stretcher_in.data(), n_samples, false);

  if (const auto n_available = stretcher->available(); n_available > 0) {
    // util::debug(log_tag + name + " available: " + util::to_string(n_available));

    data_L.resize(n_available);
    data_R.resize(n_available);

    stretcher_out[0] = data_L.data();
    stretcher_out[1] = data_R.data();

    stretcher->retrieve(stretcher_out.data(), n_available);

    for (int n = 0; n < n_available; n++) {
      deque_out_L.push_back(data_L[n]);
      deque_out_R.push_back(data_R[n]);
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

    for (uint n = 0U; !deque_out_L.empty() && n < left_out.size(); n++) {
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

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (notify_latency) {
    latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(log_tag + name + " latency: " + util::to_string(latency_value, "") + " s");

    util::idle_add([=, this]() {
      if (!post_messages || latency.empty()) {
        return;
      }

      latency.emit();
    });

    spa_process_latency_info latency_info{};

    latency_info.ns = static_cast<uint64_t>(latency_value * 1000000000.0F);

    std::array<char, 1024U> buffer{};

    spa_pod_builder b{};

    spa_pod_builder_init(&b, buffer.data(), sizeof(buffer));

    const spa_pod* param = spa_process_latency_build(&b, SPA_PARAM_ProcessLatency, &latency_info);

    pw_filter_update_params(filter, nullptr, &param, 1);

    notify_latency = false;
  }

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += buffer_duration;

    if (notification_dt >= notification_time_window) {
      notify();

      notification_dt = 0.0F;
    }
  }
}

/*
  Code based on the RubberBand LADSPA plugin

  https://github.com/breakfastquay/rubberband/blob/cc937ebe655fc3c902ad0bc5cb63ce4e782720ee/ladspa/RubberBandPitchShifter.cpp#L377
*/

auto Pitch::parse_mode_key(const std::string& key) -> Mode {
  if (key == "HighSpeed") {
    return Mode::speed;
  }

  if (key == "HighQuality") {
    return Mode::quality;
  }

  if (key == "HighConsistency") {
    return Mode::consistency;
  }

  return Mode::speed;
}

auto Pitch::parse_formant_key(const std::string& key) -> Formant {
  if (key == "Shifted") {
    return Formant::shifted;
  }

  if (key == "Preserved") {
    return Formant::preserved;
  }

  return Formant::shifted;
}

auto Pitch::parse_transients_key(const std::string& key) -> Transients {
  if (key == "Crisp") {
    return Transients::crisp;
  }

  if (key == "Mixed") {
    return Transients::mixed;
  }

  if (key == "Smooth") {
    return Transients::smooth;
  }

  return Transients::crisp;
}

auto Pitch::parse_detector_key(const std::string& key) -> Detector {
  if (key == "Compound") {
    return Detector::compound;
  }

  if (key == "Percussive") {
    return Detector::percussive;
  }

  if (key == "Soft") {
    return Detector::soft;
  }

  return Detector::compound;
}

auto Pitch::parse_phase_key(const std::string& key) -> Phase {
  if (key == "Laminar") {
    return Phase::laminar;
  }

  if (key == "Independent") {
    return Phase::independent;
  }

  return Phase::laminar;
}

void Pitch::set_mode() {
  if (stretcher == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  switch (mode) {
    case Mode::speed:
      stretcher->setPitchOption(RubberBand::RubberBandStretcher::OptionPitchHighSpeed);

      break;
    case Mode::quality:
      stretcher->setPitchOption(RubberBand::RubberBandStretcher::OptionPitchHighQuality);

      break;
    case Mode::consistency:
      stretcher->setPitchOption(RubberBand::RubberBandStretcher::OptionPitchHighConsistency);

      break;
  }
}

void Pitch::set_formant() {
  if (stretcher == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  switch (formant) {
    case Formant::shifted:
      stretcher->setFormantOption(RubberBand::RubberBandStretcher::OptionFormantShifted);

      break;
    case Formant::preserved:
      stretcher->setFormantOption(RubberBand::RubberBandStretcher::OptionFormantPreserved);

      break;
  }
}

void Pitch::set_transients() {
  if (stretcher == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  switch (transients) {
    case Transients::crisp:
      stretcher->setTransientsOption(RubberBand::RubberBandStretcher::OptionTransientsCrisp);

      break;
    case Transients::mixed:
      stretcher->setTransientsOption(RubberBand::RubberBandStretcher::OptionTransientsMixed);

      break;
    case Transients::smooth:
      stretcher->setTransientsOption(RubberBand::RubberBandStretcher::OptionTransientsSmooth);

      break;
  }
}

void Pitch::set_detector() {
  if (stretcher == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  switch (detector) {
    case Detector::compound:
      stretcher->setDetectorOption(RubberBand::RubberBandStretcher::OptionDetectorCompound);

      break;
    case Detector::percussive:
      stretcher->setDetectorOption(RubberBand::RubberBandStretcher::OptionDetectorPercussive);

      break;
    case Detector::soft:
      stretcher->setTransientsOption(RubberBand::RubberBandStretcher::OptionDetectorSoft);

      break;
  }
}

void Pitch::set_phase() {
  if (stretcher == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  switch (phase) {
    case Phase::laminar:
      stretcher->setPhaseOption(RubberBand::RubberBandStretcher::OptionPhaseLaminar);

      break;
    case Phase::independent:
      stretcher->setPhaseOption(RubberBand::RubberBandStretcher::OptionPhaseIndependent);

      break;
  }
}

void Pitch::set_pitch_scale() {
  if (stretcher == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  const double n_octaves = octaves + (static_cast<double>(semitones) / 12.0) + (static_cast<double>(cents) / 1200.0);

  const double ratio = std::pow(2.0, n_octaves);

  stretcher->setPitchScale(ratio);
}

void Pitch::init_stretcher() {
  delete stretcher;

  RubberBand::RubberBandStretcher::Options options =
      RubberBand::RubberBandStretcher::OptionProcessRealTime | RubberBand::RubberBandStretcher::OptionChannelsTogether;

  stretcher = new RubberBand::RubberBandStretcher(rate, 2, options);

  stretcher->setMaxProcessSize(n_samples);
  stretcher->setTimeRatio(time_ratio);

  set_pitch_scale();
  set_mode();
  set_formant();
  set_transients();
  set_detector();
  set_phase();
}

auto Pitch::get_latency_seconds() -> float {
  return latency_value;
}
