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

#include "stream_output_effects.hpp"
#include <pipewire/link.h>
#include <pipewire/node.h>
#include <qcontainerfwd.h>
#include <qqml.h>
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

StreamOutputEffects::StreamOutputEffects(pw::Manager* pipe_manager) : EffectsBase(pipe_manager, PipelineType::output) {
  qmlRegisterSingletonInstance<StreamOutputEffects>("EEsoe", VERSION_MAJOR, VERSION_MINOR, "EEsoe", this);

  auto* PULSE_SINK = std::getenv("PULSE_SINK");

  if (PULSE_SINK != nullptr && PULSE_SINK != tags::pipewire::ee_sink_name) {
    for (const auto& [serial, node] : pm->node_map) {
      if (node.name == PULSE_SINK) {
        pm->output_device = node;

        db::StreamOutputs::setOutputDevice(pm->output_device.name);

        break;
      }
    }
  }

  connect(pm, &pw::Manager::sinkAdded, [&](pw::NodeInfo node) {
    if (node.name == db::StreamOutputs::outputDevice()) {
      pm->output_device = node;

      if (db::Main::bypass()) {
        db::Main::setBypass(false);

        return;  // filter connected through update_bypass_state
      }

      set_bypass(false);
    }
  });

  connect(pm, &pw::Manager::sinkRemoved, [&](pw::NodeInfo node) {
    if (db::StreamOutputs::useDefaultOutputDevice() && node.name == db::StreamOutputs::outputDevice()) {
      pm->output_device.id = SPA_ID_INVALID;
      pm->output_device.serial = SPA_ID_INVALID;
    }
  });

  connect(pm, &pw::Manager::streamOutputAdded, this, &StreamOutputEffects::on_app_added);

  connect_filters();

  connect(db::StreamOutputs::self(), &db::StreamOutputs::outputDeviceChanged, [&]() {
    const auto name = db::StreamOutputs::outputDevice();

    if (name.isEmpty()) {
      return;
    }

    for (const auto& [serial, node] : pm->node_map) {
      if (node.name == name) {
        pm->output_device = node;

        if (db::Main::bypass()) {
          db::Main::setBypass(false);

          return;  // filter connected through update_bypass_state
        }

        set_bypass(false);

        break;
      }
    }
  });

  connect(db::StreamOutputs::self(), &db::StreamOutputs::pluginsChanged, [&]() {
    if (db::Main::bypass()) {
      db::Main::setBypass(false);

      return;  // filter connected through update_bypass_state
    }

    set_bypass(false);

    Q_EMIT pipelineChanged();
  });
}

StreamOutputEffects::~StreamOutputEffects() {
  disconnect_filters();

  util::debug("destroyed");
}

void StreamOutputEffects::on_app_added(const pw::NodeInfo node_info) {
  const auto blocklist = (bypass) ? QStringList() : db::StreamOutputs::blocklist();

  auto is_blocklisted = std::ranges::find(blocklist, node_info.application_id) != blocklist.end();

  is_blocklisted = is_blocklisted || std::ranges::find(blocklist, node_info.name) != blocklist.end();

  if (db::Main::processAllOutputs() != 0 && !is_blocklisted) {
    pm->connect_stream_output(node_info.id);
  }
}

auto StreamOutputEffects::apps_want_to_play() -> bool {
  return std::ranges::any_of(pm->list_links, [&](const auto& link) {
    return (link.input_node_id == pm->ee_sink_node.id) && (link.state == PW_LINK_STATE_ACTIVE);
  });
}

void StreamOutputEffects::connect_filters(const bool& bypass) {
  const auto output_device_name = db::StreamOutputs::outputDevice();

  // checking if the output device exists

  if (output_device_name.isEmpty()) {
    util::debug("No output device set. Aborting the link");

    return;
  }

  bool dev_exists = false;

  for (const auto& [serial, node] : pm->node_map) {
    if (node.name == output_device_name) {
      dev_exists = true;

      pm->output_device = node;

      break;
    }
  }

  if (!dev_exists) {
    util::debug("The output device " + output_device_name.toStdString() + " is not available. Aborting the link");

    return;
  }

  const auto list = (bypass) ? QStringList() : db::StreamOutputs::plugins();

  uint prev_node_id = pm->ee_sink_node.id;
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

        if (links.size() == 2U) {
          prev_node_id = next_node_id;
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

  // link spectrum and output level meter

  for (const auto& node_id : {spectrum->get_node_id(), output_level->get_node_id()}) {
    next_node_id = node_id;

    const auto links = pm->link_nodes(prev_node_id, next_node_id);

    for (auto* link : links) {
      list_proxies.push_back(link);
    }

    if (links.size() == 2U) {
      prev_node_id = next_node_id;
    } else {
      util::warning(" link from node " + util::to_string(prev_node_id) + " to node " + util::to_string(next_node_id) +
                    " failed");
    }
  }

  // waiting for the output device ports information to be available.

  int timeout = 0;

  while (pm->count_node_ports(pm->output_device.id) < 2) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    timeout++;

    if (timeout > 10000) {  // 10 seconds
      util::warning("Information about the ports of the output device " + pm->output_device.name.toStdString() +
                    " with id " + util::to_string(pm->output_device.id) +
                    " are taking to long to be available. Aborting the link");

      return;
    }
  }

  // link output device

  next_node_id = pm->output_device.id;

  const auto links = pm->link_nodes(prev_node_id, next_node_id);

  for (auto* link : links) {
    list_proxies.push_back(link);
  }

  if (links.size() < 2U) {
    util::warning(" link from node " + util::to_string(prev_node_id) + " to output device " +
                  util::to_string(next_node_id) + " failed");
  }
}

void StreamOutputEffects::disconnect_filters() {
  std::set<uint> link_id_list;

  const auto selected_plugins_list = (bypass) ? QStringList() : db::StreamOutputs::plugins();

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

void StreamOutputEffects::set_bypass(const bool& state) {
  bypass = state;

  disconnect_filters();

  connect_filters(state);
}