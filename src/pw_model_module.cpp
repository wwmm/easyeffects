#include "pw_model_module.hpp"
#include <qabstractitemmodel.h>
#include <qhash.h>
#include <qhashfunctions.h>
#include <qnamespace.h>
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

bool Modules::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!value.canConvert<Modules>() && role != Qt::EditRole) {
    return false;
  }

  auto it = std::next(list.begin(), index.row());

  switch (role) {
    case Roles::Id: {
      it->id = value.toInt();

      emit dataChanged(index, index, {Roles::Id});

      break;
    }
    case Roles::Serial: {
      it->serial = value.toInt();

      emit dataChanged(index, index, {Roles::Serial});

      break;
    }
    case Roles::Name: {
      it->name = value.toString();

      emit dataChanged(index, index, {Roles::Name});

      break;
    }
    case Roles::Description: {
      it->description = value.toString();

      emit dataChanged(index, index, {Roles::Description});

      break;
    }
    case Roles::Filename: {
      it->filename = value.toString();

      emit dataChanged(index, index, {Roles::Filename});

      break;
    }
    default:
      break;
  }

  return true;
}

void Modules::append(const ModuleInfo& info) {
  int pos = list.empty() ? 0 : list.size() - 1;

  beginInsertRows(QModelIndex(), pos, pos);

  list.append(info);

  endInsertRows();

  emit dataChanged(index(0), index(list.size() - 1));
}

void Modules::remove(const int& rowIndex) {
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