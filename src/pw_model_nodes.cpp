/*
 *  Copyright © 2017-2025 Wellington Wallace
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

#include "pw_model_nodes.hpp"
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
#include "pipewire/node.h"
#include "pipewire/proxy.h"
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
    proxy_output_streams.setFilterRole(Roles::MediaClass);
    proxy_output_streams.setSortRole(Roles::AppName);
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
    proxy_input_streams.setFilterRole(Roles::MediaClass);
    proxy_input_streams.setSortRole(Roles::AppName);
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
    proxy_source_devices.setSortRole(Roles::Description);
    proxy_source_devices.setSortCaseSensitivity(Qt::CaseInsensitive);
    proxy_source_devices.setFilterRole(Roles::MediaClass);
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
    proxy_sink_devices.setSortRole(Roles::Description);
    proxy_sink_devices.setSortCaseSensitivity(Qt::CaseInsensitive);
    proxy_sink_devices.setFilterRole(Roles::MediaClass);
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
      db::Manager::self().streamOutputs, &db::StreamOutputs::blocklistChanged, this,
      [this]() {
        const auto blocklist = db::StreamOutputs::blocklist();

        for (qsizetype n = 0; n < list.size(); n++) {
          if (list[n].media_class != tags::pipewire::media_class::output_stream) {
            continue;
          }

          if (blocklist.contains(list[n].name) || blocklist.contains(list[n].application_id)) {
            update_field(n, Roles::IsBlocklisted, true);

            pw::Manager::self().disconnectStream(list[n].id);
          } else {
            update_field(n, Roles::IsBlocklisted, false);

            if (db::Main::processAllOutputs()) {
              pw::Manager::self().connectStreamOutput(list[n].id);
            }
          }
        }
      },
      Qt::QueuedConnection);

  connect(
      db::Manager::self().streamInputs, &db::StreamInputs::blocklistChanged, this,
      [this]() {
        const auto blocklist = db::StreamInputs::blocklist();

        for (qsizetype n = 0; n < list.size(); n++) {
          if (list[n].media_class != tags::pipewire::media_class::input_stream) {
            continue;
          }

          if (blocklist.contains(list[n].name) || blocklist.contains(list[n].application_id)) {
            update_field(n, Roles::IsBlocklisted, true);

            pw::Manager::self().disconnectStream(list[n].id);
          } else {
            update_field(n, Roles::IsBlocklisted, false);

            if (db::Main::processAllInputs()) {
              pw::Manager::self().connectStreamInput(list[n].id);
            }
          }
        }
      },
      Qt::QueuedConnection);
}

int Nodes::rowCount(const QModelIndex& /*parent*/) const {
  return list.size();
}

QHash<int, QByteArray> Nodes::roleNames() const {
  return {{Roles::Id, "id"},
          {Roles::Serial, "serial"},
          {Roles::DeviceId, "deviceId"},
          {Roles::Name, "name"},
          {Roles::Description, "description"},
          {Roles::MediaClass, "mediaClass"},
          {Roles::MediaRole, "mediaRole"},
          {Roles::AppName, "appName"},
          {Roles::AppProcessId, "appProcessId"},
          {Roles::AppProcessBinary, "appProcessBinary"},
          {Roles::AppIconName, "appIconName"},
          {Roles::MediaIconName, "mediaIconName"},
          {Roles::DeviceIconName, "deviceIconName"},
          {Roles::MediaName, "mediaName"},
          {Roles::Format, "format"},
          {Roles::ApplicationId, "applicationId"},
          {Roles::Priority, "priority"},
          {Roles::State, "state"},
          {Roles::Mute, "mute"},
          {Roles::Connected, "connected"},
          {Roles::NinputPorts, "nInputPorts"},
          {Roles::NoutputPorts, "nOutputPorts"},
          {Roles::Rate, "rate"},
          {Roles::NvolumeChannels, "nVolumeChannels"},
          {Roles::Latency, "latency"},
          {Roles::Volume, "volume"},
          {Roles::IsBlocklisted, "isBlocklisted"},
          {Roles::DeviceProfileName, "deviceProfileName"},
          {Roles::DeviceProfileDescription, "DeviceProfileDescription"}};
}

