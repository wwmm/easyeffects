#include "pw_model_clients.hpp"
#include <qabstractitemmodel.h>
#include <qhash.h>
#include <qhashfunctions.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qstringview.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <qvariant.h>
#include <iterator>
#include "pw_objects.hpp"

namespace pw::models {

Clients::Clients(QObject* parent) : QAbstractListModel(parent) {}

int Clients::rowCount(const QModelIndex& /*parent*/) const {
  return list.size();
}

QHash<int, QByteArray> Clients::roleNames() const {
  return {{Roles::Id, "id"},
          {Roles::Serial, "serial"},
          {Roles::Name, "name"},
          {Roles::Access, "access"},
          {Roles::Api, "api"}};
}

QVariant Clients::data(const QModelIndex& index, int role) const {
  if (list.empty()) {
    return "";
  }

  const auto it = std::next(list.begin(), index.row());

  switch (role) {
    case Roles::Id:
      return it->id;
    case Roles::Serial:
      return static_cast<qint64>(it->serial);
    case Roles::Name:
      return it->name;
    case Roles::Access:
      return it->access;
    case Roles::Api:
      return it->api;
    default:
      return {};
  }
}

auto Clients::get_list() -> QList<ClientInfo> {
  return list;
}

void Clients::append(const ClientInfo& info) {
  int pos = list.empty() ? 0 : list.size() - 1;

  beginInsertRows(QModelIndex(), pos, pos);

  list.append(info);

  endInsertRows();

  emit dataChanged(index(0), index(list.size() - 1));
}

void Clients::remove_by_id(const uint& id) {
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

void Clients::reset() {
  beginResetModel();

  list.clear();

  endResetModel();
}

void Clients::begin_reset() {
  beginResetModel();
}

void Clients::end_reset() {
  endResetModel();
}

}  // namespace pw::models