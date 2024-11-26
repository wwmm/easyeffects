/*
 *  Copyright © 2017-2024 Wellington Wallace
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

#include "presets_list_model.hpp"
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
#include <qvariant.h>
#include <iterator>
#include "config.h"

ListModel::ListModel(QObject* parent) : QAbstractListModel(parent), proxy(new QSortFilterProxyModel(this)) {
  proxy->setSourceModel(this);
  proxy->setFilterRole(Roles::Name);
  proxy->setSortRole(Roles::Name);
  proxy->setDynamicSortFilter(true);
  proxy->sort(0);
}

int ListModel::rowCount(const QModelIndex& /*parent*/) const {
  return list.size();
}

QHash<int, QByteArray> ListModel::roleNames() const {
  return {{Roles::Name, "name"}};
}

QVariant ListModel::data(const QModelIndex& index, int role) const {
  if (list.empty()) {
    return "";
  }

  const auto it = std::next(list.begin(), index.row());

  switch (role) {
    case Roles::Name:
      return *it;
    default:
      return {};
  }
}

bool ListModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!value.canConvert<QString>() && role != Qt::EditRole) {
    return false;
  }

  auto it = std::next(list.begin(), index.row());

  switch (role) {
    case Roles::Name: {
      *it = value.toString();

      emit dataChanged(index, index, {Roles::Name});

      break;
    }
    default:
      break;
  }

  return true;
}

void ListModel::append(const QString& name) {
  int pos = list.empty() ? 0 : list.size() - 1;

  beginInsertRows(QModelIndex(), pos, pos);

  list.append(name);

  endInsertRows();

  emit dataChanged(index(0), index(list.size() - 1));
}

void ListModel::remove(const int& rowIndex) {
  beginRemoveRows(QModelIndex(), rowIndex, rowIndex);

  list.remove(rowIndex);

  endRemoveRows();

  emit dataChanged(index(0), index(list.size() - 1));
}

void ListModel::remove(const QString& name) {
  auto rowIndex = list.indexOf(name);

  if (rowIndex == -1) {
    return;
  }

  beginRemoveRows(QModelIndex(), rowIndex, rowIndex);

  list.remove(rowIndex);

  endRemoveRows();

  emit dataChanged(index(0), index(list.size() - 1));
}

void ListModel::reset() {
  beginResetModel();

  list.clear();

  endResetModel();
}

void ListModel::begin_reset() {
  beginResetModel();
}

void ListModel::end_reset() {
  endResetModel();
}

auto ListModel::getList() -> QList<QString> {
  return list;
}

QSortFilterProxyModel* ListModel::getProxy() {
  return proxy;
}