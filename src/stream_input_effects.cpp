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

StreamInputEffects::StreamInputEffects(PipeManager* pipe_manager)
    : EffectsBase("sie: ", "com.github.wwmm.pulseeffects.sourceoutputs", pipe_manager) {
  // auto* PULSE_SOURCE = std::getenv("PULSE_SOURCE");

  // if (PULSE_SOURCE != nullptr) {
  //   int node_id = -1;

  //   for (const auto& node : pipe_manager->list_nodes) {
  //     if (node.name == PULSE_SOURCE) {
  //       node_id = node.id;

  //       break;
  //     }
  //   }

  //   if (node_id != -1) {
  //     set_input_node_id(node_id);
  //   }
  // } else {
  //   bool use_default_source = g_settings_get_boolean(settings, "use-default-source") != 0;

  //   if (!use_default_source) {
  //     gchar* custom_source = g_settings_get_string(settings, "custom-source");

  //     if (custom_source != nullptr) {
  //       int node_id = -1;

  //       for (const auto& node : pipe_manager->list_nodes) {
  //         if (node.name == custom_source) {
  //           node_id = node.id;

  //           break;
  //         }
  //       }

  //       if (node_id != -1) {
  //         set_output_node_id(node_id);
  //       }

  //       g_free(custom_source);
  //     }
  //   }
  // }

  pm->stream_input_added.connect(sigc::mem_fun(*this, &StreamInputEffects::on_app_added));
  pm->link_changed.connect(sigc::mem_fun(*this, &StreamInputEffects::on_link_changed));
  // pm->source_changed.connect(sigc::mem_fun(*this, &StreamInputEffects::on_source_changed));

  limiter = std::make_unique<Limiter>(log_tag, "com.github.wwmm.pulseeffects.limiter",
                                      "/com/github/wwmm/pulseeffects/sourceoutputs/limiter/", pm);

  compressor = std::make_unique<Compressor>(log_tag, "com.github.wwmm.pulseeffects.compressor",
                                            "/com/github/wwmm/pulseeffects/sourceoutputs/compressor/", pm);

  filter = std::make_unique<Filter>(log_tag, "com.github.wwmm.pulseeffects.filter",
                                    "/com/github/wwmm/pulseeffects/sourceoutputs/filter/", pm);

  equalizer = std::make_unique<Equalizer>(log_tag, "com.github.wwmm.pulseeffects.equalizer",
                                          "/com/github/wwmm/pulseeffects/sourceoutputs/equalizer/",
                                          "com.github.wwmm.pulseeffects.equalizer.channel",
                                          "/com/github/wwmm/pulseeffects/sourceoutputs/equalizer/leftchannel/",
                                          "/com/github/wwmm/pulseeffects/sourceoutputs/equalizer/rightchannel/", pm);

  reverb = std::make_unique<Reverb>(log_tag, "com.github.wwmm.pulseeffects.reverb",
                                    "/com/github/wwmm/pulseeffects/sourceoutputs/reverb/", pm);

  gate = std::make_unique<Gate>(log_tag, "com.github.wwmm.pulseeffects.gate",
                                "/com/github/wwmm/pulseeffects/sourceoutputs/gate/", pm);

  deesser = std::make_unique<Deesser>(log_tag, "com.github.wwmm.pulseeffects.deesser",
                                      "/com/github/wwmm/pulseeffects/sourceoutputs/deesser/", pm);

  pitch = std::make_unique<Pitch>(log_tag, "com.github.wwmm.pulseeffects.pitch",
                                  "/com/github/wwmm/pulseeffects/sourceoutputs/pitch/", pm);

  multiband_compressor =
      std::make_unique<MultibandCompressor>(log_tag, "com.github.wwmm.pulseeffects.multibandcompressor",
                                            "/com/github/wwmm/pulseeffects/sourceoutputs/multibandcompressor/", pm);

  multiband_gate = std::make_unique<MultibandGate>(log_tag, "com.github.wwmm.pulseeffects.multibandgate",
                                                   "/com/github/wwmm/pulseeffects/sourceoutputs/multibandgate/", pm);

  stereo_tools = std::make_unique<StereoTools>(log_tag, "com.github.wwmm.pulseeffects.stereotools",
                                               "/com/github/wwmm/pulseeffects/sourceoutputs/stereotools/", pm);

  maximizer = std::make_unique<Maximizer>(log_tag, "com.github.wwmm.pulseeffects.maximizer",
                                          "/com/github/wwmm/pulseeffects/sourceoutputs/maximizer/", pm);

  rnnoise = std::make_unique<RNNoise>(log_tag, "com.github.wwmm.pulseeffects.rnnoise",
                                      "/com/github/wwmm/pulseeffects/sourceoutputs/rnnoise/", pm);

  plugins.insert(std::make_pair(limiter->name, limiter));
  plugins.insert(std::make_pair(compressor->name, compressor));
  plugins.insert(std::make_pair(filter->name, filter));
  plugins.insert(std::make_pair(equalizer->name, equalizer));
  plugins.insert(std::make_pair(reverb->name, reverb));
  plugins.insert(std::make_pair(gate->name, gate));
  plugins.insert(std::make_pair(deesser->name, deesser));
  plugins.insert(std::make_pair(pitch->name, pitch));
  // plugins.insert(std::make_pair(webrtc->name, webrtc));
  plugins.insert(std::make_pair(multiband_compressor->name, multiband_compressor));
  plugins.insert(std::make_pair(multiband_gate->name, multiband_gate));
  plugins.insert(std::make_pair(stereo_tools->name, stereo_tools));
  plugins.insert(std::make_pair(maximizer->name, maximizer));
  plugins.insert(std::make_pair(rnnoise->name, rnnoise));

  connect_filters();
}

