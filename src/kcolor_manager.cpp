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

#include "kcolor_manager.hpp"
#include <kcolorschememanager.h>
#include <qabstractitemmodel.h>

KColorManager::KColorManager() : manager(KColorSchemeManager::instance()), model(manager->model()) {
  manager->setAutosaveChanges(true);
}

void KColorManager::activateScheme(const int& index) {
  manager->activateScheme(model->index(index, 0));
}

int KColorManager::activeScheme() {
  auto index = manager->indexForSchemeId(manager->activeSchemeId());

  if (!index.isValid()) {
    return 0;
  }

  return index.row();
}
