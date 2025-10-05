/**
 * Copyright © 2017-2025 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "pw_manager.hpp"
#include <pipewire/client.h>
#include <pipewire/context.h>
#include <pipewire/core.h>
#include <pipewire/device.h>
#include <pipewire/extensions/metadata.h>
#include <pipewire/keys.h>
#include <pipewire/link.h>
#include <pipewire/module.h>
#include <pipewire/node.h>
#include <pipewire/pipewire.h>
#include <pipewire/port.h>
#include <pipewire/properties.h>
#include <pipewire/proxy.h>
#include <pipewire/thread-loop.h>
#include <pipewire/version.h>
#include <qmap.h>
#include <qobjectdefs.h>
#include <qqml.h>
#include <qtmetamacros.h>
#include <qvariant.h>
#include <spa/monitor/device.h>
#include <spa/param/audio/raw-types.h>
#include <spa/param/audio/raw.h>
#include <spa/param/format.h>
#include <spa/param/param.h>
#include <spa/param/props.h>
#include <spa/param/route.h>
#include <spa/pod/builder.h>  // NOLINT
#include <spa/pod/iter.h>
#include <spa/pod/parser.h>
#include <spa/pod/pod.h>
#include <spa/pod/vararg.h>
#include <spa/utils/defs.h>
#include <spa/utils/dict.h>
#include <spa/utils/hook.h>
#include <spa/utils/keys.h>
#include <spa/utils/result.h>
#include <spa/utils/type.h>
#include <sys/types.h>
#include <QString>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <format>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include "config.h"
#include "db_manager.hpp"
#include "pw_model_clients.hpp"
#include "pw_model_modules.hpp"
#include "pw_model_nodes.hpp"
#include "pw_node_manager.hpp"
#include "pw_objects.hpp"
#include "tags_pipewire.hpp"
#include "util.hpp"

namespace {

struct proxy_data {
  pw_proxy* proxy = nullptr;

  spa_hook proxy_listener{};

  spa_hook object_listener{};

  pw::Manager* pm = nullptr;

  uint id = SPA_ID_INVALID;

  uint64_t serial = SPA_ID_INVALID;
};

void on_removed_proxy(void* data) {
  auto* const pd = static_cast<proxy_data*>(data);

  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  if (pd->object_listener.link.next != nullptr || pd->object_listener.link.prev != nullptr) {
    spa_hook_remove(&pd->object_listener);
  }

  if (pd->proxy != nullptr) {
    pw_proxy_destroy(pd->proxy);
  }
}

auto link_info_from_props(const spa_dict* props) -> pw::LinkInfo {
  pw::LinkInfo info;

  util::spa_dict_get_num(props, PW_KEY_LINK_ID, info.id);

  util::spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, info.serial);

  util::spa_dict_get_string(props, PW_KEY_OBJECT_PATH, info.path);

  util::spa_dict_get_num(props, PW_KEY_LINK_INPUT_NODE, info.input_node_id);

  util::spa_dict_get_num(props, PW_KEY_LINK_INPUT_PORT, info.input_port_id);

  util::spa_dict_get_num(props, PW_KEY_LINK_OUTPUT_NODE, info.output_node_id);

  util::spa_dict_get_num(props, PW_KEY_LINK_OUTPUT_PORT, info.output_port_id);

  util::spa_dict_get_bool(props, PW_KEY_LINK_PASSIVE, info.passive);

  return info;
}

auto port_info_from_props(const spa_dict* props) -> pw::PortInfo {
  pw::PortInfo info;

  util::spa_dict_get_num(props, PW_KEY_PORT_ID, info.port_id);

  util::spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, info.serial);

  util::spa_dict_get_string(props, PW_KEY_PORT_NAME, info.name);

  util::spa_dict_get_num(props, PW_KEY_NODE_ID, info.node_id);

  util::spa_dict_get_string(props, PW_KEY_PORT_DIRECTION, info.direction);

  util::spa_dict_get_string(props, PW_KEY_AUDIO_CHANNEL, info.audio_channel);

  util::spa_dict_get_string(props, PW_KEY_AUDIO_FORMAT, info.format_dsp);

  util::spa_dict_get_bool(props, PW_KEY_PORT_PHYSICAL, info.physical);

  util::spa_dict_get_bool(props, PW_KEY_PORT_TERMINAL, info.terminal);

  util::spa_dict_get_bool(props, PW_KEY_PORT_MONITOR, info.monitor);

  return info;
}

void on_link_info(void* object, const struct pw_link_info* info) {
  auto* const ld = static_cast<proxy_data*>(object);
  auto* const pm = ld->pm;

  pw::LinkInfo link_copy;

  for (auto& l : ld->pm->list_links) {
    if (l.serial == ld->serial) {
      l.state = info->state;

      link_copy = l;

      Q_EMIT pm->linkChanged(link_copy);

      // util::warning(pw_link_state_as_string(l.state));

      break;
    }
  }

  // const struct spa_dict_item* item = nullptr;
  // spa_dict_for_each(item, info->props) printf("\t\t%s: \"%s\"\n", item->key, item->value);
}

void on_destroy_link_proxy(void* data) {
  auto* const ld = static_cast<proxy_data*>(data);

  spa_hook_remove(&ld->proxy_listener);

  auto it = std::ranges::remove_if(ld->pm->list_links, [=](const auto& n) { return n.serial == ld->serial; });

  ld->pm->list_links.erase(it.begin(), it.end());
}

void on_destroy_port_proxy(void* data) {
  auto* const pd = static_cast<proxy_data*>(data);

  spa_hook_remove(&pd->proxy_listener);

  auto it = std::ranges::remove_if(pd->pm->list_ports, [=](const auto& n) { return n.serial == pd->serial; });

  pd->pm->list_ports.erase(it.begin(), it.end());
}

void on_module_info(void* object, const struct pw_module_info* info) {
  auto* const md = static_cast<proxy_data*>(object);

  auto list = md->pm->model_modules.get_list();

  for (int n = 0; n < list.size(); n++) {
    if (list[n].id == info->id) {
      if (info->props != nullptr) {
        QString description;

        util::spa_dict_get_string(info->props, PW_KEY_MODULE_DESCRIPTION, description);

        md->pm->model_modules.update_field(n, pw::models::Modules::Roles::Description, description);
      }

      if (info->filename != nullptr) {
        md->pm->model_modules.update_field(n, pw::models::Modules::Roles::Filename, info->filename);
      }

      break;
    }
  }
}

void on_destroy_module_proxy(void* data) {
  auto* const md = static_cast<proxy_data*>(data);

  spa_hook_remove(&md->proxy_listener);

  md->pm->model_modules.remove_by_id(md->id);
}

void on_client_info(void* object, const struct pw_client_info* info) {
  auto* const cd = static_cast<proxy_data*>(object);

  auto list = cd->pm->model_clients.get_list();

  for (int n = 0; n < list.size(); n++) {
    if (list[n].id == info->id && info->props != nullptr) {
      QString name;
      QString access;
      QString api;

      util::spa_dict_get_string(info->props, PW_KEY_APP_NAME, name);

      util::spa_dict_get_string(info->props, PW_KEY_ACCESS, access);

      util::spa_dict_get_string(info->props, PW_KEY_CLIENT_API, api);

      cd->pm->model_clients.update_field(n, pw::models::Clients::Roles::Name, name);
      cd->pm->model_clients.update_field(n, pw::models::Clients::Roles::Access, access);
      cd->pm->model_clients.update_field(n, pw::models::Clients::Roles::Api, api);

      break;
    }
  }
}

void on_destroy_client_proxy(void* data) {
  auto* const cd = static_cast<proxy_data*>(data);

  spa_hook_remove(&cd->proxy_listener);

  cd->pm->model_clients.remove_by_id(cd->id);
}

auto on_metadata_property(void* data, uint32_t id, const char* key, const char* type, const char* value) -> int {
  auto* const pm = static_cast<pw::Manager*>(data);

  const std::string str_key = (key != nullptr) ? key : "";
  const std::string str_value = (value != nullptr) ? value : "";

  util::debug(std::format("new metadata property: {}, {}, {}, {}", id, str_key, type ? type : "", str_value));

  if (str_value.empty()) {
    return 0;
  }

  if (str_key == "default.audio.sink") {
    auto v = nlohmann::json::parse(str_value).value("name", "");

    if (v == tags::pipewire::ee_sink_name) {
      return 0;
    }

    util::debug(std::format("new default output device: {}", v));

    pm->defaultOutputDeviceName = QString::fromStdString(v);

    Q_EMIT pm->newDefaultSinkName(pm->defaultOutputDeviceName);
  }

  if (str_key == "default.audio.source") {
    auto v = nlohmann::json::parse(str_value).value("name", "");

    if (v == tags::pipewire::ee_source_name) {
      return 0;
    }

    util::debug(std::format("new default input device: {}", v));

    pm->defaultInputDeviceName = QString::fromStdString(v);

    Q_EMIT pm->newDefaultSourceName(pm->defaultInputDeviceName);
  }

  return 0;
}

const struct pw_metadata_events metadata_events = {.version = PW_VERSION_METADATA_EVENTS,
                                                   .property = on_metadata_property};

const struct pw_proxy_events link_proxy_events = {.version = 0,
                                                  .destroy = on_destroy_link_proxy,
                                                  .bound = nullptr,
                                                  .removed = on_removed_proxy,
                                                  .done = nullptr,
                                                  .error = nullptr,
                                                  .bound_props = nullptr};

const struct pw_proxy_events port_proxy_events = {.version = 0,
                                                  .destroy = on_destroy_port_proxy,
                                                  .bound = nullptr,
                                                  .removed = on_removed_proxy,
                                                  .done = nullptr,
                                                  .error = nullptr,
                                                  .bound_props = nullptr};

const struct pw_proxy_events module_proxy_events = {.version = 0,
                                                    .destroy = on_destroy_module_proxy,
                                                    .bound = nullptr,
                                                    .removed = on_removed_proxy,
                                                    .done = nullptr,
                                                    .error = nullptr,
                                                    .bound_props = nullptr};

const struct pw_proxy_events client_proxy_events = {.version = 0,
                                                    .destroy = on_destroy_client_proxy,
                                                    .bound = nullptr,
                                                    .removed = on_removed_proxy,
                                                    .done = nullptr,
                                                    .error = nullptr,
                                                    .bound_props = nullptr};

const struct pw_link_events link_events = {
    .version = 0,
    .info = on_link_info,
};

const struct pw_module_events module_events = {
    .version = 0,
    .info = on_module_info,
};

const struct pw_client_events client_events = {.version = 0, .info = on_client_info, .permissions = nullptr};

void on_registry_global(void* data,
                        uint32_t id,
                        [[maybe_unused]] uint32_t permissions,
                        const char* type,
                        [[maybe_unused]] uint32_t version,
                        const struct spa_dict* props) {
  if (id == SPA_ID_INVALID) {
    // If PipeWire send us a wrong id, we don't have issues
    return;
  }

  auto* const pm = static_cast<pw::Manager*>(data);

  if (std::strcmp(type, PW_TYPE_INTERFACE_Node) == 0) {
    pm->node_manager.registerNode(pm->registry, id, type, props);

    return;
  }

  if (std::strcmp(type, PW_TYPE_INTERFACE_Link) == 0) {
    uint64_t serial = 0U;

    if (!util::spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, serial)) {
      util::warning(
          "An error occurred while retrieving the object serial. This link cannot be handled by Easy Effects.");
      return;
    }

    auto* proxy = static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_LINK, sizeof(proxy_data)));

    auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

    pd->proxy = proxy;
    pd->pm = pm;
    pd->id = id;
    pd->serial = serial;

    pw_proxy_add_object_listener(proxy, &pd->object_listener, &link_events, pd);  // NOLINT
    pw_proxy_add_listener(proxy, &pd->proxy_listener, &link_proxy_events, pd);

    auto link_info = link_info_from_props(props);

    link_info.id = id;
    link_info.serial = serial;

    pm->list_links.push_back(link_info);

    try {
      const auto input_node = pm->model_nodes.get_node_by_id(link_info.input_node_id);

      const auto output_node = pm->model_nodes.get_node_by_id(link_info.output_node_id);

      util::debug(std::format("{} port {} is connected to {} port {}", output_node.name.toStdString(),
                              link_info.output_port_id, input_node.name.toStdString(), link_info.input_port_id));
    } catch (std::out_of_range& e) {
      util::debug(e.what());
    }

    return;
  }

  if (std::strcmp(type, PW_TYPE_INTERFACE_Port) == 0) {
    uint64_t serial = 0U;

    if (!util::spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, serial)) {
      util::warning(
          "An error occurred while retrieving the object serial. This port cannot be handled by Easy Effects.");
      return;
    }

    auto* proxy = static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_PORT, sizeof(proxy_data)));

    auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

    pd->proxy = proxy;
    pd->pm = pm;
    pd->id = id;
    pd->serial = serial;

    pw_proxy_add_listener(proxy, &pd->proxy_listener, &port_proxy_events, pd);

    auto port_info = port_info_from_props(props);

    port_info.id = id;
    port_info.serial = serial;

    // std::cout << port_info.name << "\t" << port_info.audio_channel << "\t" << port_info.direction << "\t"
    //           << port_info.format_dsp << "\t" << port_info.port_id << "\t" << port_info.node_id << std::endl;

    pm->list_ports.push_back(port_info);

    return;
  }

  if (std::strcmp(type, PW_TYPE_INTERFACE_Module) == 0) {
    uint64_t serial = 0U;

    if (!util::spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, serial)) {
      util::warning(
          "An error occurred while retrieving the object serial. This module cannot be handled by Easy Effects.");
      return;
    }

    auto* proxy =
        static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_MODULE, sizeof(proxy_data)));

    auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

    pd->proxy = proxy;
    pd->pm = pm;
    pd->id = id;
    pd->serial = serial;

    pw_proxy_add_object_listener(proxy, &pd->object_listener, &module_events, pd);  // NOLINT
    pw_proxy_add_listener(proxy, &pd->proxy_listener, &module_proxy_events, pd);

    pw::ModuleInfo m_info{.id = id, .serial = serial, .name = "", .description = "", .filename = ""};

    util::spa_dict_get_string(props, PW_KEY_MODULE_NAME, m_info.name);

    pm->model_modules.append(m_info);

    return;
  }

  if (std::strcmp(type, PW_TYPE_INTERFACE_Client) == 0) {
    uint64_t serial = 0U;

    if (!util::spa_dict_get_num(props, PW_KEY_OBJECT_SERIAL, serial)) {
      util::warning(
          "An error occurred while retrieving the object serial. This client cannot be handled by Easy Effects.");
      return;
    }

    auto* proxy =
        static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_CLIENT, sizeof(proxy_data)));

    auto* const pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

    pd->proxy = proxy;
    pd->pm = pm;
    pd->id = id;
    pd->serial = serial;

    pw_proxy_add_object_listener(proxy, &pd->object_listener, &client_events, pd);  // NOLINT
    pw_proxy_add_listener(proxy, &pd->proxy_listener, &client_proxy_events, pd);

    pw::ClientInfo c_info{.id = id, .serial = serial, .name = "", .access = "", .api = ""};

    pm->model_clients.append(c_info);

    return;
  }

  if (std::strcmp(type, PW_TYPE_INTERFACE_Metadata) == 0) {
    if (const auto* name = spa_dict_lookup(props, PW_KEY_METADATA_NAME)) {
      util::debug(std::format("found metadata: {}", name));

      if (std::strcmp(name, "default") == 0) {
        if (pm->metadata != nullptr) {
          util::debug("A new default metadata is available. We will use it");

          spa_hook_remove(&pm->metadata_listener);
        }

        pm->metadata = static_cast<pw_metadata*>(pw_registry_bind(pm->registry, id, type, PW_VERSION_METADATA, 0));

        if (pm->metadata != nullptr) {
          pw_metadata_add_listener(pm->metadata, &pm->metadata_listener, &metadata_events, pm);  // NOLINT
        } else {
          util::warning("pw_registry_bind returned a null metadata object");
        }
      }
    }

    return;
  }
}

void on_core_error(void* data, uint32_t id, [[maybe_unused]] int seq, int res, const char* message) {
  auto* const pm = static_cast<pw::Manager*>(data);

  if (id == PW_ID_CORE) {
    util::warning(std::format("Remote error res: {}", spa_strerror(res)));
    util::warning(std::format("Remote error message: {}", message));

    if (std::string(message) == "connection error") {
      util::fatal("No connection to PipeWire. Aborting!");
    }

    pw_thread_loop_signal(pm->thread_loop, false);
  }
}

void on_core_info(void* data, const struct pw_core_info* info) {
  auto* const pm = static_cast<pw::Manager*>(data);

  pm->runtimeVersion = info->version;

  util::spa_dict_get_string(info->props, "default.clock.rate", pm->defaultClockRate);

  util::spa_dict_get_string(info->props, "default.clock.min-quantum", pm->defaultMinQuantum);

  util::spa_dict_get_string(info->props, "default.clock.max-quantum", pm->defaultMaxQuantum);

  util::spa_dict_get_string(info->props, "default.clock.quantum", pm->defaultQuantum);

  util::warning(std::format("core version: {}", info->version));
  util::warning(std::format("core name: {}", info->name));
}

void on_core_done(void* data, uint32_t id, [[maybe_unused]] int seq) {
  auto* const pm = static_cast<pw::Manager*>(data);

  if (id == PW_ID_CORE) {
    pw_thread_loop_signal(pm->thread_loop, false);
  }
}

const struct pw_core_events core_events = {.version = PW_VERSION_CORE_EVENTS,
                                           .info = on_core_info,
                                           .done = on_core_done,
                                           .ping = nullptr,
                                           .error = on_core_error,
                                           .remove_id = nullptr,
                                           .bound_id = nullptr,
                                           .add_mem = nullptr,
                                           .remove_mem = nullptr,
                                           .bound_props = nullptr};

const struct pw_registry_events registry_events = {.version = 0,
                                                   .global = on_registry_global,
                                                   .global_remove = nullptr};

}  // namespace

namespace pw {

Manager::Manager()
    : headerVersion(pw_get_headers_version()),
      libraryVersion(pw_get_library_version()),
      node_manager(NodeManager(model_nodes, metadata, ee_sink_node, ee_source_node, list_links)) {
  register_models();

  connect(&node_manager, &NodeManager::sourceAdded, [&](NodeInfo node) { Q_EMIT sourceAdded(node); });

  connect(&node_manager, &NodeManager::sinkAdded, [&](NodeInfo node) { Q_EMIT sinkAdded(node); });

  connect(&node_manager, &NodeManager::sourceProfileNameChanged,
          [&](NodeInfo node) { Q_EMIT sourceProfileNameChanged(node); });

  connect(&node_manager, &NodeManager::sinkProfileNameChanged,
          [&](NodeInfo node) { Q_EMIT sinkProfileNameChanged(node); });

  pw_init(nullptr, nullptr);

  spa_zero(core_listener);
  spa_zero(registry_listener);

  util::debug(std::format("compiled with PipeWire: {}", headerVersion.toStdString()));
  util::debug(std::format("linked to PipeWire: {}", libraryVersion.toStdString()));

  // this needs to occur after pw_init(), so putting it before pw_init() in the initializer breaks this
  // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
  thread_loop = pw_thread_loop_new("ee-pipewire-thread", nullptr);

  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  if (thread_loop == nullptr) {
    util::fatal("could not create PipeWire loop");
  }

  if (pw_thread_loop_start(thread_loop) != 0) {
    util::fatal("could not start the loop");
  }

  lock();

  pw_properties* props_context = pw_properties_new(nullptr, nullptr);

  const auto comparison_result = util::compare_versions(libraryVersion.toStdString(), "1.4.0");

  if (comparison_result == -1) {
    pw_properties_set(props_context, PW_KEY_CONFIG_NAME, "client-rt.conf");
  }

  pw_properties_set(props_context, PW_KEY_MEDIA_TYPE, "Audio");
  pw_properties_set(props_context, PW_KEY_MEDIA_CATEGORY, "Manager");
  pw_properties_set(props_context, PW_KEY_MEDIA_ROLE, "Music");

  context = pw_context_new(pw_thread_loop_get_loop(thread_loop), props_context, 0);

  if (context == nullptr) {
    util::fatal("could not create PipeWire context");
  }

  core = pw_context_connect(context, nullptr, 0);

  if (core == nullptr) {
    util::fatal("context connection failed");
  }

  registry = pw_core_get_registry(core, PW_VERSION_REGISTRY, 0);

  if (registry == nullptr) {
    util::fatal("could not get the registry");
  }

  pw_core_add_listener(core, &core_listener, &core_events, this);  // NOLINT

  pw_registry_add_listener(registry, &registry_listener, &registry_events, this);  // NOLINT

  if (ee_sink_node.id == SPA_ID_INVALID || ee_source_node.id == SPA_ID_INVALID) {
    auto r = NodeManager::load_virtual_devices(core);

    proxy_stream_input_source = r.first;
    proxy_stream_output_sink = r.second;
  }

  sync_wait_unlock();

  do {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    ee_sink_node = model_nodes.get_node_by_name(tags::pipewire::ee_sink_name);
    ee_source_node = model_nodes.get_node_by_name(tags::pipewire::ee_source_name);
  } while (ee_sink_node.id == SPA_ID_INVALID || ee_source_node.id == SPA_ID_INVALID);

  if (ee_sink_node.id != SPA_ID_INVALID) {
    util::warning(std::format("{} node successfully retrieved with id {} and serial {}", tags::pipewire::ee_sink_name,
                              ee_sink_node.id, ee_sink_node.serial));
  }

  if (ee_source_node.id != SPA_ID_INVALID) {
    util::warning(std::format("{} node successfully retrieved with id {} and serial {}", tags::pipewire::ee_source_name,
                              ee_source_node.id, ee_source_node.serial));
  }

  /**
   * By the time our virtual devices are loaded we may have already received
   * some streams. So we connected them here now that our virtual devices are
   * available.
   */

  for (const auto& node : model_nodes.get_list()) {
    if (node.media_class == tags::pipewire::media_class::output_stream) {
      if (db::Main::processAllOutputs() && !node.is_blocklisted) {
        connectStreamOutput(node.id);
      }
    } else if (node.media_class == tags::pipewire::media_class::input_stream) {
      if (db::Main::processAllInputs() && !node.is_blocklisted) {
        connectStreamInput(node.id);
      }
    }
  }
}

