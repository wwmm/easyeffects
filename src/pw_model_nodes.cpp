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

#include "pw_model_nodes.hpp"
#include <pipewire/node.h>
#include <pipewire/proxy.h>
#include <qabstractitemmodel.h>
#include <qhash.h>
#include <qhashfunctions.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qqml.h>
#include <qsortfilterproxymodel.h>
#include <qstringview.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <qvariant.h>
#include <KLocalizedString>
#include <algorithm>
#include <format>
#include <iterator>
#include "config.h"
#include "db_manager.hpp"
#include "pw_manager.hpp"
#include "pw_objects.hpp"
#include "tags_pipewire.hpp"
#include "util.hpp"

namespace pw::models {

Nodes::Nodes(QObject* parent)
    : QAbstractListModel(parent),
      proxy_input_streams(QSortFilterProxyModel(this)),
      proxy_output_streams(QSortFilterProxyModel(this)),
      proxy_sink_devices(QSortFilterProxyModel(this)),
      proxy_source_devices(QSortFilterProxyModel(this)) {
  // Output streams model

  {
    proxy_output_streams.setSourceModel(this);
    proxy_output_streams.setFilterRole(static_cast<int>(Roles::MediaClass));
    proxy_output_streams.setSortRole(static_cast<int>(Roles::AppName));
    proxy_output_streams.setSortCaseSensitivity(Qt::CaseInsensitive);
    proxy_output_streams.setDynamicSortFilter(true);

    auto pattern = "^" + QString(tags::pipewire::media_class::output_stream) + "$";
    proxy_output_streams.setFilterRegularExpression(
        QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));

    proxy_output_streams.sort(0, Qt::AscendingOrder);

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete)
    qmlRegisterSingletonInstance<QSortFilterProxyModel>("ee.pipewire", VERSION_MAJOR, VERSION_MINOR,
                                                        "ModelOutputStreams", &proxy_output_streams);
  }

  // Input streams model

  {
    proxy_input_streams.setSourceModel(this);
    proxy_input_streams.setFilterRole(static_cast<int>(Roles::MediaClass));
    proxy_input_streams.setSortRole(static_cast<int>(Roles::AppName));
    proxy_input_streams.setSortCaseSensitivity(Qt::CaseInsensitive);
    proxy_input_streams.setDynamicSortFilter(true);

    auto pattern = "^" + QString(tags::pipewire::media_class::input_stream) + "$";
    proxy_input_streams.setFilterRegularExpression(
        QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));

    proxy_input_streams.sort(0, Qt::AscendingOrder);

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete)
    qmlRegisterSingletonInstance<QSortFilterProxyModel>("ee.pipewire", VERSION_MAJOR, VERSION_MINOR,
                                                        "ModelInputStreams", &proxy_input_streams);
  }

  // Source devices model

  {
    proxy_source_devices.setSourceModel(this);
    proxy_source_devices.setSortRole(static_cast<int>(Roles::Description));
    proxy_source_devices.setSortCaseSensitivity(Qt::CaseInsensitive);
    proxy_source_devices.setFilterRole(static_cast<int>(Roles::MediaClass));
    proxy_source_devices.setDynamicSortFilter(true);

    auto pattern = "^" + QString(tags::pipewire::media_class::source) + "$";
    proxy_source_devices.setFilterRegularExpression(
        QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));

    proxy_source_devices.sort(0, Qt::AscendingOrder);

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete)
    qmlRegisterSingletonInstance<QSortFilterProxyModel>("ee.pipewire", VERSION_MAJOR, VERSION_MINOR,
                                                        "ModelSourceDevices", &proxy_source_devices);
  }

  // Output devices model

  {
    proxy_sink_devices.setSourceModel(this);
    proxy_sink_devices.setSortRole(static_cast<int>(Roles::Description));
    proxy_sink_devices.setSortCaseSensitivity(Qt::CaseInsensitive);
    proxy_sink_devices.setFilterRole(static_cast<int>(Roles::MediaClass));
    proxy_sink_devices.setDynamicSortFilter(true);

    auto pattern = "^" + QString(tags::pipewire::media_class::sink) + "$";
    proxy_sink_devices.setFilterRegularExpression(
        QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));

    proxy_sink_devices.sort(0, Qt::AscendingOrder);

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete)
    qmlRegisterSingletonInstance<QSortFilterProxyModel>("ee.pipewire", VERSION_MAJOR, VERSION_MINOR, "ModelSinkDevices",
                                                        &proxy_sink_devices);
  }

  connect(
      DbStreamOutputs::self(), &DbStreamOutputs::blocklistChanged, this, [&]() { onOutputBlocklistChanged(); },
      Qt::QueuedConnection);

  connect(
      DbStreamOutputs::self(), &DbStreamOutputs::blocklistUsesMediaNameChanged, this,
      [&]() { onOutputBlocklistChanged(); }, Qt::QueuedConnection);

  connect(
      DbStreamInputs::self(), &DbStreamInputs::blocklistChanged, this, [&]() { onInputBlocklistChanged(); },
      Qt::QueuedConnection);

  connect(
      DbStreamInputs::self(), &DbStreamInputs::blocklistUsesMediaNameChanged, this,
      [&]() { onInputBlocklistChanged(); }, Qt::QueuedConnection);
}

