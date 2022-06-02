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

#include "stream_input_effects.hpp"

StreamInputEffects::StreamInputEffects(PipeManager* pipe_manager)
    : EffectsBase("sie: ", tags::app::id + ".streaminputs", pipe_manager) {
  pm->input_device = pm->default_input_device;

  if (g_settings_get_boolean(settings, "use-default-input-device") != 0) {
    g_settings_set_string(settings, "input-device", pm->input_device.name.c_str());
  } else {
    auto found = false;

    const auto input_device = util::gsettings_get_string(settings, "input-device");

    if (input_device != pm->ee_source_name) {
      for (const auto& [serial, node] : pm->node_map) {
        if (node.name == input_device) {
          pm->input_device = node;

          found = true;

          break;
        }
      }
    }

    if (!found) {
      g_settings_set_string(settings, "input-device", pm->input_device.name.c_str());
    }
  }

  auto* PULSE_SOURCE = std::getenv("PULSE_SOURCE");

  if (PULSE_SOURCE != nullptr && PULSE_SOURCE != pm->ee_source_name) {
    for (const auto& [serial, node] : pm->node_map) {
      if (node.name == PULSE_SOURCE) {
        pm->input_device = node;

        g_settings_set_string(settings, "input-device", pm->input_device.name.c_str());

        break;
      }
    }
  }

  connections.push_back(pm->source_added.connect([=, this](const NodeInfo node) {
    if (node.name == util::gsettings_get_string(settings, "input-device")) {
      pm->input_device = node;

      if (g_settings_get_boolean(global_settings, "bypass") != 0) {
        g_settings_set_boolean(global_settings, "bypass", 0);

        return;  // filter connected through update_bypass_state
      }

      set_bypass(false);
    }
  }));

  connections.push_back(pm->source_removed.connect([=, this](const NodeInfo node) {
    if (g_settings_get_boolean(settings, "use-default-input-device") == 0) {
      if (node.name == util::gsettings_get_string(settings, "input-device")) {
        pm->input_device.id = SPA_ID_INVALID;
        pm->input_device.serial = SPA_ID_INVALID;
      }
    }
  }));

  connections.push_back(pm->stream_input_added.connect(sigc::mem_fun(*this, &StreamInputEffects::on_app_added)));
  connections.push_back(pm->link_changed.connect(sigc::mem_fun(*this, &StreamInputEffects::on_link_changed)));

  connect_filters();

  gconnections.push_back(g_signal_connect(settings, "changed::input-device",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<StreamInputEffects*>(user_data);

                                            const auto name = util::gsettings_get_string(settings, key);

                                            if (name.empty()) {
                                              return;
                                            }

                                            for (const auto& [serial, node] : self->pm->node_map) {
                                              if (node.name == name) {
                                                self->pm->input_device = node;

                                                if (g_settings_get_boolean(self->global_settings, "bypass") != 0) {
                                                  g_settings_set_boolean(self->global_settings, "bypass", 0);

                                                  return;  // filter connected through update_bypass_state
                                                }

                                                self->set_bypass(false);

                                                break;
                                              }
                                            }
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::plugins",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<StreamInputEffects*>(user_data);

                                            if (g_settings_get_boolean(self->global_settings, "bypass") != 0) {
                                              g_settings_set_boolean(self->global_settings, "bypass", 0);

                                              return;  // filter connected through update_bypass_state
                                            }

                                            self->set_bypass(false);
                                          }),
                                          this));
}

StreamInputEffects::~StreamInputEffects() {
  disconnect_filters();

  util::debug(log_tag + "destroyed");
}

void StreamInputEffects::on_app_added(const NodeInfo node_info) {
  const auto blocklist = util::gchar_array_to_vector(g_settings_get_strv(settings, "blocklist"));

  auto is_blocklisted = std::ranges::find(blocklist, node_info.application_id) != blocklist.end();

  is_blocklisted = is_blocklisted || std::ranges::find(blocklist, node_info.name) != blocklist.end();

  if (g_settings_get_boolean(global_settings, "process-all-inputs") != 0 && !is_blocklisted) {
    pm->connect_stream_input(node_info.id);
  }
}

auto StreamInputEffects::apps_want_to_play() -> bool {
  for (const auto& link : pm->list_links) {
    if (link.output_node_id == pm->ee_source_node.id) {
      if (link.state == PW_LINK_STATE_ACTIVE) {
        return true;
      }
    }
  }

  return false;
}

void StreamInputEffects::on_link_changed(const LinkInfo link_info) {
  // We are not interested in the other link states

  if (link_info.state != PW_LINK_STATE_ACTIVE && link_info.state != PW_LINK_STATE_PAUSED) {
    return;
  }

  if (pm->default_input_device.serial == pm->ee_source_node.serial) {
    return;
  }

  /*
    If bypass is enabled do not touch the plugin pipeline
  */

  if (bypass) {
    return;
  }

  if (apps_want_to_play()) {
    if (list_proxies.empty()) {
      util::debug(log_tag + "At least one app linked to our device wants to play. Linking our filters.");

      connect_filters();
    }
  } else {
    int inactivity_timeout = g_settings_get_int(global_settings, "inactivity-timeout");

    g_timeout_add_seconds(
        inactivity_timeout, GSourceFunc(+[](StreamInputEffects* self) {
          if (!self->apps_want_to_play() && !self->list_proxies.empty()) {
            util::debug(self->log_tag + "No app linked to our device wants to play. Unlinking our filters.");

            self->disconnect_filters();
          }

          return G_SOURCE_REMOVE;
        }),
        this);
  }
}

