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

#include <pipewire/node.h>
#include <pipewire/proxy.h>
#include <qabstractitemmodel.h>
#include <qhash.h>
#include <qhashfunctions.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qsortfilterproxymodel.h>
#include <qstringview.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <qvariant.h>
#include <array>
#include <cstdint>
#include <iterator>
#include <utility>
#include "pw_objects.hpp"

namespace pw::models {

class Nodes : public QAbstractListModel {
  Q_OBJECT

 public:
  explicit Nodes(QObject* parent = nullptr);

  enum class Roles {
    Id = Qt::UserRole,
    Serial,
    DeviceId,
    Name,
    Description,
    MediaClass,
    MediaRole,
    AppName,
    AppProcessId,
    AppProcessBinary,
    AppIconName,
    MediaIconName,
    DeviceIconName,
    MediaName,
    Format,
    ApplicationId,
    Priority,
    State,
    Mute,
    Connected,
    NinputPorts,
    NoutputPorts,
    Rate,
    NvolumeChannels,
    Latency,
    Volume,
    IsBlocklisted,
    DeviceRouteName,
    DeviceRouteDescription
  };
  Q_ENUM(Roles)

  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;

  bool setData(const QModelIndex& index, const QVariant& value, int role) override;

  void reset();

  void begin_reset();

  void end_reset();

  auto get_list() -> QList<NodeInfo>;

  void append(NodeInfo info);

  void remove_by_id(const uint& id);

  void remove_by_serial(const uint& serial);

  auto has_serial(const uint& serial) -> bool;

  void update_info(NodeInfo new_info);

  auto get_row_by_serial(const uint& serial) -> int;

  auto get_proxy_by_serial(const uint& serial) -> pw_proxy*;

  auto get_node_by_name(QString name) -> NodeInfo;

  auto get_node_by_id(const uint& id) -> NodeInfo;

  auto get_nodes_by_device_id(const uint& id) -> QList<NodeInfo>;

  Q_INVOKABLE QString getNodeName(const uint& rowIndex);
  Q_INVOKABLE QString getNodeDescription(QString nodeName);
  Q_INVOKABLE QModelIndex getModelIndexByName(QString nodeName);

  template <typename T>
  void update_field(const int& row, const Roles& role, const T& value) {
    auto model_index = this->index(row);

    auto it = std::next(list.begin(), row);

    switch (role) {
      case Roles::Id: {
        if constexpr (std::is_same_v<T, uint>) {
          it->id = value;
        }

        break;
      }
      case Roles::Serial: {
        if constexpr (std::is_same_v<T, uint64_t>) {
          it->serial = value;
        }

        break;
      }
      case Roles::DeviceId: {
        if constexpr (std::is_same_v<T, uint>) {
          it->device_id = value;
        }

        break;
      }
      case Roles::Name: {
        if constexpr (std::is_same_v<T, QString>) {
          it->name = value;
        }

        break;
      }
      case Roles::Description: {
        if constexpr (std::is_same_v<T, QString>) {
          it->description = value;
        }

        break;
      }
      case Roles::MediaClass: {
        if constexpr (std::is_same_v<T, QString>) {
          it->media_class = value;
        }

        break;
      }
      case Roles::MediaRole: {
        if constexpr (std::is_same_v<T, QString>) {
          it->media_role = value;
        }

        break;
      }
      case Roles::AppName: {
        if constexpr (std::is_same_v<T, QString>) {
          it->app_name = value;
        }

        break;
      }
      case Roles::AppProcessId: {
        if constexpr (std::is_same_v<T, QString>) {
          it->app_process_id = value;
        }

        break;
      }
      case Roles::AppProcessBinary: {
        if constexpr (std::is_same_v<T, QString>) {
          it->app_process_binary = value;
        }

        break;
      }
      case Roles::AppIconName: {
        if constexpr (std::is_same_v<T, QString>) {
          it->app_icon_name = value;
        }

        break;
      }
      case Roles::MediaIconName: {
        if constexpr (std::is_same_v<T, QString>) {
          it->media_icon_name = value;
        }

        break;
      }
      case Roles::DeviceIconName: {
        if constexpr (std::is_same_v<T, QString>) {
          it->device_icon_name = value;
        }

        break;
      }
      case Roles::MediaName: {
        if constexpr (std::is_same_v<T, QString>) {
          it->media_name = value;
        }

        break;
      }
      case Roles::Format: {
        if constexpr (std::is_same_v<T, QString>) {
          it->format = value;
        }

        break;
      }
      case Roles::ApplicationId: {
        if constexpr (std::is_same_v<T, QString>) {
          it->application_id = value;
        }

        break;
      }
      case Roles::Priority: {
        if constexpr (std::is_same_v<T, int>) {
          it->priority = value;
        }

        break;
      }
      case Roles::State: {
        if constexpr (std::is_same_v<T, pw_node_state>) {
          it->state = value;
        }

        break;
      }
      case Roles::Mute: {
        if constexpr (std::is_same_v<T, bool>) {
          it->mute = value;
        }

        break;
      }
      case Roles::Connected: {
        if constexpr (std::is_same_v<T, bool>) {
          it->connected = value;
        }

        break;
      }
      case Roles::NinputPorts: {
        if constexpr (std::is_same_v<T, int>) {
          it->n_input_ports = value;
        }

        break;
      }
      case Roles::NoutputPorts: {
        if constexpr (std::is_same_v<T, int>) {
          it->n_output_ports = value;
        }

        break;
      }
      case Roles::Rate: {
        if constexpr (std::is_same_v<T, int>) {
          it->rate = value;
        }

        break;
      }
      case Roles::NvolumeChannels: {
        if constexpr (std::is_same_v<T, uint>) {
          it->n_volume_channels = value;
        }

        break;
      }
      case Roles::Latency: {
        if constexpr (std::is_same_v<T, float>) {
          it->latency = value;
        }

        break;
      }
      case Roles::Volume: {
        if constexpr (std::is_same_v<T, float>) {
          it->volume = value;
        }

        break;
      }
      case Roles::IsBlocklisted: {
        if constexpr (std::is_same_v<T, bool>) {
          it->is_blocklisted = value;
        }

        break;
      }
      case Roles::DeviceRouteName: {
        if constexpr (std::is_same_v<T, QString>) {
          it->device_route_name = value;
        }

        break;
      }
      case Roles::DeviceRouteDescription: {
        if constexpr (std::is_same_v<T, QString>) {
          it->device_route_description = value;
        }

        break;
      }
      default:
        break;
    }

    Q_EMIT dataChanged(model_index, model_index, {static_cast<int>(role)});
  }

 private:
  QList<NodeInfo> list;

  QSortFilterProxyModel proxy_input_streams;
  QSortFilterProxyModel proxy_output_streams;
  QSortFilterProxyModel proxy_sink_devices;
  QSortFilterProxyModel proxy_source_devices;

  constexpr static auto icon_map =
      std::to_array<std::pair<const char*, const char*>>({{"chromium-browser", "chromium"},
                                                          {"firefox", "firefox"},
                                                          {"nightly", "firefox-nightly"},
                                                          {"obs", "com.obsproject.Studio"}});

  static auto node_state_to_qstring(const pw_node_state& state) -> QString;

  static auto get_app_icon_name(const NodeInfo* node_info) -> QString;

  void onOutputBlocklistChanged();

  void onInputBlocklistChanged();
};

}  // namespace pw::models