Manager::~Manager() {
  exiting = true;

  lock();

  spa_hook_remove(&registry_listener);
  spa_hook_remove(&core_listener);
  spa_hook_remove(&metadata_listener);

  if (metadata != nullptr) {
    pw_proxy_destroy((struct pw_proxy*)metadata);
  }

  pw_proxy_destroy(proxy_stream_output_sink);
  pw_proxy_destroy(proxy_stream_input_source);

  util::debug("Destroying PipeWire registry...");
  pw_proxy_destroy((struct pw_proxy*)registry);

  util::debug("Disconnecting PipeWire core...");
  pw_core_disconnect(core);

  unlock();

  util::debug("Stopping PipeWire's loop...");
  pw_thread_loop_stop(thread_loop);

  util::debug("Destroying PipeWire's context...");
  pw_context_destroy(context);

  util::debug("Destroying PipeWire's loop...");
  pw_thread_loop_destroy(thread_loop);
}

void Manager::register_models() {
  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDelete)
  qmlRegisterSingletonInstance<pw::Manager>("ee.pipewire", VERSION_MAJOR, VERSION_MINOR, "Manager", this);

  qmlRegisterSingletonInstance<pw::models::Nodes>("ee.pipewire", VERSION_MAJOR, VERSION_MINOR, "ModelNodes",
                                                  &model_nodes);

  qmlRegisterSingletonInstance<pw::models::Modules>("ee.pipewire", VERSION_MAJOR, VERSION_MINOR, "ModelModules",
                                                    &model_modules);

  qmlRegisterSingletonInstance<pw::models::Clients>("ee.pipewire", VERSION_MAJOR, VERSION_MINOR, "ModelClients",
                                                    &model_clients);
  // NOLINTEND(clang-analyzer-cplusplus.NewDelete)
}

