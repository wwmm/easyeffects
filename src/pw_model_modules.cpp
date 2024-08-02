#include "pw_model_modules.hpp"
#include <qabstractitemmodel.h>
#include <qhash.h>
#include <qhashfunctions.h>
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

int Modules::rowCount(const QModelIndex& /*parent*/) const {
  return list.size();
}

QHash<int, QByteArray> Modules::roleNames() const {
  return {{Roles::Id, "id"},
          {Roles::Serial, "serial"},
          {Roles::Name, "name"},
          {Roles::Description, "description"},
          {Roles::Filename, "filename"}};
}

QVariant Modules::data(const QModelIndex& index, int role) const {
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
    case Roles::Description:
      return it->description;
    case Roles::Filename:
      return it->filename;
    default:
      return {};
  }
}

auto Modules::get_list() -> QList<ModuleInfo> {
  return list;
}

void Modules::append(const ModuleInfo& info) {
  int pos = list.empty() ? 0 : list.size() - 1;

  beginInsertRows(QModelIndex(), pos, pos);

  list.append(info);

  endInsertRows();

  emit dataChanged(index(0), index(list.size() - 1));
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

  emit dataChanged(index(0), index(list.size() - 1));
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