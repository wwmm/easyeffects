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

#pragma once

#include <pipewire/core.h>
#include <pipewire/extensions/metadata.h>
#include <pipewire/node.h>
#include <pipewire/proxy.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <spa/pod/pod.h>
#include <spa/utils/dict.h>
#include <spa/utils/hook.h>
#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include "pw_metadata_manager.hpp"
#include "pw_model_nodes.hpp"
#include "pw_objects.hpp"

namespace pw {

class NodeManager : public QObject {
  Q_OBJECT

 public:
  explicit NodeManager(models::Nodes& model_nodes,
                       MetadataManager& metadata_manager,
                       NodeInfo& ee_sink_node,
                       NodeInfo& ee_source_node,
                       std::vector<LinkInfo>& list_links);
  ~NodeManager() override = default;

  NodeManager(const NodeManager&) = delete;
  auto operator=(const NodeManager&) -> NodeManager& = delete;
  NodeManager(NodeManager&&) = delete;
  auto operator=(NodeManager&&) -> NodeManager& = delete;

  auto registerNode(pw_registry* registry, uint32_t id, const char* type, const spa_dict* props) -> bool;

  static auto load_virtual_devices(pw_core* core) -> std::pair<pw_proxy*, pw_proxy*>;

  void setNodeMute(uint64_t serial, bool state);
  void setNodeVolume(uint64_t serial, uint n_vol_ch, float value);

  static void onNodeInfo(void* object, const pw_node_info* info);
  static void onNodeParam(void* data, int seq, uint32_t id, uint32_t index, uint32_t next, const spa_pod* param);

  static void onDestroyNodeProxy(void* data);
  static void onRemovedNodeProxy(void* data);

 Q_SIGNALS:
  /**
   * IMPORTANT
   * Do not pass NodeInfo by reference. Sometimes it dies before we use it
   * and a segmentation fault happens.
   */

  void sourceAdded(NodeInfo node);
  void sinkAdded(NodeInfo node);

 private:
  struct NodeData {
    NodeManager* nm = nullptr;

    pw_proxy* proxy = nullptr;

    spa_hook proxy_listener{};

    spa_hook object_listener{};

    pw::NodeInfo* nd_info{};
  };

  inline static bool exiting = false;

  models::Nodes& model_nodes;

  MetadataManager& metadata_manager;

  NodeInfo &ee_sink_node, &ee_source_node;

  std::vector<LinkInfo>& list_links;

  const struct pw_node_events node_events = {.version = 0, .info = onNodeInfo, .param = onNodeParam};

  const struct pw_proxy_events node_proxy_events = {.version = 0,
                                                    .destroy = pw::NodeManager::onDestroyNodeProxy,
                                                    .bound = nullptr,
                                                    .removed = pw::NodeManager::onRemovedNodeProxy,
                                                    .done = nullptr,
                                                    .error = nullptr,
                                                    .bound_props = nullptr};

  constexpr static auto blocklist_node_name =
      std::to_array({"Easy Effects", "EasyEffects", "easyeffects", "pwvucontrol-peak-detect", "speech-dispatcher",
                     "speech-dispatcher-dummy", "speech-dispatcher-espeak-ng"});

  std::array<std::string, 2U> blocklist_app_id = {"org.PulseAudio.pavucontrol"};

  std::array<std::string, 2U> blocklist_media_role = {"event", "Notification"};

  constexpr static auto blocklist_notification_nodes =
      std::to_array({"libcanberra", "org.gnome.VolumeControl", "GNOME Shell", "Mutter", "gsd-media-keys"});

  auto stream_is_connected(const uint& id, const QString& media_class) -> bool;
};

}  // namespace pw