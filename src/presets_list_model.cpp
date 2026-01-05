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

#include "presets_list_model.hpp"
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
#include <filesystem>
#include <fstream>
#include <iterator>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include "config.h"

ListModel::ListModel(QObject* parent, const ModelType& model_type)
    : QAbstractListModel(parent), proxy(new QSortFilterProxyModel(this)), model_type(model_type) {
  proxy->setSourceModel(this);

  switch (model_type) {
    case ModelType::Local:
      proxy->setFilterRole(static_cast<int>(Roles::Name));
      proxy->setSortRole(static_cast<int>(Roles::Name));
      break;
    case ModelType::Community:
      proxy->setFilterRole(static_cast<int>(Roles::Path));
      proxy->setSortRole(static_cast<int>(Roles::Path));
      break;
    case ModelType::Autoload:
      proxy->setFilterRole(static_cast<int>(Roles::DeviceDescription));
      proxy->setSortRole(static_cast<int>(Roles::DeviceDescription));
      break;
    case ModelType::IRS:
      proxy->setFilterRole(static_cast<int>(Roles::Name));
      proxy->setSortRole(static_cast<int>(Roles::Name));
      break;
    case ModelType::RNNOISE:
      proxy->setFilterRole(static_cast<int>(Roles::Name));
      proxy->setSortRole(static_cast<int>(Roles::Name));
      break;
  }

  proxy->setDynamicSortFilter(true);
  proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
  proxy->sort(0);
}

int ListModel::rowCount(const QModelIndex& /* parent */) const {
  return listPaths.size();
}

QHash<int, QByteArray> ListModel::roleNames() const {
  return {{static_cast<int>(Roles::Name), "name"},
          {static_cast<int>(Roles::Path), "path"},
          {static_cast<int>(Roles::PresetPackage), "presetPackage"},
          {static_cast<int>(Roles::DeviceName), "deviceName"},
          {static_cast<int>(Roles::DeviceDescription), "deviceDescription"},
          {static_cast<int>(Roles::DeviceProfile), "deviceProfile"},
          {static_cast<int>(Roles::DevicePreset), "devicePreset"}};
}

QVariant ListModel::data(const QModelIndex& index, int role) const {
  if (listPaths.empty()) {
    return "";
  }

  const auto it = std::next(listPaths.begin(), index.row());

  if (model_type == ModelType::Local || model_type == ModelType::Community || model_type == ModelType::IRS ||
      model_type == ModelType::RNNOISE) {
    switch (static_cast<Roles>(role)) {
      case Roles::Name:
        return QString::fromStdString(it->stem().string());
      case Roles::Path:
        return QString::fromStdString(it->string());
      case Roles::PresetPackage:
        return QString::fromStdString(it->parent_path().stem().string());  // getting the parent folder
      default:
        return {};
    }
  }

  if (model_type == ModelType::Autoload) {
    nlohmann::json json;

    std::ifstream is(*it);

    is >> json;

    switch (static_cast<Roles>(role)) {
      case Roles::DeviceName:
        return QString::fromStdString(json.value("device", ""));
      case Roles::DeviceDescription:
        return QString::fromStdString(json.value("device-description", ""));
      case Roles::DeviceProfile:
        return QString::fromStdString(json.value("device-profile", ""));
      case Roles::DevicePreset:
        return QString::fromStdString(json.value("preset-name", ""));
      default:
        return {};
    }
  }

  return {};
}

void ListModel::append(const std::filesystem::path& path) {
  int pos = listPaths.empty() ? 0 : listPaths.size() - 1;

  beginInsertRows(QModelIndex(), pos, pos);

  listPaths.append(path);

  endInsertRows();

  Q_EMIT dataChanged(index(0), index(listPaths.size() - 1));
}

void ListModel::remove(const QString& name) {
  qsizetype rowIndex = -1;

  for (qsizetype n = 0; n < listPaths.size(); n++) {
    if (listPaths[n].stem().string() == name.toStdString()) {
      rowIndex = n;
    }
  }

  if (rowIndex == -1) {
    return;
  }

  beginRemoveRows(QModelIndex(), rowIndex, rowIndex);

  listPaths.remove(rowIndex);

  endRemoveRows();

  Q_EMIT dataChanged(index(0), index(listPaths.size() - 1));
}

void ListModel::remove(const int& rowIndex) {
  beginRemoveRows(QModelIndex(), rowIndex, rowIndex);

  listPaths.remove(rowIndex);

  endRemoveRows();

  Q_EMIT dataChanged(index(0), index(listPaths.size() - 1));
}

void ListModel::remove(const std::filesystem::path& path) {
  qsizetype rowIndex = listPaths.indexOf(path);

  beginRemoveRows(QModelIndex(), rowIndex, rowIndex);

  listPaths.remove(rowIndex);

  endRemoveRows();

  Q_EMIT dataChanged(index(0), index(listPaths.size() - 1));
}

void ListModel::reset() {
  beginResetModel();

  listPaths.clear();

  endResetModel();
}

void ListModel::begin_reset() {
  beginResetModel();
}

void ListModel::end_reset() {
  endResetModel();
}

auto ListModel::getList() -> QList<std::filesystem::path> {
  return listPaths;
}

QSortFilterProxyModel* ListModel::getProxy() {
  return proxy;
}

void ListModel::emit_data_changed(const std::filesystem::path& path) {
  qsizetype rowIndex = listPaths.indexOf(path);

  Q_EMIT dataChanged(index(rowIndex), index(rowIndex));
}

void ListModel::update(const QList<std::filesystem::path>& paths) {
  auto current_list = listPaths;

  beginResetModel();

  for (const auto& v : paths) {
    if (!current_list.contains(v)) {
      append(v);
    }
  }

  for (const auto& v : current_list) {
    if (!paths.contains(v)) {
      remove(v);
    }
  }

  endResetModel();
}