void Manager::connectStreamOutput(const uint& id) const {
  set_metadata_target_node(id, ee_sink_node.id, ee_sink_node.serial);
}

void Manager::connectStreamInput(const uint& id) const {
  set_metadata_target_node(id, ee_source_node.id, ee_source_node.serial);
}

void Manager::set_metadata_target_node(const uint& origin_id,
                                       const uint& target_id,
                                       const uint64_t& target_serial) const {
  if (metadata == nullptr) {
    return;
  }

  lock();

  // target.node for backward compatibility with old PW session managers
  // NOLINTNEXTLINE
  pw_metadata_set_property(metadata, origin_id, "target.node", "Spa:Id", util::to_string(target_id).c_str());
  // NOLINTNEXTLINE
  pw_metadata_set_property(metadata, origin_id, "target.object", "Spa:Id", util::to_string(target_serial).c_str());

  sync_wait_unlock();
}

void Manager::disconnectStream(const uint& stream_id) const {
  if (metadata == nullptr) {
    return;
  }

  lock();

  // target.node for backward compatibility with old PW session managers
  pw_metadata_set_property(metadata, stream_id, "target.node", nullptr, nullptr);    // NOLINT
  pw_metadata_set_property(metadata, stream_id, "target.object", nullptr, nullptr);  // NOLINT

  sync_wait_unlock();
}