int Nodes::rowCount(const QModelIndex& /* parent */) const {
  return list.size();
}

QHash<int, QByteArray> Nodes::roleNames() const {
  return {{static_cast<int>(Roles::Id), "id"},
          {static_cast<int>(Roles::Serial), "serial"},
          {static_cast<int>(Roles::DeviceId), "deviceId"},
          {static_cast<int>(Roles::Name), "name"},
          {static_cast<int>(Roles::Description), "description"},
          {static_cast<int>(Roles::MediaClass), "mediaClass"},
          {static_cast<int>(Roles::MediaRole), "mediaRole"},
          {static_cast<int>(Roles::AppName), "appName"},
          {static_cast<int>(Roles::AppProcessId), "appProcessId"},
          {static_cast<int>(Roles::AppProcessBinary), "appProcessBinary"},
          {static_cast<int>(Roles::AppIconName), "appIconName"},
          {static_cast<int>(Roles::MediaIconName), "mediaIconName"},
          {static_cast<int>(Roles::DeviceIconName), "deviceIconName"},
          {static_cast<int>(Roles::MediaName), "mediaName"},
          {static_cast<int>(Roles::Format), "format"},
          {static_cast<int>(Roles::ApplicationId), "applicationId"},
          {static_cast<int>(Roles::Priority), "priority"},
          {static_cast<int>(Roles::State), "state"},
          {static_cast<int>(Roles::Mute), "mute"},
          {static_cast<int>(Roles::Connected), "connected"},
          {static_cast<int>(Roles::NinputPorts), "nInputPorts"},
          {static_cast<int>(Roles::NoutputPorts), "nOutputPorts"},
          {static_cast<int>(Roles::Rate), "rate"},
          {static_cast<int>(Roles::NvolumeChannels), "nVolumeChannels"},
          {static_cast<int>(Roles::Latency), "latency"},
          {static_cast<int>(Roles::Volume), "volume"},
          {static_cast<int>(Roles::IsBlocklisted), "isBlocklisted"},
          {static_cast<int>(Roles::DeviceRouteName), "deviceRouteName"},
          {static_cast<int>(Roles::DeviceRouteDescription), "deviceRouteDescription"}};
}

QVariant Nodes::data(const QModelIndex& index, int role) const {
  if (list.empty() || !index.isValid()) {
    return "";
  }

  const auto it = std::next(list.begin(), index.row());

  if (it == list.end()) {
    util::warning(std::format("Invalid model index.row(): {}", index.row()));

    return {};
  }

  switch (static_cast<Roles>(role)) {
    case Roles::Id:
      return it->id;
    case Roles::Serial:
      return static_cast<qint64>(it->serial);
    case Roles::DeviceId:
      return it->device_id;
    case Roles::Name:
      return it->name;
    case Roles::Description:
      return (!it->description.isEmpty()) ? it->description : it->name;
    case Roles::MediaClass:
      return it->media_class;
    case Roles::MediaRole:
      return it->media_role;
    case Roles::AppName:
      return it->app_name;
    case Roles::AppProcessId:
      return it->app_process_id;
    case Roles::AppProcessBinary:
      return it->app_process_binary;
    case Roles::AppIconName:
      return get_app_icon_name(&(*it));
    case Roles::MediaIconName:
      return it->media_icon_name;
    case Roles::DeviceIconName:
      return it->device_icon_name;
    case Roles::MediaName:
      return it->media_name;
    case Roles::Format:
      return it->format;
    case Roles::ApplicationId:
      return it->application_id;
    case Roles::Priority:
      return it->priority;
    case Roles::State:
      return node_state_to_qstring(it->state);
    case Roles::Mute:
      return it->mute;
    case Roles::Connected:
      return it->connected;
    case Roles::NinputPorts:
      return it->n_input_ports;
    case Roles::NoutputPorts:
      return it->n_output_ports;
    case Roles::Rate:
      return static_cast<float>(it->rate) / 1000.0F;
    case Roles::NvolumeChannels:
      return it->n_volume_channels;
    case Roles::Latency:
      return it->latency * 1000.0F;
    case Roles::Volume:
      return it->volume;
    case Roles::IsBlocklisted:
      return it->is_blocklisted;
    case Roles::DeviceRouteName:
      return it->device_route_name;
    case Roles::DeviceRouteDescription:
      return it->device_route_description;
    default:
      return {};
  }
}

