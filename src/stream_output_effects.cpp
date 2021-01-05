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

#include "stream_output_effects.hpp"
#include <cstring>
#include <string>
#include "pipeline_common.hpp"
#include "rnnoise.hpp"
#include "util.hpp"

namespace {

void on_message_element(const GstBus* gst_bus, GstMessage* message, StreamOutputEffects* soe) {
  auto* src_name = GST_OBJECT_NAME(message->src);

  // To optimize this call we move at the top of the nested "if statements" the most used messages
  // which are 'global_level_meter' and the level meters for the most used plugins for sink inputs:
  // equalizer and autogain. The rest is sorted alphabetically.

  if (std::strcmp(src_name, "global_level_meter") == 0) {
    soe->global_output_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "equalizer_input_level") == 0) {
    soe->equalizer_input_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "equalizer_output_level") == 0) {
    soe->equalizer_output_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "autogain_input_level") == 0) {
    soe->autogain_input_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "autogain_output_level") == 0) {
    soe->autogain_output_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "bass_enhancer_input_level") == 0) {
    soe->bass_enhancer_input_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "bass_enhancer_output_level") == 0) {
    soe->bass_enhancer_output_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "convolver_input_level") == 0) {
    soe->convolver_input_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "convolver_output_level") == 0) {
    soe->convolver_output_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "crossfeed_input_level") == 0) {
    soe->crossfeed_input_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "crossfeed_output_level") == 0) {
    soe->crossfeed_output_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "crystalizer_input_level") == 0) {
    soe->crystalizer_input_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "crystalizer_output_level") == 0) {
    soe->crystalizer_output_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "deesser_input_level") == 0) {
    soe->deesser_input_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "deesser_output_level") == 0) {
    soe->deesser_output_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "delay_input_level") == 0) {
    soe->delay_input_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "delay_output_level") == 0) {
    soe->delay_output_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "exciter_input_level") == 0) {
    soe->exciter_input_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "exciter_output_level") == 0) {
    soe->exciter_output_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "gate_input_level") == 0) {
    soe->gate_input_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "gate_output_level") == 0) {
    soe->gate_output_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "loudness_input_level") == 0) {
    soe->loudness_input_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "loudness_output_level") == 0) {
    soe->loudness_output_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "maximizer_input_level") == 0) {
    soe->maximizer_input_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "maximizer_output_level") == 0) {
    soe->maximizer_output_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "pitch_input_level") == 0) {
    soe->pitch_input_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "pitch_output_level") == 0) {
    soe->pitch_output_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "rnnoise_input_level") == 0) {
    soe->rnnoise_input_level.emit(StreamOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "rnnoise_output_level") == 0) {
    soe->rnnoise_output_level.emit(StreamOutputEffects::get_peak(message));
  }
}

void on_latency_changed(GSettings* settings, gchar* key, StreamOutputEffects* soe) {
  gst_element_set_state(soe->pipeline, GST_STATE_NULL);

  soe->set_latency();

  // int blocksize = 2;

  // while (blocksize < desired_buffer_size) {
  //   if (blocksize * 2 > desired_buffer_size) {
  //     break;
  //   }

  //   blocksize *= 2;
  // }

  soe->update_pipeline_state();
}

}  // namespace

