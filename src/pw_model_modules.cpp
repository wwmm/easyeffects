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

#include "pw_model_modules.hpp"
#include <qabstractitemmodel.h>
#include <qhash.h>
#include <qlist.h>
#include <qobject.h>
#include <qstringview.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <qvariant.h>
#include <iterator>
#include "pw_objects.hpp"

namespace pw::models {

Modules::Modules(QObject* parent) : QAbstractListModel(parent) {}

int Modules::rowCount(const QModelIndex& /* parent */) const {
  return list.size();
}

QHash<int, QByteArray> Modules::roleNames() const {
  return {{static_cast<int>(Roles::Id), "id"},
          {static_cast<int>(Roles::Serial), "serial"},
          {static_cast<int>(Roles::Name), "name"},
          {static_cast<int>(Roles::Description), "description"},
          {static_cast<int>(Roles::Filename), "filename"},
          {static_cast<int>(Roles::Version), "version"}};
}

QVariant Modules::data(const QModelIndex& index, int role) const {
  if (list.empty()) {
    return "";
  }

  const auto it = std::next(list.begin(), index.row());

  switch (static_cast<Roles>(role)) {
    case Roles::Id:
      return it->id;
    case Roles::Serial:
      return static_cast<qint64>(it->serial);
    case Roles::Name:
      return it->name;
    case Roles::Description:
      return it->description;
    case Roles::Filename:
      return it->filename;
    case Roles::Version:
      return it->version;
    default:
      return {};
  }
}

auto Modules::get_list() -> QList<ModuleInfo> {
  return list;
}

void Modules::append(const ModuleInfo& info) {
  beginInsertRows(QModelIndex(), list.size(), list.size());

  list.append(info);

  endInsertRows();

  Q_EMIT dataChanged(index(0), index(list.size() - 1));
}

void Modules::remove_by_id(const uint& id) {
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

  Q_EMIT dataChanged(index(0), index(list.size() - 1));
}

void Modules::reset() {
  beginResetModel();

  list.clear();

  endResetModel();
}

void Modules::begin_reset() {
  beginResetModel();
}

void Modules::end_reset() {
  endResetModel();
}

}  // namespace pw::models