bool Nodes::setData(const QModelIndex& index, const QVariant& value, int role) {
  auto it = std::next(list.begin(), index.row());

  switch (static_cast<Roles>(role)) {
    case Roles::IsBlocklisted: {
      if (value.canConvert<bool>()) {
        it->is_blocklisted = value.toBool();

        if (it->is_blocklisted) {
          if (it->media_class == tags::pipewire::media_class::output_stream) {
            auto blocklist = DbStreamOutputs::blocklist();

            if (blocklist.indexOf(it->name) == -1) {
              if (DbStreamOutputs::blocklistUsesMediaName()) {
                blocklist.append(it->name + ":" + it->media_name);
              } else {
                blocklist.append(it->name);
              }

              DbStreamOutputs::setBlocklist(blocklist);
            }

          } else if (it->media_class == tags::pipewire::media_class::input_stream) {
            auto blocklist = DbStreamInputs::blocklist();

            if (blocklist.indexOf(it->name) == -1) {
              if (DbStreamInputs::blocklistUsesMediaName()) {
                blocklist.append(it->name + ":" + it->media_name);
              } else {
                blocklist.append(it->name);
              }

              DbStreamInputs::setBlocklist(blocklist);
            }
          }
        } else {
          if (it->media_class == tags::pipewire::media_class::output_stream) {
            auto blocklist = DbStreamOutputs::blocklist();

            auto idx = blocklist.indexOf(it->name);

            idx = (idx == -1) ? blocklist.indexOf(it->application_id) : idx;

            idx = (idx == -1) ? blocklist.indexOf(it->name + ":" + it->media_name) : idx;

            if (idx != -1) {
              blocklist.removeAt(idx);

              DbStreamOutputs::setBlocklist(blocklist);
            }
          } else if (it->media_class == tags::pipewire::media_class::input_stream) {
            auto blocklist = DbStreamInputs::blocklist();

            auto idx = blocklist.indexOf(it->name);

            idx = (idx == -1) ? blocklist.indexOf(it->application_id) : idx;

            idx = (idx == -1) ? blocklist.indexOf(it->name + ":" + it->media_name) : idx;

            if (idx != -1) {
              blocklist.removeAt(idx);

              DbStreamInputs::setBlocklist(blocklist);
            }
          }
        }

        Q_EMIT dataChanged(index, index, {static_cast<int>(Roles::IsBlocklisted)});
      }

      break;
    }
    default:
      break;
  }

  return true;
}

auto Nodes::get_list() -> QList<NodeInfo> {
  return list;
}

void Nodes::append(NodeInfo info) {
  beginInsertRows(QModelIndex(), list.size(), list.size());

  list.append(info);

  endInsertRows();

  Q_EMIT dataChanged(index(0), index(list.size() - 1));
}

void Nodes::remove_by_id(const uint& id) {
  int rowIndex = -1;

  for (int n = 0; n < list.size(); n++) {
    if (list[n].id == id) {
      rowIndex = n;

      break;
    }
  }

  if (rowIndex == -1) {
    return;
  }

  list[rowIndex].proxy = nullptr;

  beginRemoveRows(QModelIndex(), rowIndex, rowIndex);

  list.remove(rowIndex);

  endRemoveRows();

  Q_EMIT dataChanged(index(0), index(list.size() - 1));
}