StreamOutputEffects::StreamOutputEffects(PipeManager* pipe_manager) : PipelineBase("soe: ", pipe_manager) {
  pipe_props += ",application.id=com.github.wwmm.pulseeffects.streamoutputs";

  child_settings = g_settings_new("com.github.wwmm.pulseeffects.sinkinputs");

  auto default_output = pipe_manager->get_default_sink();

  set_input_node_id(pm->pe_sink_node.id);
  set_output_node_id(default_output.id);

  set_sampling_rate(48000);  // 48 kHz is the default pipewire sampling rate

  set_latency();

  auto* PULSE_SINK = std::getenv("PULSE_SINK");

  if (PULSE_SINK != nullptr) {
    int node_id = -1;

    for (const auto& node : pipe_manager->list_nodes) {
      if (node.name == PULSE_SINK) {
        node_id = node.id;

        break;
      }
    }

    if (node_id != -1) {
      set_output_node_id(node_id);
    }
  } else {
    bool use_default_sink = g_settings_get_boolean(settings, "use-default-sink") != 0;

    if (!use_default_sink) {
      gchar* custom_sink = g_settings_get_string(settings, "custom-sink");

      if (custom_sink != nullptr) {
        int node_id = -1;

        for (const auto& node : pipe_manager->list_nodes) {
          if (node.name == custom_sink) {
            node_id = node.id;

            break;
          }
        }

        if (node_id != -1) {
          set_output_node_id(node_id);
        }

        g_free(custom_sink);
      }
    }
  }

  pm->stream_output_added.connect(sigc::mem_fun(*this, &StreamOutputEffects::on_app_added));
  pm->stream_output_changed.connect(sigc::mem_fun(*this, &StreamOutputEffects::on_app_changed));
  pm->sink_changed.connect(sigc::mem_fun(*this, &StreamOutputEffects::on_sink_changed));

  // element message callback

  g_signal_connect(bus, "message::element", G_CALLBACK(on_message_element), this);

  limiter = std::make_unique<Limiter>(log_tag, "com.github.wwmm.pulseeffects.limiter",
                                      "/com/github/wwmm/pulseeffects/sinkinputs/limiter/");

  compressor = std::make_unique<Compressor>(log_tag, "com.github.wwmm.pulseeffects.compressor",
                                            "/com/github/wwmm/pulseeffects/sinkinputs/compressor/");

  filter = std::make_unique<Filter>(log_tag, "com.github.wwmm.pulseeffects.filter",
                                    "/com/github/wwmm/pulseeffects/sinkinputs/filter/");

  equalizer = std::make_unique<Equalizer>(log_tag, "com.github.wwmm.pulseeffects.equalizer",
                                          "/com/github/wwmm/pulseeffects/sinkinputs/equalizer/",
                                          "com.github.wwmm.pulseeffects.equalizer.channel",
                                          "/com/github/wwmm/pulseeffects/sinkinputs/equalizer/leftchannel/",
                                          "/com/github/wwmm/pulseeffects/sinkinputs/equalizer/rightchannel/");

  reverb = std::make_unique<Reverb>(log_tag, "com.github.wwmm.pulseeffects.reverb",
                                    "/com/github/wwmm/pulseeffects/sinkinputs/reverb/");

  bass_enhancer = std::make_unique<BassEnhancer>(log_tag, "com.github.wwmm.pulseeffects.bassenhancer",
                                                 "/com/github/wwmm/pulseeffects/sinkinputs/bassenhancer/");

  exciter = std::make_unique<Exciter>(log_tag, "com.github.wwmm.pulseeffects.exciter",
                                      "/com/github/wwmm/pulseeffects/sinkinputs/exciter/");

  crossfeed = std::make_unique<Crossfeed>(log_tag, "com.github.wwmm.pulseeffects.crossfeed",
                                          "/com/github/wwmm/pulseeffects/sinkinputs/crossfeed/");

  maximizer = std::make_unique<Maximizer>(log_tag, "com.github.wwmm.pulseeffects.maximizer",
                                          "/com/github/wwmm/pulseeffects/sinkinputs/maximizer/");

  multiband_compressor =
      std::make_unique<MultibandCompressor>(log_tag, "com.github.wwmm.pulseeffects.multibandcompressor",
                                            "/com/github/wwmm/pulseeffects/sinkinputs/multibandcompressor/");

  loudness = std::make_unique<Loudness>(log_tag, "com.github.wwmm.pulseeffects.loudness",
                                        "/com/github/wwmm/pulseeffects/sinkinputs/loudness/");

  gate = std::make_unique<Gate>(log_tag, "com.github.wwmm.pulseeffects.gate",
                                "/com/github/wwmm/pulseeffects/sinkinputs/gate/");

  pitch = std::make_unique<Pitch>(log_tag, "com.github.wwmm.pulseeffects.pitch",
                                  "/com/github/wwmm/pulseeffects/sinkinputs/pitch/");

  multiband_gate = std::make_unique<MultibandGate>(log_tag, "com.github.wwmm.pulseeffects.multibandgate",
                                                   "/com/github/wwmm/pulseeffects/sinkinputs/multibandgate/");

  deesser = std::make_unique<Deesser>(log_tag, "com.github.wwmm.pulseeffects.deesser",
                                      "/com/github/wwmm/pulseeffects/sinkinputs/deesser/");

  stereo_tools = std::make_unique<StereoTools>(log_tag, "com.github.wwmm.pulseeffects.stereotools",
                                               "/com/github/wwmm/pulseeffects/sinkinputs/stereotools/");

  convolver = std::make_unique<Convolver>(log_tag, "com.github.wwmm.pulseeffects.convolver",
                                          "/com/github/wwmm/pulseeffects/sinkinputs/convolver/");

  crystalizer = std::make_unique<Crystalizer>(log_tag, "com.github.wwmm.pulseeffects.crystalizer",
                                              "/com/github/wwmm/pulseeffects/sinkinputs/crystalizer/");

  autogain = std::make_unique<AutoGain>(log_tag, "com.github.wwmm.pulseeffects.autogain",
                                        "/com/github/wwmm/pulseeffects/sinkinputs/autogain/");

  delay = std::make_unique<Delay>(log_tag, "com.github.wwmm.pulseeffects.delay",
                                  "/com/github/wwmm/pulseeffects/sinkinputs/delay/");

  rnnoise = std::make_unique<RNNoise>(log_tag, "com.github.wwmm.pulseeffects.rnnoise",
                                      "/com/github/wwmm/pulseeffects/sinkinputs/rnnoise/");

  // doing some plugin configurations

  rnnoise->set_caps_out(sampling_rate);

  g_object_set(convolver->adapter, "blocksize", 512, nullptr);

  g_object_set(crystalizer->adapter, "blocksize", 512, nullptr);

  // inserting the plugins in the containers

  plugins.insert(std::make_pair(limiter->name, limiter->plugin));
  plugins.insert(std::make_pair(compressor->name, compressor->plugin));
  plugins.insert(std::make_pair(filter->name, filter->plugin));
  plugins.insert(std::make_pair(equalizer->name, equalizer->plugin));
  plugins.insert(std::make_pair(reverb->name, reverb->plugin));
  plugins.insert(std::make_pair(bass_enhancer->name, bass_enhancer->plugin));
  plugins.insert(std::make_pair(exciter->name, exciter->plugin));
  plugins.insert(std::make_pair(crossfeed->name, crossfeed->plugin));
  plugins.insert(std::make_pair(maximizer->name, maximizer->plugin));
  plugins.insert(std::make_pair(multiband_compressor->name, multiband_compressor->plugin));
  plugins.insert(std::make_pair(loudness->name, loudness->plugin));
  plugins.insert(std::make_pair(gate->name, gate->plugin));
  plugins.insert(std::make_pair(pitch->name, pitch->plugin));
  plugins.insert(std::make_pair(multiband_gate->name, multiband_gate->plugin));
  plugins.insert(std::make_pair(deesser->name, deesser->plugin));
  plugins.insert(std::make_pair(stereo_tools->name, stereo_tools->plugin));
  plugins.insert(std::make_pair(convolver->name, convolver->plugin));
  plugins.insert(std::make_pair(crystalizer->name, crystalizer->plugin));
  plugins.insert(std::make_pair(autogain->name, autogain->plugin));
  plugins.insert(std::make_pair(delay->name, delay->plugin));
  plugins.insert(std::make_pair(rnnoise->name, rnnoise->plugin));

  add_plugins_to_pipeline();

  g_signal_connect(child_settings, "changed::plugins", G_CALLBACK(on_plugins_order_changed<StreamOutputEffects>), this);

  g_signal_connect(child_settings, "changed::latency", G_CALLBACK(on_latency_changed), this);
}

