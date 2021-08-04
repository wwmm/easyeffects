/*
 *  Copyright © 2017-2020 Wellington Wallace
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
    bool found = false;

    for (const auto& node : pm->list_nodes) {
      if (node.name == std::string(settings->get_string("output-device"))) {
        pm->output_device = node;

        found = true;

        break;
      }
    }

    if (!found) {
      settings->set_string("output-device", pm->output_device.name);
    }
  }

  auto* PULSE_SINK = std::getenv("PULSE_SINK");

  if (PULSE_SINK != nullptr) {
    for (const auto& node : pm->list_nodes) {
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

  settings->signal_changed("output-device").connect([&, this](auto key) {
    auto name = std::string(settings->get_string(key));

    if (name.empty()) {
      return;
    }

    for (const auto& node : pm->list_nodes) {
      if (node.name == name) {
        pm->output_device = node;

        disconnect_filters();

        connect_filters();

        break;
      }
    }
  });

  settings->signal_changed("plugins").connect([&, this](auto key) {
    disconnect_filters();

    connect_filters();
  });
}

StreamOutputEffects::~StreamOutputEffects() {
  disconnect_filters();

  util::debug(log_tag + "destroyed");
}

void StreamOutputEffects::on_app_added(const NodeInfo& node_info) {
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

void StreamOutputEffects::on_link_changed(const LinkInfo& link_info) {
  /*
    If bypass is enable to do touch the plugin pipeline
  */

  if (bypass) {
    return;
  }

  bool want_to_play = false;

  for (const auto& link : pm->list_links) {
    if (link.input_node_id == pm->pe_sink_node.id) {
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

void StreamOutputEffects::connect_filters() {
  auto list = settings->get_string_array("plugins");

  if (list.empty()) {
    auto links = pm->link_nodes(pm->pe_sink_node.id, spectrum->get_node_id());

    for (const auto& link : links) {
      list_proxies.emplace_back(link);
    }
  } else {
    if (!plugins[list[0]]->connected_to_pw) {
      plugins[list[0]]->connect_to_pw();
    }

    auto links = pm->link_nodes(pm->pe_sink_node.id, plugins[list[0]]->get_node_id());

    for (const auto& link : links) {
      list_proxies.emplace_back(link);
    }

    auto list_size = list.size();

    for (size_t n = 1; n < list_size; n++) {
      if (!plugins[list[n]]->connected_to_pw) {
        plugins[list[n]]->connect_to_pw();
      }

      auto links = pm->link_nodes(plugins[list[n - 1]]->get_node_id(), plugins[list[n]]->get_node_id());

      for (const auto& link : links) {
        list_proxies.emplace_back(link);
      }
    }

    links = pm->link_nodes(plugins[list[list_size - 1]]->get_node_id(), spectrum->get_node_id());

    for (const auto& link : links) {
      list_proxies.emplace_back(link);
    }

    // checking if we have to link the echo_canceller probe to the output device

    for (auto& name : list) {
      if (name == plugin_name::echo_canceller) {
        auto links = pm->link_nodes(pm->output_device.id, plugins[name]->get_node_id(), true);

        for (const auto& link : links) {
          list_proxies.emplace_back(link);
        }

        break;
      }
    }
  }

  auto links = pm->link_nodes(spectrum->get_node_id(), output_level->get_node_id());

  for (const auto& link : links) {
    list_proxies.emplace_back(link);
  }

  links = pm->link_nodes(output_level->get_node_id(), pm->output_device.id);

  for (const auto& link : links) {
    list_proxies.emplace_back(link);
  }
}

void StreamOutputEffects::disconnect_filters() {
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
    pm->destroy_object(static_cast<int>(id));
  }

  pm->destroy_links(list_proxies);

  list_proxies.clear();
}

void StreamOutputEffects::set_bypass(const bool& state) {
  bypass = state;

  if (state) {
    disconnect_filters();

    pm->link_nodes(pm->pe_sink_node.id, spectrum->get_node_id());
    pm->link_nodes(spectrum->get_node_id(), output_level->get_node_id());
    pm->link_nodes(output_level->get_node_id(), pm->output_device.id);
  } else {
    disconnect_filters();

    connect_filters();
  }
}