void Nodes::remove_by_serial(const uint& serial) {
  int rowIndex = -1;

  for (int n = 0; n < list.size(); n++) {
    if (list[n].serial == serial) {
      rowIndex = n;

      break;
    }
  }

  if (rowIndex == -1) {
    return;
  }

  list[rowIndex].proxy = nullptr;

  beginRemoveRows(QModelIndex(), rowIndex, rowIndex);

  list.remove(rowIndex);

  endRemoveRows();

  Q_EMIT dataChanged(index(0), index(list.size() - 1));
}

auto Nodes::has_serial(const uint& serial) -> bool {
  return std::ranges::find_if(list, [=](const NodeInfo& ni) { return ni.serial == serial; }) != list.end();
}

void Nodes::update_info(NodeInfo new_info) {
  const int row = get_row_by_serial(new_info.serial);

  if (row < 0) {
    return;
  }

  NodeInfo info = list[row];
  bool anyChanged = false;

  auto updateIfDifferent = [&](auto role, const auto& oldVal, const auto& newVal) {
    if (oldVal != newVal) {
      update_field(row, role, newVal);
      anyChanged = true;
    }
  };

  updateIfDifferent(Roles::Id, info.id, new_info.id);
  updateIfDifferent(Roles::Serial, info.serial, new_info.serial);
  updateIfDifferent(Roles::DeviceId, info.device_id, new_info.device_id);
  updateIfDifferent(Roles::Name, info.name, new_info.name);
  updateIfDifferent(Roles::Description, info.description, new_info.description);
  updateIfDifferent(Roles::MediaClass, info.media_class, new_info.media_class);
  updateIfDifferent(Roles::MediaRole, info.media_role, new_info.media_role);
  updateIfDifferent(Roles::AppName, info.app_name, new_info.app_name);
  updateIfDifferent(Roles::AppProcessId, info.app_process_id, new_info.app_process_id);
  updateIfDifferent(Roles::AppProcessBinary, info.app_process_binary, new_info.app_process_binary);
  updateIfDifferent(Roles::AppIconName, info.app_icon_name, new_info.app_icon_name);
  updateIfDifferent(Roles::MediaIconName, info.media_icon_name, new_info.media_icon_name);
  updateIfDifferent(Roles::DeviceIconName, info.device_icon_name, new_info.device_icon_name);
  updateIfDifferent(Roles::MediaName, info.media_name, new_info.media_name);
  updateIfDifferent(Roles::Format, info.format, new_info.format);
  updateIfDifferent(Roles::ApplicationId, info.application_id, new_info.application_id);
  updateIfDifferent(Roles::Priority, info.priority, new_info.priority);
  updateIfDifferent(Roles::State, info.state, new_info.state);
  updateIfDifferent(Roles::Mute, info.mute, new_info.mute);
  updateIfDifferent(Roles::Connected, info.connected, new_info.connected);
  updateIfDifferent(Roles::NinputPorts, info.n_input_ports, new_info.n_input_ports);
  updateIfDifferent(Roles::NoutputPorts, info.n_output_ports, new_info.n_output_ports);
  updateIfDifferent(Roles::Rate, info.rate, new_info.rate);
  updateIfDifferent(Roles::NvolumeChannels, info.n_volume_channels, new_info.n_volume_channels);
  updateIfDifferent(Roles::Latency, info.latency, new_info.latency);
  updateIfDifferent(Roles::Volume, info.volume, new_info.volume);
  updateIfDifferent(Roles::IsBlocklisted, info.is_blocklisted, new_info.is_blocklisted);
  updateIfDifferent(Roles::DeviceRouteName, info.device_route_name, new_info.device_route_name);
  updateIfDifferent(Roles::DeviceRouteDescription, info.device_route_description, new_info.device_route_description);
}

auto Nodes::get_row_by_serial(const uint& serial) -> int {
  for (int n = 0; n < list.size(); n++) {
    if (list[n].serial == serial) {
      return n;
    }
  }

  return -1;
}

auto Nodes::get_proxy_by_serial(const uint& serial) -> pw_proxy* {
  for (auto& info : list) {
    if (info.serial == serial) {
      return info.proxy;
    }
  }

  return nullptr;
}

void Nodes::reset() {
  beginResetModel();

  list.clear();

  endResetModel();
}

void Nodes::begin_reset() {
  beginResetModel();
}

void Nodes::end_reset() {
  endResetModel();
}

auto Nodes::node_state_to_qstring(const pw_node_state& state) -> QString {
  switch (state) {
    case PW_NODE_STATE_RUNNING:
      return i18n("Running");
    case PW_NODE_STATE_SUSPENDED:
      return i18n("Suspended");
    case PW_NODE_STATE_IDLE:
      return i18n("Idle");
    case PW_NODE_STATE_CREATING:
      return i18n("Creating");
    case PW_NODE_STATE_ERROR:
      return i18n("Error");
    default:
      return i18n("Unknown");
  }
}

