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

#include <kconfigskeleton.h>
#include <ksharedconfig.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <QJSEngine>
#include <QQmlEngine>
#include <QString>

class KConfigBaseEE : public KConfigSkeleton {
  Q_OBJECT
 public:
  KConfigBaseEE() = default;
  KConfigBaseEE(const QString& configname = QString(), QObject* parent = nullptr);
  KConfigBaseEE(KSharedConfig::Ptr config, QObject* parent = nullptr);

  Q_INVOKABLE QVariant getMinValue(const QString& itemName);
  Q_INVOKABLE QVariant getMaxValue(const QString& itemName);

  Q_INVOKABLE void resetProperty(const QString& itemName);

  Q_INVOKABLE QVariant getDefaultValue(const QString& itemName);
};
