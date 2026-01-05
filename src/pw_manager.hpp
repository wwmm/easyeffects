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

#pragma once

#include <pipewire/context.h>
#include <pipewire/core.h>
#include <pipewire/extensions/metadata.h>
#include <pipewire/proxy.h>
#include <pipewire/thread-loop.h>
#include <qmap.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <spa/utils/hook.h>
#include <sys/types.h>
#include <cstdint>
#include <vector>
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

namespace pw {

class Manager : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString headerVersion MEMBER headerVersion NOTIFY headerVersionChanged)
  Q_PROPERTY(QString libraryVersion MEMBER libraryVersion NOTIFY libraryVersionChanged)
  Q_PROPERTY(QString runtimeVersion MEMBER runtimeVersion NOTIFY runtimeVersionChanged)
  Q_PROPERTY(QString defaultClockRate MEMBER defaultClockRate NOTIFY defaultClockRateChanged)
  Q_PROPERTY(QString defaultMinQuantum MEMBER defaultMinQuantum NOTIFY defaultMinQuantumChanged)
  Q_PROPERTY(QString defaultMaxQuantum MEMBER defaultMaxQuantum NOTIFY defaultMaxQuantumChanged)
  Q_PROPERTY(QString defaultQuantum MEMBER defaultQuantum NOTIFY defaultQuantumChanged)

  Q_PROPERTY(QString defaultOutputDeviceName MEMBER defaultOutputDeviceName NOTIFY defaultOutputDeviceNameChanged)
  Q_PROPERTY(QString defaultInputDeviceName MEMBER defaultInputDeviceName NOTIFY defaultInputDeviceNameChanged)

 public:
  Manager();
  Manager(const Manager&) = delete;
  auto operator=(const Manager&) -> Manager& = delete;
  Manager(const Manager&&) = delete;
  auto operator=(const Manager&&) -> Manager& = delete;
  ~Manager() override;

  static Manager& self() {
    static Manager pm;
    return pm;
  }

  pw_thread_loop* thread_loop = nullptr;
  pw_core* core = nullptr;
  pw_registry* registry = nullptr;

  inline static bool exiting = false;

  spa_hook metadata_listener{};

  QString defaultInputDeviceName, defaultOutputDeviceName;

  NodeInfo ee_sink_node, ee_source_node;

  QString headerVersion;
  QString libraryVersion;
  QString runtimeVersion;
  QString defaultClockRate;
  QString defaultMinQuantum;
  QString defaultMaxQuantum;
  QString defaultQuantum;

  pw::models::Nodes model_nodes;
  pw::models::Modules model_modules;
  pw::models::Clients model_clients;

  MetadataManager metadata_manager;
  NodeManager node_manager;
  LinkManager link_manager;
  ModuleManager module_manager;
  ClientManager client_manager;
  DeviceManager device_manager;

  [[nodiscard]] auto count_node_ports(const uint& node_id) const -> uint;

  // Links the output ports of the node output_node_id to the input ports of
  // the node input_node_id

  auto link_nodes(const uint& output_node_id,
                  const uint& input_node_id,
                  const bool& probe_link = false,
                  const bool& link_passive = true) -> std::vector<pw_proxy*>;

  void destroy_object(const int& id) const;

  // Destroy all the filters links

  void destroy_links(const std::vector<pw_proxy*>& list) const;

  void lock() const;

  void unlock() const;

  void sync_wait_unlock() const;

  [[nodiscard]] auto wait_full() const -> int;

  [[nodiscard]] auto get_links() const -> const std::vector<LinkInfo>&;

  Q_INVOKABLE void setNodeMute(const uint& serial, const bool& state);
  Q_INVOKABLE void setNodeVolume(const uint& serial, const uint& n_vol_ch, const float& value);
  Q_INVOKABLE void connectStreamOutput(const uint& id) const;
  Q_INVOKABLE void connectStreamInput(const uint& id) const;
  Q_INVOKABLE void disconnectStream(const uint& id) const;

 Q_SIGNALS:

  void headerVersionChanged();
  void libraryVersionChanged();
  void runtimeVersionChanged();
  void defaultClockRateChanged();
  void defaultMinQuantumChanged();
  void defaultMaxQuantumChanged();
  void defaultQuantumChanged();

  void defaultInputDeviceNameChanged();
  void defaultOutputDeviceNameChanged();

  /**
   * IMPORTANT
   * Do not pass NodeInfo by reference. Sometimes it dies before we use it
   * and a segmentation fault happens.
   */

  void sourceAdded(NodeInfo node);
  void sinkAdded(NodeInfo node);

  void sinkRouteChanged(NodeInfo node);
  void sourceRouteChanged(NodeInfo node);

  void linkChanged(LinkInfo link);

 private:
  pw_context* context = nullptr;
  pw_proxy *proxy_stream_output_sink = nullptr, *proxy_stream_input_source = nullptr;

  spa_hook core_listener{}, registry_listener{};

  std::vector<LinkInfo> list_links;
  std::vector<DeviceInfo> list_devices;

  void register_models();
  void set_metadata_target_node(const uint& origin_id, const uint& target_id, const uint64_t& target_serial) const;
};

}  // namespace pw
