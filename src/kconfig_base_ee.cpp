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

#include "kconfig_base_ee.hpp"
#include <kconfigskeleton.h>
#include <ksharedconfig.h>
#include <qobject.h>
#include <QString>

KConfigBaseEE::KConfigBaseEE([[maybe_unused]] const QString& configname, [[maybe_unused]] QObject* parent)
    : KConfigSkeleton(configname) {}

KConfigBaseEE::KConfigBaseEE([[maybe_unused]] KSharedConfig::Ptr config, [[maybe_unused]] QObject* parent) {}

QVariant KConfigBaseEE::getMinValue(const QString& itemName) {
  if (itemName.isEmpty()) {
    return {};
  }

  if (auto item = findItem(itemName); item != nullptr) {
    return item->minValue();
  }

  return {};
}

QVariant KConfigBaseEE::getMaxValue(const QString& itemName) {
  if (itemName.isEmpty()) {
    return {};
  }

  if (auto item = findItem(itemName); item != nullptr) {
    return item->maxValue();
  }

  return {};
}

void KConfigBaseEE::resetProperty(const QString& itemName) {
  if (itemName.isEmpty()) {
    return;
  }

  if (auto item = findItem(itemName); item != nullptr) {
    item->setDefault();
  }
}

QVariant KConfigBaseEE::getDefaultValue(const QString& itemName) {
  if (itemName.isEmpty()) {
    return {};
  }

  if (auto item = findItem(itemName); item != nullptr) {
    return item->getDefault();
  }

  return {};
}
