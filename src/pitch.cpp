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
  formant_preserving = g_settings_get_boolean(settings, "formant-preserving") != 0;
  faster = g_settings_get_boolean(settings, "faster") != 0;

  crispness = g_settings_get_int(settings, "crispness");
  octaves = g_settings_get_int(settings, "octaves");
  semitones = g_settings_get_int(settings, "semitones");
  cents = g_settings_get_int(settings, "cents");

  gconnections.push_back(g_signal_connect(
      settings, "changed::formant-preserving", G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
        auto self = static_cast<Pitch*>(user_data);

        self->formant_preserving = g_settings_get_boolean(settings, key) != 0;

        std::scoped_lock<std::mutex> lock(self->data_mutex);

        if (!self->rubberband_ready) {
          return;
        }

        self->stretcher->setFormantOption(self->formant_preserving
                                              ? RubberBand::RubberBandStretcher::OptionFormantPreserved
                                              : RubberBand::RubberBandStretcher::OptionFormantShifted);
      }),
      this));

  gconnections.push_back(g_signal_connect(
      settings, "changed::faster", G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
        auto self = static_cast<Pitch*>(user_data);

        self->faster = g_settings_get_boolean(settings, key) != 0;

        std::scoped_lock<std::mutex> lock(self->data_mutex);

        if (!self->rubberband_ready) {
          return;
        }

        self->stretcher->setPitchOption(self->faster ? RubberBand::RubberBandStretcher::OptionPitchHighSpeed
                                                     : RubberBand::RubberBandStretcher::OptionPitchHighConsistency);
      }),
      this));

  gconnections.push_back(g_signal_connect(settings, "changed::crispness",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<Pitch*>(user_data);

                                            self->crispness = g_settings_get_int(settings, key);

                                            std::scoped_lock<std::mutex> lock(self->data_mutex);

                                            self->update_crispness();
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::octaves",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<Pitch*>(user_data);

                                            self->octaves = g_settings_get_int(settings, key);

                                            std::scoped_lock<std::mutex> lock(self->data_mutex);

                                            self->update_pitch_scale();
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::semitones",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<Pitch*>(user_data);

                                            self->semitones = g_settings_get_int(settings, key);

                                            std::scoped_lock<std::mutex> lock(self->data_mutex);

                                            self->update_pitch_scale();
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::cents",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<Pitch*>(user_data);

                                            self->cents = g_settings_get_int(settings, key);

                                            std::scoped_lock<std::mutex> lock(self->data_mutex);

                                            self->update_pitch_scale();
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

  deque_out_L.resize(0);
  deque_out_R.resize(0);

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
    // util::debug(log_tag + name + " available: " + std::to_string(n_available));

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

    util::debug(log_tag + name + " latency: " + std::to_string(latency_value) + " s");

    util::idle_add([=, this]() {
      if (!post_messages) {
        return;
      }

      latency.emit(latency_value);
    });

    g_idle_add((GSourceFunc) +
                   [](gpointer user_data) {
                     if (!post_messages) {
                       return G_SOURCE_REMOVE;
                     }

                     auto* self = static_cast<Pitch*>(user_data);

                     if (self->latency.empty()) {
                       return G_SOURCE_REMOVE;
                     }

                     self->latency.emit(self->latency_value);

                     return G_SOURCE_REMOVE;
                   },
               this);

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
