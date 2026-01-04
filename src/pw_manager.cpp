/**
 * Copyright © 2017-2026 Wellington Wallace
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
#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <format>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <thread>
#include <vector>
#include "config.h"
#include "db_manager.hpp"
#include "pw_client_manager.hpp"
#include "pw_device_manager.hpp"
#include "pw_link_manager.hpp"
#include "pw_metadata_manager.hpp"
#include "pw_model_clients.hpp"
#include "pw_model_modules.hpp"
#include "pw_model_nodes.hpp"
#include "pw_module_manager.hpp"
#include "pw_node_manager.hpp"
#include "pw_objects.hpp"
#include "tags_pipewire.hpp"
#include "util.hpp"

namespace {

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
    pm->link_manager.register_link(pm->registry, id, type, props);

    return;
  }

  if (std::strcmp(type, PW_TYPE_INTERFACE_Port) == 0) {
    pm->link_manager.register_port(pm->registry, id, type, props);

    return;
  }

  if (std::strcmp(type, PW_TYPE_INTERFACE_Module) == 0) {
    pm->module_manager.register_module(pm->registry, id, type, props);

    return;
  }

  if (std::strcmp(type, PW_TYPE_INTERFACE_Client) == 0) {
    pm->client_manager.register_client(pm->registry, id, type, props);

    return;
  }

  if (std::strcmp(type, PW_TYPE_INTERFACE_Metadata) == 0) {
    pm->metadata_manager.register_metadata(pm->registry, id, type, props);

    return;
  }

  if (std::strcmp(type, PW_TYPE_INTERFACE_Device) == 0) {
    pm->device_manager.register_device(pm->registry, id, type, props);

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

  util::debug(std::format("Core version: {}", info->version));
  util::debug(std::format("Core name: {}", info->name));
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
      node_manager(NodeManager(model_nodes, metadata_manager, ee_sink_node, ee_source_node, list_links)),
      link_manager(LinkManager(core, thread_loop, model_nodes, list_links)),
      module_manager(ModuleManager(core, thread_loop, model_modules)),
      client_manager(ClientManager(core, thread_loop, model_clients)),
      device_manager(DeviceManager(list_devices)) {
  register_models();

  connect(&metadata_manager, &MetadataManager::defaultSourceChanged, [&](const QString& name) {
    defaultInputDeviceName = name;

    if (DbStreamInputs::useDefaultInputDevice() || DbStreamInputs::inputDevice().isEmpty()) {
      DbStreamInputs::setInputDevice(name);
    }

    Q_EMIT defaultInputDeviceNameChanged();
  });

  connect(&metadata_manager, &MetadataManager::defaultSinkChanged, [&](const QString& name) {
    defaultOutputDeviceName = name;

    if (DbStreamOutputs::useDefaultOutputDevice() || DbStreamOutputs::outputDevice().isEmpty()) {
      DbStreamOutputs::setOutputDevice(name);
    }

    Q_EMIT defaultOutputDeviceNameChanged();
  });

  connect(&node_manager, &NodeManager::sourceAdded, [&](NodeInfo node) { Q_EMIT sourceAdded(node); });

  connect(&node_manager, &NodeManager::sinkAdded, [&](NodeInfo node) { Q_EMIT sinkAdded(node); });

  connect(&link_manager, &LinkManager::linkChanged, [&](LinkInfo link) { Q_EMIT linkChanged(link); });

  connect(&device_manager, &DeviceManager::inputRouteChanged, [&](DeviceInfo device) {
    QTimer::singleShot(DbMain::presetsAutoloadInterval(), this, [&, device]() {
      auto nodes = model_nodes.get_nodes_by_device_id(device.id);

      if (nodes.empty()) {
        util::warning(std::format("Could not find a node related to {}", device.name.toStdString()));
        return;
      }

      for (auto& node : nodes) {
        if (node.media_class == tags::pipewire::media_class::source) {
          node.device_route_name = device.input_route_name;
          node.device_route_description = device.input_route_description;

          model_nodes.update_info(node);

          Q_EMIT sourceRouteChanged(node);
        }
      }
    });
  });

  connect(&device_manager, &DeviceManager::outputRouteChanged, [&](DeviceInfo device) {
    QTimer::singleShot(DbMain::presetsAutoloadInterval(), this, [&, device]() {
      auto nodes = model_nodes.get_nodes_by_device_id(device.id);

      if (nodes.empty()) {
        util::warning(std::format("Could not find a node related to {}", device.name.toStdString()));
        return;
      }

      for (auto& node : nodes) {
        if (node.media_class == tags::pipewire::media_class::sink) {
          node.device_route_name = device.output_route_name;
          node.device_route_description = device.output_route_description;

          model_nodes.update_info(node);

          Q_EMIT sinkRouteChanged(node);
        }
      }
    });
  });

  pw_init(nullptr, nullptr);

  spa_zero(core_listener);
  spa_zero(registry_listener);

  util::debug(std::format("Compiled with PipeWire: {}", headerVersion.toStdString()));
  util::debug(std::format("Linked to PipeWire: {}", libraryVersion.toStdString()));

  // this needs to occur after pw_init(), so putting it before pw_init() in the initializer breaks this
  // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
  thread_loop = pw_thread_loop_new("ee-pipewire-thread", nullptr);

  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  if (thread_loop == nullptr) {
    util::fatal("Could not create PipeWire loop");
  }

  if (pw_thread_loop_start(thread_loop) != 0) {
    util::fatal("Could not start the loop");
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
    util::fatal("Could not create PipeWire context");
  }

  core = pw_context_connect(context, nullptr, 0);

  if (core == nullptr) {
    util::fatal("Context connection failed");
  }

  registry = pw_core_get_registry(core, PW_VERSION_REGISTRY, 0);

  if (registry == nullptr) {
    util::fatal("Could not get the registry");
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
    util::debug(std::format("{} node successfully retrieved with id {} and serial {}", tags::pipewire::ee_sink_name,
                            ee_sink_node.id, ee_sink_node.serial));
  }

  if (ee_source_node.id != SPA_ID_INVALID) {
    util::debug(std::format("{} node successfully retrieved with id {} and serial {}", tags::pipewire::ee_source_name,
                            ee_source_node.id, ee_source_node.serial));
  }

  /**
   * By the time our virtual devices are loaded we may have already received
   * some streams. So we connected them here now that our virtual devices are
   * available.
   */

  for (const auto& node : model_nodes.get_list()) {
    if (node.media_class == tags::pipewire::media_class::output_stream) {
      if (DbMain::processAllOutputs() && !node.is_blocklisted) {
        connectStreamOutput(node.id);
      }
    } else if (node.media_class == tags::pipewire::media_class::input_stream) {
      if (DbMain::processAllInputs() && !node.is_blocklisted) {
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

  metadata_manager.destroy_metadata();

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
  lock();

  // target.node for backward compatibility with old PW session managers
  metadata_manager.set_property(origin_id, "target.node", "Spa:Id", util::to_string(target_id).c_str());
  metadata_manager.set_property(origin_id, "target.object", "Spa:Id", util::to_string(target_serial).c_str());

  sync_wait_unlock();
}

void Manager::disconnectStream(const uint& stream_id) const {
  lock();

  // target.node for backward compatibility with old PW session managers
  metadata_manager.set_property(stream_id, "target.node", nullptr, nullptr);
  metadata_manager.set_property(stream_id, "target.object", nullptr, nullptr);

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

auto Manager::count_node_ports(const uint& node_id) const -> uint {
  return link_manager.count_node_ports(node_id);
}

auto Manager::link_nodes(const uint& output_node_id,
                         const uint& input_node_id,
                         const bool& probe_link,
                         const bool& link_passive) -> std::vector<pw_proxy*> {
  return link_manager.link_nodes(output_node_id, input_node_id, probe_link, link_passive);
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

auto Manager::get_links() const -> const std::vector<LinkInfo>& {
  return list_links;
}

}  // namespace pw