void Manager::setNodeVolume(const uint& serial, const uint& n_vol_ch, const float& value) {
  lock();

  node_manager.setNodeVolume(serial, n_vol_ch, value);

  sync_wait_unlock();
}

void Manager::setNodeMute(const uint& serial, const bool& state) {
  lock();

  node_manager.setNodeMute(serial, state);

  sync_wait_unlock();
}

auto Manager::count_node_ports(const uint& node_id) -> uint {
  uint count = 0U;

  for (const auto& port : list_ports) {
    if (port.node_id == node_id) {
      count++;
    }
  }

  return count;
}

auto Manager::link_nodes(const uint& output_node_id,
                         const uint& input_node_id,
                         const bool& probe_link,
                         const bool& link_passive) -> std::vector<pw_proxy*> {
  std::vector<pw_proxy*> list;
  std::vector<PortInfo> list_output_ports;
  std::vector<PortInfo> list_input_ports;
  auto use_audio_channel = true;

  for (const auto& port : list_ports) {
    if (port.node_id == output_node_id && port.direction == "out") {
      list_output_ports.push_back(port);

      if (!probe_link) {
        if (port.audio_channel != "FL" && port.audio_channel != "FR") {
          use_audio_channel = false;
        }
      }
    }

    if (port.node_id == input_node_id && port.direction == "in") {
      if (!probe_link) {
        list_input_ports.push_back(port);

        if (port.audio_channel != "FL" && port.audio_channel != "FR") {
          use_audio_channel = false;
        }
      } else {
        if (port.audio_channel == "PROBE_FL" || port.audio_channel == "PROBE_FR") {
          list_input_ports.push_back(port);
        }
      }
    }
  }

  if (list_input_ports.empty()) {
    util::debug(std::format("node {} has no input ports yet. Aborting the link", input_node_id));

    return list;
  }

  if (list_output_ports.empty()) {
    util::debug(std::format("node {} has no output ports yet. Aborting the link", output_node_id));

    return list;
  }

  for (const auto& outp : list_output_ports) {
    for (const auto& inp : list_input_ports) {
      bool ports_match = false;

      if (!probe_link) {
        if (use_audio_channel) {
          ports_match = outp.audio_channel == inp.audio_channel;
        } else {
          ports_match = outp.port_id == inp.port_id;
        }
      } else {
        if (outp.audio_channel == "FL" && inp.audio_channel == "PROBE_FL") {
          ports_match = true;
        }

        if (outp.audio_channel == "FR" && inp.audio_channel == "PROBE_FR") {
          ports_match = true;
        }
      }

      if (ports_match) {
        pw_properties* props = pw_properties_new(nullptr, nullptr);

        pw_properties_set(props, PW_KEY_LINK_PASSIVE, (link_passive) ? "true" : "false");
        pw_properties_set(props, PW_KEY_OBJECT_LINGER, "false");
        pw_properties_set(props, PW_KEY_LINK_OUTPUT_NODE, util::to_string(output_node_id).c_str());
        pw_properties_set(props, PW_KEY_LINK_OUTPUT_PORT, util::to_string(outp.id).c_str());
        pw_properties_set(props, PW_KEY_LINK_INPUT_NODE, util::to_string(input_node_id).c_str());
        pw_properties_set(props, PW_KEY_LINK_INPUT_PORT, util::to_string(inp.id).c_str());

        lock();

        auto* proxy = static_cast<pw_proxy*>(
            pw_core_create_object(core, "link-factory", PW_TYPE_INTERFACE_Link, PW_VERSION_LINK, &props->dict, 0));

        pw_properties_free(props);

        if (proxy == nullptr) {
          util::warning(std::format("failed to link the node {} to {}", output_node_id, input_node_id));

          unlock();

          return list;
        }

        sync_wait_unlock();

        list.push_back(proxy);
      }
    }
  }

  return list;
}

void Manager::lock() const {
  pw_thread_loop_lock(thread_loop);
}

void Manager::unlock() const {
  pw_thread_loop_unlock(thread_loop);
}

void Manager::sync_wait_unlock() const {
  pw_core_sync(core, PW_ID_CORE, 0);  // NOLINT

  pw_thread_loop_wait(thread_loop);

  pw_thread_loop_unlock(thread_loop);
}

auto Manager::wait_full() const -> int {
  timespec abstime;

  pw_thread_loop_get_time(thread_loop, &abstime, 30 * SPA_NSEC_PER_SEC);

  return pw_thread_loop_timed_wait_full(thread_loop, &abstime);
}

void Manager::destroy_object(const int& id) const {
  lock();

  pw_registry_destroy(registry, id);  // NOLINT

  sync_wait_unlock();
}

void Manager::destroy_links(const std::vector<pw_proxy*>& list) const {
  for (auto* proxy : list) {
    if (proxy != nullptr) {
      lock();

      pw_proxy_destroy(proxy);

      sync_wait_unlock();
    }
  }
}

}  // namespace pw
