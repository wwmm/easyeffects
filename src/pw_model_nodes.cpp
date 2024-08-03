#include "pw_model_nodes.hpp"
#include <qabstractitemmodel.h>
#include <qhash.h>
#include <qhashfunctions.h>
#include <qlist.h>
#include <qobject.h>
#include <qqml.h>
#include <qsortfilterproxymodel.h>
#include <qstringview.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <qvariant.h>
#include <KLocalizedString>
#include <algorithm>
#include <cctype>
#include <format>
#include <iterator>
#include <string>
#include "config.h"
#include "pipewire/node.h"
#include "pw_objects.hpp"

namespace pw::models {

Nodes::Nodes(QObject* parent) : QAbstractListModel(parent) {
  // Output streams model

  auto* proxyOutputStreams = new QSortFilterProxyModel(this);

  proxyOutputStreams->setSourceModel(this);
  proxyOutputStreams->setFilterRole(Roles::MediaClass);
  proxyOutputStreams->setSortRole(Roles::MediaClass);
  proxyOutputStreams->setDynamicSortFilter(true);
  proxyOutputStreams->sort(0);
  proxyOutputStreams->setFilterRegularExpression(
      QRegularExpression("Stream/Output/Audio", QRegularExpression::CaseInsensitiveOption));

  qmlRegisterSingletonInstance<QSortFilterProxyModel>("EEpw", VERSION_MAJOR, VERSION_MINOR, "ModelOutputStreams",
                                                      proxyOutputStreams);

  // Input streams model

  auto* proxyInputStreams = new QSortFilterProxyModel(this);

  proxyInputStreams->setSourceModel(this);
  proxyInputStreams->setFilterRole(Roles::MediaClass);
  proxyInputStreams->setSortRole(Roles::MediaClass);
  proxyInputStreams->setDynamicSortFilter(true);
  proxyInputStreams->sort(0);
  proxyInputStreams->setFilterRegularExpression(
      QRegularExpression("Stream/Input/Audio", QRegularExpression::CaseInsensitiveOption));

  qmlRegisterSingletonInstance<QSortFilterProxyModel>("EEpw", VERSION_MAJOR, VERSION_MINOR, "ModelInputStreams",
                                                      proxyInputStreams);
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
          {Roles::Volume, "volume"}};
}

QVariant Nodes::data(const QModelIndex& index, int role) const {
  if (list.empty()) {
    return "";
  }

  const auto it = std::next(list.begin(), index.row());

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
      return it->description;
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
      return it->app_icon_name;
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
      return QString::fromStdString(std::format("{0:d} {1}", it->n_volume_channels, i18n("channels").toStdString()));
    case Roles::Latency:
      return QString::fromStdString(std::format("{0:.0f} ms", 1000.0F * it->latency));
    case Roles::Volume:
      return it->volume;
    default:
      return {};
  }
}

auto Nodes::get_list() -> QList<NodeInfo> {
  return list;
}

void Nodes::append(const NodeInfo& info) {
  int pos = list.empty() ? 0 : list.size() - 1;

  beginInsertRows(QModelIndex(), pos, pos);

  list.append(info);

  endInsertRows();

  emit dataChanged(index(0), index(list.size() - 1));
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

  beginRemoveRows(QModelIndex(), rowIndex, rowIndex);

  list.remove(rowIndex);

  endRemoveRows();

  emit dataChanged(index(0), index(list.size() - 1));
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

  beginRemoveRows(QModelIndex(), rowIndex, rowIndex);

  list.remove(rowIndex);

  endRemoveRows();

  emit dataChanged(index(0), index(list.size() - 1));
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

auto Nodes::get_app_icon_name(const NodeInfo& node_info) -> std::string {
  // map to handle cases where PipeWire does not set icon name string or app name equal to icon name.

  std::string icon_name;

  if (!node_info.app_icon_name.isEmpty()) {
    icon_name = node_info.app_icon_name.toStdString();
  } else if (!node_info.media_icon_name.isEmpty()) {
    icon_name = node_info.media_icon_name.toStdString();
  } else if (!node_info.name.isEmpty()) {
    const std::string prefix = "alsa_playback.";

    if (node_info.name.toStdString().substr(0, prefix.size()) == prefix) {
      icon_name = node_info.name.toStdString().substr(prefix.size());
    } else {
      icon_name = node_info.name.toStdString();
    }

    // get lowercase name so if it changes in the future, we have a chance to pick the same index

    std::transform(icon_name.begin(), icon_name.end(), icon_name.begin(),
                   [](unsigned char c) { return std::tolower(c); });
  }

  for (const auto& [key, value] : icon_map) {
    if (key == icon_name) {
      return value;
    }
  }

  return icon_name;
}

}  // namespace pw::models