void StreamInputEffects::connect_filters(const bool& bypass) {
  if (pm->input_device.id == SPA_ID_INVALID || pm->input_device.serial == SPA_ID_INVALID) {
    util::debug(log_tag + "Input device id is invalid. Aborting the link between filters in the microphone pipeline");

    return;
  }

  const auto list =
      (bypass) ? std::vector<std::string>() : util::gchar_array_to_vector(g_settings_get_strv(settings, "plugins"));

  auto mic_linked = false;

  // checking if the output device exists

  bool dev_exists = false;

  for (const auto& [serial, node] : pm->node_map) {
    if (node.serial == pm->input_device.serial) {
      dev_exists = true;

      break;
    }
  }

  if (!dev_exists) {
    util::warning(log_tag + "The input device " + pm->input_device.name + " with id " +
                  util::to_string(pm->input_device.id) + " does not exist anymore. Aborting the link");

    return;
  }

  // waiting for the input device ports information to be available.

  int timeout = 0;

  while (pm->count_node_ports(pm->input_device.id) < 1) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    timeout++;

    if (timeout > 10000) {
      util::warning(log_tag + "Information about the ports of the input device " + pm->input_device.name + " with id " +
                    util::to_string(pm->input_device.id) + " are taking to long to be available. Aborting the link");

      return;
    }
  }

  uint prev_node_id = pm->input_device.id;
  uint next_node_id = 0U;

  // link plugins

  if (!list.empty()) {
    for (const auto& name : list) {
      if (!plugins.contains(name)) {
        continue;
      }

      if (!plugins[name]->connected_to_pw ? plugins[name]->connect_to_pw() : true) {
        next_node_id = plugins[name]->get_node_id();

        const auto links = pm->link_nodes(prev_node_id, next_node_id);

        for (auto* link : links) {
          list_proxies.push_back(link);
        }

        if (mic_linked && (links.size() == 2U)) {
          prev_node_id = next_node_id;
        } else if (!mic_linked && (!links.empty())) {
          prev_node_id = next_node_id;
          mic_linked = true;
        } else {
          util::warning(log_tag + " link from node " + util::to_string(prev_node_id) + " to node " +
                        util::to_string(next_node_id) + " failed");
        }
      }
    }

    // checking if we have to link the echo_canceller probe to the output device

    for (const auto& name : list) {
      if (!plugins.contains(name)) {
        continue;
      }

      if (name == plugin_name::echo_canceller) {
        if (plugins[name]->connected_to_pw) {
          for (const auto& link : pm->link_nodes(pm->output_device.id, plugins[name]->get_node_id(), true)) {
            list_proxies.push_back(link);
          }
        }
      }

      plugins[name]->update_probe_links();
    }
  }

  // link spectrum, output level meter and source node

  for (const auto node_id : {spectrum->get_node_id(), output_level->get_node_id(), pm->ee_source_node.id}) {
    next_node_id = node_id;

    const auto links = pm->link_nodes(prev_node_id, next_node_id);

    for (auto* link : links) {
      list_proxies.push_back(link);
    }

    if (mic_linked && (links.size() == 2U)) {
      prev_node_id = next_node_id;
    } else if (!mic_linked && (!links.empty())) {
      prev_node_id = next_node_id;
      mic_linked = true;
    } else {
      util::warning(log_tag + " link from node " + util::to_string(prev_node_id) + " to node " +
                    util::to_string(next_node_id) + " failed");
    }
  }
}

void StreamInputEffects::disconnect_filters() {
  std::set<uint> link_id_list;

  const auto selected_plugins_list =
      (bypass) ? std::vector<std::string>() : util::gchar_array_to_vector(g_settings_get_strv(settings, "plugins"));

  for (const auto& plugin : plugins | std::views::values) {
    for (const auto& link : pm->list_links) {
      if (link.input_node_id == plugin->get_node_id() || link.output_node_id == plugin->get_node_id()) {
        link_id_list.insert(link.id);
      }
    }

    if (plugin->connected_to_pw) {
      if (std::ranges::find(selected_plugins_list, plugin->name) == selected_plugins_list.end()) {
        util::debug(log_tag + "disconnecting the " + plugin->name + " filter from PipeWire");

        plugin->disconnect_from_pw();
      }
    }
  }

  for (const auto& link : pm->list_links) {
    if (link.input_node_id == spectrum->get_node_id() || link.output_node_id == spectrum->get_node_id() ||
        link.input_node_id == output_level->get_node_id() || link.output_node_id == output_level->get_node_id()) {
      link_id_list.insert(link.id);
    }
  }

  for (const auto& id : link_id_list) {
    pm->destroy_object(static_cast<int>(id));
  }

  pm->destroy_links(list_proxies);

  list_proxies.clear();
}

void StreamInputEffects::set_bypass(const bool& state) {
  bypass = state;

  disconnect_filters();

  connect_filters(state);
}

void StreamInputEffects::set_listen_to_mic(const bool& state) {
  if (state) {
    for (const auto& link : pm->link_nodes(pm->ee_source_node.id, pm->output_device.id, false, false)) {
      list_proxies_listen_mic.push_back(link);
    }
  } else {
    pm->destroy_links(list_proxies_listen_mic);

    list_proxies_listen_mic.clear();
  }
}