StreamOutputEffects::~StreamOutputEffects() {
  util::debug(log_tag + "destroyed");
}

void StreamOutputEffects::on_app_added(const NodeInfo& node_info) {
  bool forbidden_app = false;
  bool connected = false;
  auto* blocklist = g_settings_get_strv(settings, "blocklist-out");

  for (std::size_t i = 0; blocklist[i] != nullptr; i++) {
    if (node_info.name == blocklist[i]) {
      forbidden_app = true;
    }

    g_free(blocklist[i]);
  }

  for (const auto& link : pm->list_links) {
    if (link.output_node_id == node_info.id && link.input_node_id == pm->pe_sink_node.id) {
      connected = true;

      break;
    }
  }

  if (connected) {
    if (forbidden_app) {
      pm->disconnect_stream_output(node_info);
    }
  } else {
    auto enable_all = g_settings_get_boolean(settings, "enable-all-sinkinputs");

    if (!forbidden_app && enable_all != 0) {
      pm->connect_stream_output(node_info);
    }
  }

  g_free(blocklist);
}

void StreamOutputEffects::on_app_changed(const NodeInfo& node_info) {
  apps_want_to_play = false;

  for (const auto& link : pm->list_links) {
    if (link.input_node_id == pm->pe_sink_node.id) {
      for (const auto& node : pm->list_nodes) {
        if (node.id == link.output_node_id && node.state == PW_NODE_STATE_RUNNING) {
          apps_want_to_play = true;
        }
      }
    }
  }

  update_pipeline_state();
}

