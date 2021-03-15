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
#include <ranges>

StreamOutputEffects::StreamOutputEffects(PipeManager* pipe_manager)
    : EffectsBase("soe: ", "com.github.wwmm.pulseeffects.sinkinputs", pipe_manager) {
  // auto* PULSE_SINK = std::getenv("PULSE_SINK");

  // if (PULSE_SINK != nullptr) {
  //   int node_id = -1;

  //   for (const auto& node : pipe_manager->list_nodes) {
  //     if (node.name == PULSE_SINK) {
  //       node_id = node.id;

  //       break;
  //     }
  //   }

  //   if (node_id != -1) {
  //     set_output_node_id(node_id);
  //   }
  // } else {
  //   bool use_default_sink = g_settings_get_boolean(settings, "use-default-sink") != 0;

  //   if (!use_default_sink) {
  //     gchar* custom_sink = g_settings_get_string(settings, "custom-sink");

  //     if (custom_sink != nullptr) {
  //       int node_id = -1;

  //       for (const auto& node : pipe_manager->list_nodes) {
  //         if (node.name == custom_sink) {
  //           node_id = node.id;

  //           break;
  //         }
  //       }

  //       if (node_id != -1) {
  //         set_output_node_id(node_id);
  //       }

  //       g_free(custom_sink);
  //     }
  //   }
  // }

  pm->stream_output_added.connect(sigc::mem_fun(*this, &StreamOutputEffects::on_app_added));
  pm->link_changed.connect(sigc::mem_fun(*this, &StreamOutputEffects::on_link_changed));

  pm->lock();

  connect_filters();

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pm->unlock();
}

StreamOutputEffects::~StreamOutputEffects() {
  util::debug(log_tag + "destroyed");
}

void StreamOutputEffects::on_app_added(NodeInfo node_info) {
  bool forbidden_app = false;
  bool connected = false;
  auto blocklist = settings->get_string_array("blocklist");

  forbidden_app = std::ranges::find(blocklist, Glib::ustring(node_info.name)) != blocklist.end();

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
    if (!forbidden_app && global_settings->get_boolean("process-all-outputs")) {
      pm->connect_stream_output(node_info);
    }
  }
}

void StreamOutputEffects::on_link_changed(LinkInfo link_info) {
  bool want_to_play = false;

  for (const auto& link : pm->list_links) {
    if (link.input_node_id == pm->pe_sink_node.id) {
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

void StreamOutputEffects::change_output_device(const NodeInfo& node) {
  // if (node.id == get_output_node_id()) {
  //   return;
  // }

  // util::debug(log_tag + "The user has requested a new output device. Restarting the pipeline...");

  // gst_element_set_state(pipeline, GST_STATE_NULL);

  // if (node.rate != 0) {
  //   set_sampling_rate(node.rate);
  // }

  // set_output_node_id(node.id);

  // rnnoise->set_caps_out(sampling_rate);

  // update_pipeline_state();
}

void StreamOutputEffects::connect_filters() {
  auto list = settings->get_string_array("selected-plugins");

  // for (auto& p : plugins | std::views::keys) {
  //   std::cout << p << std::endl;
  // }

  if (list.empty()) {
    pm->link_nodes(pm->pe_sink_node.id, spectrum->get_node_id());
  } else {
    pm->link_nodes(pm->pe_sink_node.id, plugins[list[0]]->get_node_id());

    for (size_t n = 1; n < list.size(); n++) {
      pm->link_nodes(plugins[list[n - 1]]->get_node_id(), plugins[list[n]]->get_node_id());
    }

    pm->link_nodes(plugins[list[list.size() - 1]]->get_node_id(), spectrum->get_node_id());
  }

  pm->link_nodes(spectrum->get_node_id(), output_level->get_node_id());

  pm->link_nodes(output_level->get_node_id(), pm->default_sink.id);
}