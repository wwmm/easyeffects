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

#include "stream_input_effects.hpp"
#include <cstring>
#include "pipeline_common.hpp"
#include "rnnoise.hpp"

namespace {

void on_message_element(const GstBus* gst_bus, GstMessage* message, StreamInputEffects* sie) {
  auto* src_name = GST_OBJECT_NAME(message->src);

  // To optimize this call we move at the top of the nested "if statements" the most used messages
  // which are 'global_level_meter' and the level meters for the most used plugins for source outputs:
  // equalizer and webrtc. The rest is sorted alphabetically.

  if (std::strcmp(src_name, "global_level_meter") == 0) {
    sie->global_output_level.emit(StreamInputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "equalizer_input_level") == 0) {
    sie->equalizer_input_level.emit(StreamInputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "equalizer_output_level") == 0) {
    sie->equalizer_output_level.emit(StreamInputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "webrtc_input_level") == 0) {
    sie->webrtc_input_level.emit(StreamInputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "webrtc_output_level") == 0) {
    sie->webrtc_output_level.emit(StreamInputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "deesser_input_level") == 0) {
    sie->deesser_input_level.emit(StreamInputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "deesser_output_level") == 0) {
    sie->deesser_output_level.emit(StreamInputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "gate_input_level") == 0) {
    sie->gate_input_level.emit(StreamInputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "gate_output_level") == 0) {
    sie->gate_output_level.emit(StreamInputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "maximizer_input_level") == 0) {
    sie->maximizer_input_level.emit(StreamInputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "maximizer_output_level") == 0) {
    sie->maximizer_output_level.emit(StreamInputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "pitch_input_level") == 0) {
    sie->pitch_input_level.emit(StreamInputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "pitch_output_level") == 0) {
    sie->pitch_output_level.emit(StreamInputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "rnnoise_input_level") == 0) {
    sie->rnnoise_input_level.emit(StreamInputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "rnnoise_output_level") == 0) {
    sie->rnnoise_output_level.emit(StreamInputEffects::get_peak(message));
  }
}

void on_latency_changed(GSettings* settings, gchar* key, StreamInputEffects* sie) {
  gst_element_set_state(sie->pipeline, GST_STATE_NULL);

  sie->set_latency();

  sie->update_pipeline_state();
}

}  // namespace

StreamInputEffects::StreamInputEffects(PipeManager* pipe_manager) : PipelineBase("sie: ", pipe_manager) {
  pipe_props += ",node.name=pulseeffects_sie,application.id=com.github.wwmm.pulseeffects.streaminputs";

  child_settings = g_settings_new("com.github.wwmm.pulseeffects.sourceoutputs");

  auto default_input = pipe_manager->default_source;

  set_input_node_id(default_input.id);

  set_output_node_id(pm->pe_source_node.id);

  set_sampling_rate(48000);  // 48 kHz is the default pipewire sampling rate

  set_latency();

  auto* PULSE_SOURCE = std::getenv("PULSE_SOURCE");

  if (PULSE_SOURCE != nullptr) {
    int node_id = -1;

    for (const auto& node : pipe_manager->list_nodes) {
      if (node.name == PULSE_SOURCE) {
        node_id = node.id;

        break;
      }
    }

    if (node_id != -1) {
      set_input_node_id(node_id);
    }
  } else {
    bool use_default_source = g_settings_get_boolean(settings, "use-default-source") != 0;

    if (!use_default_source) {
      gchar* custom_source = g_settings_get_string(settings, "custom-source");

      if (custom_source != nullptr) {
        int node_id = -1;

        for (const auto& node : pipe_manager->list_nodes) {
          if (node.name == custom_source) {
            node_id = node.id;

            break;
          }
        }

        if (node_id != -1) {
          set_output_node_id(node_id);
        }

        g_free(custom_source);
      }
    }
  }

  pm->stream_input_added.connect(sigc::mem_fun(*this, &StreamInputEffects::on_app_added));
  pm->link_changed.connect(sigc::mem_fun(*this, &StreamInputEffects::on_link_changed));
  pm->source_changed.connect(sigc::mem_fun(*this, &StreamInputEffects::on_source_changed));

  // element message callback

  g_signal_connect(bus, "message::element", G_CALLBACK(on_message_element), this);

  limiter = std::make_unique<Limiter>(log_tag, "com.github.wwmm.pulseeffects.limiter",
                                      "/com/github/wwmm/pulseeffects/sourceoutputs/limiter/");

  compressor = std::make_unique<Compressor>(log_tag, "com.github.wwmm.pulseeffects.compressor",
                                            "/com/github/wwmm/pulseeffects/sourceoutputs/compressor/");

  filter = std::make_unique<Filter>(log_tag, "com.github.wwmm.pulseeffects.filter",
                                    "/com/github/wwmm/pulseeffects/sourceoutputs/filter/");

  equalizer = std::make_unique<Equalizer>(log_tag, "com.github.wwmm.pulseeffects.equalizer",
                                          "/com/github/wwmm/pulseeffects/sourceoutputs/equalizer/",
                                          "com.github.wwmm.pulseeffects.equalizer.channel",
                                          "/com/github/wwmm/pulseeffects/sourceoutputs/equalizer/leftchannel/",
                                          "/com/github/wwmm/pulseeffects/sourceoutputs/equalizer/rightchannel/");

  reverb = std::make_unique<Reverb>(log_tag, "com.github.wwmm.pulseeffects.reverb",
                                    "/com/github/wwmm/pulseeffects/sourceoutputs/reverb/");

  gate = std::make_unique<Gate>(log_tag, "com.github.wwmm.pulseeffects.gate",
                                "/com/github/wwmm/pulseeffects/sourceoutputs/gate/");

  deesser = std::make_unique<Deesser>(log_tag, "com.github.wwmm.pulseeffects.deesser",
                                      "/com/github/wwmm/pulseeffects/sourceoutputs/deesser/");

  pitch = std::make_unique<Pitch>(log_tag, "com.github.wwmm.pulseeffects.pitch",
                                  "/com/github/wwmm/pulseeffects/sourceoutputs/pitch/");

  webrtc = std::make_unique<Webrtc>(log_tag, "com.github.wwmm.pulseeffects.webrtc",
                                    "/com/github/wwmm/pulseeffects/sourceoutputs/webrtc/",
                                    48000 /* pm->mic_sink_info->rate*/);

  multiband_compressor =
      std::make_unique<MultibandCompressor>(log_tag, "com.github.wwmm.pulseeffects.multibandcompressor",
                                            "/com/github/wwmm/pulseeffects/sourceoutputs/multibandcompressor/");

  multiband_gate = std::make_unique<MultibandGate>(log_tag, "com.github.wwmm.pulseeffects.multibandgate",
                                                   "/com/github/wwmm/pulseeffects/sourceoutputs/multibandgate/");

  stereo_tools = std::make_unique<StereoTools>(log_tag, "com.github.wwmm.pulseeffects.stereotools",
                                               "/com/github/wwmm/pulseeffects/sourceoutputs/stereotools/");

  maximizer = std::make_unique<Maximizer>(log_tag, "com.github.wwmm.pulseeffects.maximizer",
                                          "/com/github/wwmm/pulseeffects/sourceoutputs/maximizer/");

  rnnoise = std::make_unique<RNNoise>(log_tag, "com.github.wwmm.pulseeffects.rnnoise",
                                      "/com/github/wwmm/pulseeffects/sourceoutputs/rnnoise/");

  rnnoise->set_caps_out(sampling_rate);

  plugins.insert(std::make_pair(limiter->name, limiter->plugin));
  plugins.insert(std::make_pair(compressor->name, compressor->plugin));
  plugins.insert(std::make_pair(filter->name, filter->plugin));
  plugins.insert(std::make_pair(equalizer->name, equalizer->plugin));
  plugins.insert(std::make_pair(reverb->name, reverb->plugin));
  plugins.insert(std::make_pair(gate->name, gate->plugin));
  plugins.insert(std::make_pair(deesser->name, deesser->plugin));
  plugins.insert(std::make_pair(pitch->name, pitch->plugin));
  plugins.insert(std::make_pair(webrtc->name, webrtc->plugin));
  plugins.insert(std::make_pair(multiband_compressor->name, multiband_compressor->plugin));
  plugins.insert(std::make_pair(multiband_gate->name, multiband_gate->plugin));
  plugins.insert(std::make_pair(stereo_tools->name, stereo_tools->plugin));
  plugins.insert(std::make_pair(maximizer->name, maximizer->plugin));
  plugins.insert(std::make_pair(rnnoise->name, rnnoise->plugin));

  add_plugins_to_pipeline();

  g_signal_connect(child_settings, "changed::plugins", G_CALLBACK(on_plugins_order_changed<StreamInputEffects>), this);

  g_signal_connect(child_settings, "changed::latency", G_CALLBACK(on_latency_changed), this);
}