QVariant Nodes::data(const QModelIndex& index, int role) const {
  if (list.empty() || !index.isValid()) {
    return "";
  }

  const auto it = std::next(list.begin(), index.row());

  if (it == list.end()) {
    util::warning("invalid model index.row(): " + util::to_string(index.row()));

    return {};
  }

  switch (role) {
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
      return QString::fromStdString(std::format("{0:.1Lf} kHz", static_cast<float>(it->rate) / 1000.0F));
    case Roles::NvolumeChannels:
      return it->n_volume_channels;
    case Roles::Latency:
      return QString::fromStdString(std::format("{0:.0f} ms", 1000.0F * it->latency));
    case Roles::Volume:
      return it->volume;
    case Roles::IsBlocklisted:
      return it->is_blocklisted;
    case Roles::DeviceProfileName:
      return it->device_profile_name;
    case Roles::DeviceProfileDescription:
      return it->device_profile_description;
    default:
      return {};
  }
}

bool Nodes::setData(const QModelIndex& index, const QVariant& value, int role) {
  auto it = std::next(list.begin(), index.row());

  switch (role) {
    case Roles::IsBlocklisted: {
      if (value.canConvert<bool>()) {
        it->is_blocklisted = value.toBool();

        if (it->is_blocklisted) {
          if (it->media_class == tags::pipewire::media_class::output_stream) {
            auto blocklist = db::StreamOutputs::blocklist();

            if (blocklist.indexOf(it->name) == -1) {
              blocklist.append(it->name);
              db::StreamOutputs::setBlocklist(blocklist);
            }

          } else if (it->media_class == tags::pipewire::media_class::input_stream) {
            auto blocklist = db::StreamInputs::blocklist();

            if (blocklist.indexOf(it->name) == -1) {
              blocklist.append(it->name);
              db::StreamInputs::setBlocklist(blocklist);
            }
          }
        } else {
          if (it->media_class == tags::pipewire::media_class::output_stream) {
            auto blocklist = db::StreamOutputs::blocklist();

            auto idx = blocklist.indexOf(it->name);

            idx = (idx == -1) ? blocklist.indexOf(it->application_id) : idx;

            if (idx != -1) {
              blocklist.removeAt(idx);

              db::StreamOutputs::setBlocklist(blocklist);
            }
          } else if (it->media_class == tags::pipewire::media_class::input_stream) {
            auto blocklist = db::StreamInputs::blocklist();

            auto idx = blocklist.indexOf(it->name);

            idx = (idx == -1) ? blocklist.indexOf(it->application_id) : idx;

            if (idx != -1) {
              blocklist.removeAt(idx);

              db::StreamInputs::setBlocklist(blocklist);
            }
          }
        }

        Q_EMIT dataChanged(index, index, {Roles::IsBlocklisted});
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

void Nodes::append(const NodeInfo& info) {
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

void Nodes::update_info(const NodeInfo& new_info) {
  for (int n = 0; n < list.size(); n++) {
    if (list[n].serial == new_info.serial) {
      list[n] = new_info;

      auto model_index = this->index(n);

      Q_EMIT dataChanged(model_index, model_index);

      return;
    }
  }
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
  // map to handle cases where PipeWire does not set icon name string or app name equal to icon name.

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

    // get lowercase name so if it changes in the future, we have a chance to pick the same index

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

QString Nodes::getNodeDescription(const QString& nodeName) {
  for (const auto& node : list) {
    if (node.name == nodeName) {
      return node.description;
    }
  }

  return "";
}

QModelIndex Nodes::getModelIndexByName(const QString& nodeName) {
  for (int n = 0; n < list.size(); n++) {
    if (list[n].name == nodeName) {
      return this->index(n);
    }
  }

  return this->index(-1);
}

auto Nodes::get_node_by_name(const QString& name) -> NodeInfo {
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

}  // namespace pw::models
