/*
 *  Copyright © 2017-2024 Wellington Wallace
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

#include "stream_input_effects.hpp"
#include <pipewire/link.h>
#include <qcontainerfwd.h>
#include <qnamespace.h>
#include <qqml.h>
#include <qtimer.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <spa/utils/defs.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ranges>
#include <set>
#include <string>
#include <thread>
#include <vector>
#include "config.h"
#include "db_manager.hpp"
#include "effects_base.hpp"
#include "pipeline_type.hpp"
#include "pw_manager.hpp"
#include "pw_objects.hpp"
#include "tags_pipewire.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

StreamInputEffects::StreamInputEffects(pw::Manager* pipe_manager) : EffectsBase(pipe_manager, PipelineType::input) {
  qmlRegisterSingletonInstance<StreamInputEffects>("ee.pipeline", VERSION_MAJOR, VERSION_MINOR, "Input", this);

  auto* PULSE_SOURCE = std::getenv("PULSE_SOURCE");

  if (PULSE_SOURCE != nullptr && PULSE_SOURCE != tags::pipewire::ee_source_name) {
    for (const auto& [serial, node] : pm->node_map) {
      if (node.name == PULSE_SOURCE) {
        pm->input_device = node;

        db::StreamInputs::setInputDevice(pm->input_device.name);

        break;
      }
    }
  }

  connect(pm, &pw::Manager::sourceAdded, [&](pw::NodeInfo node) {
    if (node.name == db::StreamInputs::inputDevice()) {
      pm->input_device = node;

      if (db::Main::bypass()) {
        db::Main::setBypass(false);

        return;  // filter connected through update_bypass_state
      }

      set_bypass(false);
    }
  });

  connect(pm, &pw::Manager::newDefaultSourceName, this, &StreamInputEffects::onNewDefaultSourceName,
          Qt::QueuedConnection);

  connect(db::StreamInputs::self(), &db::StreamInputs::inputDeviceChanged, [&]() {
    const auto name = db::StreamInputs::inputDevice();

    if (name.isEmpty()) {
      return;
    }

    for (const auto& [serial, node] : pm->node_map) {
      if (node.name == name) {
        pm->input_device = node;

        if (db::Main::bypass()) {
          db::Main::setBypass(false);

          return;  // filter connected through update_bypass_state
        }

        set_bypass(false);

        break;
      }
    }
  });

  connect(db::StreamInputs::self(), &db::StreamInputs::pluginsChanged, [&]() {
    if (db::Main::bypass()) {
      db::Main::setBypass(false);

      return;  // filter connected through update_bypass_state
    }

    set_bypass(false);

    Q_EMIT pipelineChanged();
  });

  connect(pm, &pw::Manager::linkChanged, this, &StreamInputEffects::on_link_changed);

  connect_filters();
}

StreamInputEffects::~StreamInputEffects() {
  disconnect_filters();

  util::debug("destroyed");
}

void StreamInputEffects::onNewDefaultSourceName(const QString& name) {
  if (db::StreamInputs::useDefaultInputDevice()) {
    db::StreamInputs::setInputDevice(name);
  }
}

auto StreamInputEffects::apps_want_to_play() -> bool {
  return std::ranges::any_of(pm->list_links, [&](const auto& link) {
    return (link.output_node_id == pm->ee_source_node.id) && (link.state == PW_LINK_STATE_ACTIVE);
  });

  return false;
}

void StreamInputEffects::on_link_changed(const pw::LinkInfo link_info) {
  // We are not interested in the other link states

  if (link_info.state != PW_LINK_STATE_ACTIVE && link_info.state != PW_LINK_STATE_PAUSED) {
    return;
  }

  if (pm->defaultInputDeviceName == pm->ee_source_node.name) {
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
      util::debug("At least one app linked to our device wants to play. Linking our filters.");

      connect_filters();
    };
  } else {
    if (db::Main::inactivityTimerEnable()) {
      // if the timer is enabled, wait for the timeout, then unlink plugin pipeline

      QTimer::singleShot(db::Main::inactivityTimeout() * 1000, this, [&]() {
        if (!apps_want_to_play() && !list_proxies.empty()) {
          util::debug("No app linked to our device wants to play. Unlinking our filters.");

          disconnect_filters();
        }
      });
    } else {
      // otherwise, do nothing
      if (!list_proxies.empty()) {
        util::debug(
            "No app linked to our device wants to play, but the inactivity timer is disabled. Leaving filters linked.");
      }
    }
  }
}

void StreamInputEffects::connect_filters(const bool& bypass) {
  const auto input_device_name = db::StreamInputs::inputDevice();

  // checking if the output device exists

  if (input_device_name.isEmpty()) {
    util::debug("No input device set. Aborting the link");

    return;
  }

  bool dev_exists = false;

  for (const auto& [serial, node] : pm->node_map) {
    if (node.name == input_device_name) {
      dev_exists = true;

      pm->input_device = node;

      break;
    }
  }

  if (!dev_exists) {
    util::debug("The input device " + input_device_name.toStdString() + " is not available. Aborting the link");

    return;
  }

  const auto list = (bypass) ? QStringList() : db::StreamInputs::plugins();

  auto mic_linked = false;

  // waiting for the input device ports information to be available.

  int timeout = 0;

  while (pm->count_node_ports(pm->input_device.id) < 1) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    timeout++;

    if (timeout > 10000) {
      util::warning("Information about the ports of the input device " + pm->input_device.name.toStdString() +
                    " with id " + util::to_string(pm->input_device.id) +
                    " are taking to long to be available. Aborting the link");

      return;
    }
  }

  uint prev_node_id = pm->input_device.id;
  uint next_node_id = 0U;

  // link plugins

  if (!list.empty()) {
    for (const auto& name : list) {
      if (!plugins.contains(name) || plugins[name] == nullptr) {
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
          util::warning(" link from node " + util::to_string(prev_node_id) + " to node " +
                        util::to_string(next_node_id) + " failed");
        }
      }
    }

    // checking if we have to link the echo_canceller probe to the output device

    for (const auto& name : list) {
      if (!plugins.contains(name) || plugins[name] == nullptr) {
        continue;
      }

      if (name.startsWith(tags::plugin_name::BaseName::echoCanceller)) {
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
      util::warning(" link from node " + util::to_string(prev_node_id) + " to node " + util::to_string(next_node_id) +
                    " failed");
    }
  }
}

void StreamInputEffects::disconnect_filters() {
  std::set<uint> link_id_list;

  const auto selected_plugins_list = (bypass) ? QStringList() : db::StreamInputs::plugins();

  for (const auto& plugin : plugins | std::views::values) {
    if (plugin == nullptr) {
      continue;
    }

    for (const auto& link : pm->list_links) {
      if (link.input_node_id == plugin->get_node_id() || link.output_node_id == plugin->get_node_id()) {
        link_id_list.insert(link.id);
      }
    }

    if (plugin->connected_to_pw) {
      if (std::ranges::find(selected_plugins_list, plugin->name) == selected_plugins_list.end()) {
        util::debug("disconnecting the " + plugin->name.toStdString() + " filter from PipeWire");

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

  remove_unused_filters();
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