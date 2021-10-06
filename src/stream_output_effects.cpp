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

#include "stream_output_effects.hpp"

StreamOutputEffects::StreamOutputEffects(PipeManager* pipe_manager)
    : EffectsBase("soe: ", "com.github.wwmm.easyeffects.streamoutputs", pipe_manager) {
  pm->output_device = pm->default_output_device;

  if (settings->get_boolean("use-default-output-device")) {
    settings->set_string("output-device", pm->output_device.name);
  } else {
    auto found = false;

    const auto* output_device = settings->get_string("output-device").c_str();

    if (output_device != pm->ee_sink_name) {
      for (const auto& [ts, node] : pm->node_map) {
        if (node.name == output_device) {
          pm->output_device = node;

          found = true;

          break;
        }
      }
    }

    if (!found) {
      settings->set_string("output-device", pm->output_device.name);
    }
  }

  auto* PULSE_SINK = std::getenv("PULSE_SINK");

  if (PULSE_SINK != nullptr && PULSE_SINK != pm->ee_sink_name) {
    for (const auto& [ts, node] : pm->node_map) {
      if (node.name == PULSE_SINK) {
        pm->output_device = node;

        settings->set_string("output-device", pm->output_device.name);

        break;
      }
    }
  }

  pm->stream_output_added.connect(sigc::mem_fun(*this, &StreamOutputEffects::on_app_added));
  pm->link_changed.connect(sigc::mem_fun(*this, &StreamOutputEffects::on_link_changed));

  connect_filters();

  auto reset_filter_connection = [=, this]() {
    if (global_settings->get_boolean("bypass")) {
      global_settings->set_boolean("bypass", false);

      return;  // filter connected through update_bypass_state
    }

    set_bypass(false);
  };

  settings->signal_changed("output-device").connect([=, this](const auto& key) {
    const auto name = settings->get_string(key).raw();

    if (name.empty()) {
      return;
    }

    for (const auto& [ts, node] : pm->node_map) {
      if (node.name == name) {
        pm->output_device = node;

        reset_filter_connection();

        break;
      }
    }
  });

  settings->signal_changed("plugins").connect([=, this](const auto& key) { reset_filter_connection(); });
}

StreamOutputEffects::~StreamOutputEffects() {
  disconnect_filters();

  util::debug(log_tag + "destroyed");
}

void StreamOutputEffects::on_app_added(const NodeInfo node_info) {
  const auto& blocklist = settings->get_string_array("blocklist");

  const auto& is_blocklisted = std::ranges::find(blocklist, node_info.name.c_str()) != blocklist.end();

  if (is_blocklisted) {
    pm->disconnect_stream_output(node_info.id);
  } else if (global_settings->get_boolean("process-all-outputs")) {
    pm->connect_stream_output(node_info.id);
  }
}

void StreamOutputEffects::on_link_changed(const LinkInfo link_info) {
  /*
    If bypass is enabled do not touch the plugin pipeline
  */

  if (bypass) {
    return;
  }

  auto want_to_play = false;

  for (const auto& link : pm->list_links) {
    if (link.input_node_id == pm->ee_sink_node.id) {
      if (link.state == PW_LINK_STATE_ACTIVE) {
        want_to_play = true;

        break;
      }
    }
  }

  if (!want_to_play) {
    disconnect_filters();

    return;
  }

  if (list_proxies.empty()) {
    connect_filters();
  }
}

void StreamOutputEffects::connect_filters(const bool& bypass) {
  const auto& list = (bypass) ? std::vector<Glib::ustring>() : settings->get_string_array("plugins");

  uint prev_node_id = pm->ee_sink_node.id;
  uint next_node_id = 0U;

  // link plugins

  if (!list.empty()) {
    for (const auto& name : list) {
      if ((!plugins[name]->connected_to_pw) ? plugins[name]->connect_to_pw() : true) {
        next_node_id = plugins[name]->get_node_id();

        const auto& links = pm->link_nodes(prev_node_id, next_node_id);

        const auto& link_size = links.size();

        for (size_t n = 0U; n < link_size; n++) {
          list_proxies.push_back(links[n]);
        }

        if (link_size == 2U) {
          prev_node_id = next_node_id;
        } else {
          util::warning(log_tag + " link from node " + std::to_string(prev_node_id) + " to node " +
                        std::to_string(next_node_id) + " failed");
        }
      }
    }

    // checking if we have to link the echo_canceller probe to the output device

    for (const auto& name : list) {
      if (name == plugin_name::echo_canceller) {
        if (plugins[name]->connected_to_pw) {
          for (const auto& link : pm->link_nodes(pm->output_device.id, plugins[name]->get_node_id(), true)) {
            list_proxies.push_back(link);
          }
        }

        break;
      }
    }
  }

  // link spectrum and output level meter

  for (const auto& node_id : {spectrum->get_node_id(), output_level->get_node_id()}) {
    next_node_id = node_id;

    const auto& links = pm->link_nodes(prev_node_id, next_node_id);

    const auto& link_size = links.size();

    for (size_t n = 0U; n < link_size; n++) {
      list_proxies.push_back(links[n]);
    }

    if (link_size == 2U) {
      prev_node_id = next_node_id;
    } else {
      util::warning(log_tag + " link from node " + std::to_string(prev_node_id) + " to node " +
                    std::to_string(next_node_id) + " failed");
    }
  }

  // link output device

  next_node_id = pm->output_device.id;

  const auto& links = pm->link_nodes(prev_node_id, next_node_id);

  const auto& link_size = links.size();

  for (size_t n = 0U; n < link_size; n++) {
    list_proxies.push_back(links[n]);
  }

  if (link_size < 2U) {
    util::warning(log_tag + " link from node " + std::to_string(prev_node_id) + " to output device " +
                  std::to_string(next_node_id) + " failed");
  }
}

void StreamOutputEffects::disconnect_filters() {
  std::set<uint> list;

  for (const auto& plugin : plugins | std::views::values) {
    for (const auto& link : pm->list_links) {
      if (link.input_node_id == plugin->get_node_id() || link.output_node_id == plugin->get_node_id()) {
        list.insert(link.id);
      }
    }
  }

  for (const auto& link : pm->list_links) {
    if (link.input_node_id == spectrum->get_node_id() || link.output_node_id == spectrum->get_node_id() ||
        link.input_node_id == output_level->get_node_id() || link.output_node_id == output_level->get_node_id()) {
      list.insert(link.id);
    }
  }

  for (const auto& id : list) {
    pm->destroy_object(static_cast<int>(id));
  }

  pm->destroy_links(list_proxies);

  list_proxies.clear();
}

void StreamOutputEffects::set_bypass(const bool& state) {
  bypass = state;

  disconnect_filters();

  connect_filters(state);
}