StreamInputEffects::~StreamInputEffects() {
  util::debug(log_tag + "destroyed");
}

void StreamInputEffects::on_app_added(NodeInfo node_info) {
  bool forbidden_app = false;
  bool connected = false;
  auto blocklist = settings->get_string_array("blocklist");

  forbidden_app =
      std::find(std::begin(blocklist), std::end(blocklist), Glib::ustring(node_info.name)) != std::end(blocklist);

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
    if (!forbidden_app && global_settings->get_boolean("process-all-inputs")) {
      pm->connect_stream_input(node_info);
    }
  }
}

void StreamInputEffects::on_link_changed(LinkInfo link_info) {
  if (pm->default_source.id == pm->pe_source_node.id) {
    return;
  }

  bool want_to_play = false;

  for (const auto& link : pm->list_links) {
    if (link.output_node_id == pm->pe_source_node.id) {
      if (link.state == PW_LINK_STATE_ACTIVE) {
        want_to_play = true;

        break;
      }
    }
  }

  if (want_to_play != apps_want_to_play) {
    apps_want_to_play = want_to_play;

    // update_pipeline_state();
  }
}

void StreamInputEffects::on_source_changed(NodeInfo node_info) {
  // auto id = get_input_node_id();

  // if (node_info.id == id) {
  //   if (node_info.rate != sampling_rate && node_info.rate != 0) {
  //     util::debug(log_tag + "pulseeffects_source sampling rate has changed. Restarting the pipeline...");

  //     gst_element_set_state(pipeline, GST_STATE_NULL);

  //     set_sampling_rate(node_info.rate);

  //     rnnoise->set_caps_out(sampling_rate);

  //     update_pipeline_state();
  //   }
  // }
}

void StreamInputEffects::change_input_device(const NodeInfo& node) {
  // if (node.id == get_input_node_id()) {
  //   return;
  // }

  // util::debug(log_tag + "The user has requested a new input device. Restarting the pipeline...");

  // gst_element_set_state(pipeline, GST_STATE_NULL);

  // if (node.rate != 0) {
  //   set_sampling_rate(node.rate);
  // }

  // set_input_node_id(node.id);

  // rnnoise->set_caps_out(sampling_rate);

  // update_pipeline_state();
}

void StreamInputEffects::connect_filters() {
  // pm->lock();

  // pm->link_nodes(pm->pe_sink_node.id, delay->get_node_id());

  // pm->link_nodes(delay->get_node_id(), pm->default_sink.id);

  // pw_core_sync(pm->core, PW_ID_CORE, 0);

  // pw_thread_loop_wait(pm->thread_loop);

  // pm->unlock();
}