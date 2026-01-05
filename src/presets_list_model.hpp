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

#include <qabstractitemmodel.h>
#include <qhash.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qqmlintegration.h>
#include <qsortfilterproxymodel.h>
#include <qtmetamacros.h>
#include <filesystem>

class ListModel : public QAbstractListModel {
  Q_OBJECT
  QML_NAMED_ELEMENT(PresetsListModel)

 public:
  enum class ModelType { Local = Qt::UserRole, Community, Autoload, IRS, RNNOISE };

  explicit ListModel(QObject* parent = nullptr, const ModelType& model_type = ModelType::Local);

  enum class Roles {
    Name = Qt::UserRole,
    Path,
    PresetPackage,
    DeviceName,
    DeviceDescription,
    DeviceProfile,
    DevicePreset
  };
  Q_ENUM(Roles)

  [[nodiscard]] int rowCount(const QModelIndex& /* parent */) const override;

  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;

  void reset();

  void begin_reset();

  void end_reset();

  auto getList() -> QList<std::filesystem::path>;

  void append(const std::filesystem::path& path);

  void remove(const QString& name);

  void remove(const int& rowIndex);

  void remove(const std::filesystem::path& path);

  void update(const QList<std::filesystem::path>& paths);

  void emit_data_changed(const std::filesystem::path& path);

  Q_INVOKABLE QSortFilterProxyModel* getProxy();

 private:
  QList<std::filesystem::path> listPaths;

  QSortFilterProxyModel* proxy = nullptr;

  ModelType model_type;
};
