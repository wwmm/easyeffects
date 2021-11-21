/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "module_info_holder.hpp"
#include <string>

namespace ui::holders {

G_DEFINE_TYPE(ModuleInfoHolder, module_info_holder, G_TYPE_OBJECT);

void module_info_holder_finalize(GObject* object) {
  auto* self = EE_MODULE_INFO_HOLDER(object);

  self->info_updated.clear();

  G_OBJECT_CLASS(module_info_holder_parent_class)->finalize(object);
}

void module_info_holder_class_init(ModuleInfoHolderClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);

  object_class->finalize = module_info_holder_finalize;
}

void module_info_holder_init(ModuleInfoHolder* self) {
  self->id = SPA_ID_INVALID;

  /*
    gtk is goind something weird when initializing _ModuleInfoHolder
    if we do not do something like the one below we may segfault if info.descrition and similars are empty
   */

  self->name = " ";
  self->description = " ";
  self->filename = " ";
}

auto create(const ModuleInfo& info) -> ModuleInfoHolder* {
  auto* holder = static_cast<ModuleInfoHolder*>(g_object_new(EE_TYPE_MODULE_INFO_HOLDER, nullptr));

  holder->id = info.id;
  holder->name = info.name;
  holder->description = info.description;
  holder->filename = info.filename;

  return holder;
}

}  // namespace ui::holders