void StreamOutputEffects::on_sink_changed(const NodeInfo& node_info) {
  if (node_info.name == "pulseeffects_sink") {
    if (node_info.rate != sampling_rate && node_info.rate != 0) {
      gst_element_set_state(pipeline, GST_STATE_NULL);

      set_sampling_rate(node_info.rate);

      rnnoise->set_caps_out(sampling_rate);

      update_pipeline_state();
    }
  }
}

void StreamOutputEffects::change_output_device(const NodeInfo& node) {
  gst_element_set_state(pipeline, GST_STATE_NULL);

  if (node.rate != 0) {
    set_sampling_rate(node.rate);
  }

  set_output_node_id(node.id);

  rnnoise->set_caps_out(sampling_rate);

  update_pipeline_state();
}

void StreamOutputEffects::add_plugins_to_pipeline() {
  gchar* name = nullptr;
  GVariantIter* iter = nullptr;
  std::vector<std::string> default_order;

  g_settings_get(child_settings, "plugins", "as", &iter);

  while (g_variant_iter_next(iter, "s", &name) != 0) {
    plugins_order.emplace_back(name);
    g_free(name);
  }

  auto* gvariant = g_settings_get_default_value(child_settings, "plugins");

  g_variant_get(gvariant, "as", &iter);

  g_variant_unref(gvariant);

  while (g_variant_iter_next(iter, "s", &name) != 0) {
    default_order.emplace_back(name);
    g_free(name);
  }

  g_variant_iter_free(iter);

  // updating user list if there is any new plugin

  if (plugins_order.size() != default_order.size()) {
    plugins_order = default_order;

    g_settings_reset(child_settings, "plugins");
  }

  for (const auto& v : plugins_order) {
    // checking if the plugin exists. If not we reset the list to default

    if (std::find(default_order.begin(), default_order.end(), v) == default_order.end()) {
      plugins_order = default_order;

      g_settings_reset(child_settings, "plugins");

      break;
    }
  }

  // adding plugins to effects_bin

  for (const auto& p : plugins) {
    gst_bin_add(GST_BIN(effects_bin), p.second);
  }

  // linking plugins

  gst_element_unlink(identity_in, identity_out);

  gst_element_link(identity_in, plugins[plugins_order[0]]);

  for (unsigned long int n = 1U; n < plugins_order.size(); n++) {
    gst_element_link(plugins[plugins_order[n - 1U]], plugins[plugins_order[n]]);
  }

  gst_element_link(plugins[plugins_order[plugins_order.size() - 1U]], identity_out);
}
