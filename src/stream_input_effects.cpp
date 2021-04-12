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

  // uint n_disconnected_links = disconnect_filters();

  // if (n_disconnected_links != 0) {
  //   util::warning(log_tag + "disconnecting " + std::to_string(n_disconnected_links) +
  //                 " links in the initialization phase?!");
  // }

  connect_filters();

  settings->signal_changed("selected-plugins").connect([&, this](auto key) {
    // disconnect_filters();
    pm->destroy_links(list_proxies);

    list_proxies.clear();

    connect_filters();
  });
}

StreamInputEffects::~StreamInputEffects() {
  util::debug(log_tag + "destroyed");

  // disconnect_filters();
  pm->destroy_links(list_proxies);

  list_proxies.clear();
}

void StreamInputEffects::on_app_added(const NodeInfo& node_info) {
  bool forbidden_app = false;
  bool connected = false;
  auto blocklist = settings->get_string_array("blocklist");

  forbidden_app = std::ranges::find(blocklist, Glib::ustring(node_info.name)) != blocklist.end();

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

void StreamInputEffects::on_link_changed(const LinkInfo& link_info) {
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

void StreamInputEffects::on_source_changed(const NodeInfo& node_info) {
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
  auto list = settings->get_string_array("selected-plugins");

  if (list.empty()) {
    auto links = pm->link_nodes(pm->default_source.id, spectrum->get_node_id());

    for (const auto& link : links) {
      list_proxies.emplace_back(link);
    }
  } else {
    auto links = pm->link_nodes(pm->default_source.id, plugins[list[0]]->get_node_id());

    for (const auto& link : links) {
      list_proxies.emplace_back(link);
    }

    for (size_t n = 1; n < list.size(); n++) {
      auto links = pm->link_nodes(plugins[list[n - 1]]->get_node_id(), plugins[list[n]]->get_node_id());

      for (const auto& link : links) {
        list_proxies.emplace_back(link);
      }
    }

    links = pm->link_nodes(plugins[list[list.size() - 1]]->get_node_id(), spectrum->get_node_id());

    for (const auto& link : links) {
      list_proxies.emplace_back(link);
    }
  }

  auto links = pm->link_nodes(spectrum->get_node_id(), output_level->get_node_id());

  for (const auto& link : links) {
    list_proxies.emplace_back(link);
  }

  links = pm->link_nodes(output_level->get_node_id(), pm->pe_source_node.id);

  for (const auto& link : links) {
    list_proxies.emplace_back(link);
  }
}

auto StreamInputEffects::disconnect_filters() -> uint {
  std::set<uint> list;

  for (auto& plugin : plugins | std::views::values) {
    for (auto& link : pm->list_links) {
      if (link.input_node_id == plugin->get_node_id() || link.output_node_id == plugin->get_node_id()) {
        list.insert(link.id);
      }
    }
  }

  for (auto& link : pm->list_links) {
    if (link.input_node_id == spectrum->get_node_id() || link.output_node_id == spectrum->get_node_id() ||
        link.input_node_id == output_level->get_node_id() || link.output_node_id == output_level->get_node_id()) {
      list.insert(link.id);
    }
  }

  for (const auto& id : list) {
    pm->destroy_object(id);
  }

  return list.size();
}