auto Nodes::get_app_icon_name(const NodeInfo* node_info) -> QString {
  // map to handle cases where PipeWire does not set icon name string
  // or app name equal to icon name.

  QString icon_name;

  if (!node_info->app_icon_name.isEmpty()) {
    icon_name = node_info->app_icon_name;
  } else if (!node_info->media_icon_name.isEmpty()) {
    icon_name = node_info->media_icon_name;
  } else if (!node_info->name.isEmpty()) {
    const QString prefix = "alsa_playback.";

    if (node_info->name.startsWith(prefix)) {
      icon_name = node_info->name.sliced(prefix.size() + 1);
    } else {
      icon_name = node_info->name;
    }

    // get lowercase name so if it changes in the future, we have a
    // chance to pick the same index

    icon_name = icon_name.toLower();
  }

  for (const auto& [key, value] : icon_map) {
    if (key == icon_name) {
      return value;
    }
  }

  return icon_name;
}

QString Nodes::getNodeName(const uint& rowIndex) {
  if (rowIndex >= list.size()) {
    return "";
  }

  return list[rowIndex].name;
}

QString Nodes::getNodeDescription(QString nodeName) {
  for (const auto& node : list) {
    if (node.name == nodeName) {
      return node.description;
    }
  }

  return "";
}

QModelIndex Nodes::getModelIndexByName(QString nodeName) {
  for (int n = 0; n < list.size(); n++) {
    if (list[n].name == nodeName) {
      return this->index(n);
    }
  }

  return this->index(-1);
}

auto Nodes::get_node_by_name(QString name) -> NodeInfo {
  for (const auto& node : list) {
    if (node.name == name) {
      return node;
    }
  }

  return {};
}

auto Nodes::get_node_by_id(const uint& id) -> NodeInfo {
  for (const auto& node : list) {
    if (node.id == id) {
      return node;
    }
  }

  return {};
}

auto Nodes::get_nodes_by_device_id(const uint& id) -> QList<NodeInfo> {
  QList<NodeInfo> nodes;

  for (const auto& node : list) {
    if (node.device_id == id) {
      nodes.append(node);
    }
  }

  return nodes;
}

void Nodes::onOutputBlocklistChanged() {
  const auto blocklist = DbStreamOutputs::blocklist();

  for (qsizetype n = 0; n < list.size(); n++) {
    if (list[n].media_class != tags::pipewire::media_class::output_stream) {
      continue;
    }

    auto is_blocklisted = blocklist.contains(list[n].name) || blocklist.contains(list[n].application_id) ||
                          blocklist.contains(list[n].app_process_binary);

    if (DbStreamOutputs::blocklistUsesMediaName()) {
      is_blocklisted = is_blocklisted || blocklist.contains(list[n].name + ":" + list[n].media_name);
    }

    if (is_blocklisted) {
      update_field(n, Roles::IsBlocklisted, true);

      pw::Manager::self().disconnectStream(list[n].id);
    } else {
      update_field(n, Roles::IsBlocklisted, false);

      if (DbMain::processAllOutputs()) {
        pw::Manager::self().connectStreamOutput(list[n].id);
      }
    }
  }
}

void Nodes::onInputBlocklistChanged() {
  const auto blocklist = DbStreamInputs::blocklist();

  for (qsizetype n = 0; n < list.size(); n++) {
    if (list[n].media_class != tags::pipewire::media_class::input_stream) {
      continue;
    }

    auto is_blocklisted = blocklist.contains(list[n].name) || blocklist.contains(list[n].application_id) ||
                          blocklist.contains(list[n].app_process_binary);

    if (DbStreamInputs::blocklistUsesMediaName()) {
      is_blocklisted = is_blocklisted || blocklist.contains(list[n].name + ":" + list[n].media_name);
    }

    if (is_blocklisted) {
      update_field(n, Roles::IsBlocklisted, true);

      pw::Manager::self().disconnectStream(list[n].id);
    } else {
      update_field(n, Roles::IsBlocklisted, false);

      if (DbMain::processAllInputs()) {
        pw::Manager::self().connectStreamInput(list[n].id);
      }
    }
  }
}

}  // namespace pw::models