StreamInputEffects::~StreamInputEffects() {
  util::debug(log_tag + "destroyed");
}

void StreamInputEffects::on_app_added(NodeInfo node_info) {
  bool forbidden_app = false;
  bool connected = false;
  auto* blocklist = g_settings_get_strv(settings, "blocklist-in");

  for (std::size_t i = 0; blocklist[i] != nullptr; i++) {
    if (node_info.name == blocklist[i]) {
      forbidden_app = true;
    }

    g_free(blocklist[i]);
  }

  for (const auto& link : pm->list_links) {
    if (link.input_node_id == node_info.id && link.output_node_id == pm->pe_source_node.id) {
      connected = true;

      break;
    }
  }

  if (connected) {
    if (forbidden_app) {
      pm->disconnect_stream_input(node_info);
    }
  } else {
    auto enable_all = g_settings_get_boolean(settings, "enable-all-sourceoutputs");

    if (!forbidden_app && enable_all != 0) {
      pm->connect_stream_input(node_info);
    }
  }

  g_free(blocklist);
}

void StreamInputEffects::on_link_changed(LinkInfo link_info) {
  apps_want_to_play = false;

  for (const auto& link : pm->list_links) {
    if (link.output_node_id == pm->pe_source_node.id) {
      if (link.state == PW_LINK_STATE_ACTIVE) {
        for (const auto& node : pm->list_nodes) {
          if (node.id == link.input_node_id) {
            apps_want_to_play = true;

            break;
          }
        }
      }
    }
  }

  update_pipeline_state();
}

void StreamInputEffects::on_source_changed(NodeInfo node_info) {
  auto id = get_input_node_id();

  if (node_info.id == id) {
    if (node_info.rate != sampling_rate && node_info.rate != 0) {
      gst_element_set_state(pipeline, GST_STATE_NULL);

      set_sampling_rate(node_info.rate);

      rnnoise->set_caps_out(sampling_rate);

      update_pipeline_state();
    }
  }
}

void StreamInputEffects::change_input_device(const NodeInfo& node) {
  gst_element_set_state(pipeline, GST_STATE_NULL);

  if (node.rate != 0) {
    set_sampling_rate(node.rate);
  }

  set_input_node_id(node.id);

  rnnoise->set_caps_out(sampling_rate);

  update_pipeline_state();
}

void StreamInputEffects::add_plugins_to_pipeline() {
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
