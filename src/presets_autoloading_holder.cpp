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

#include "presets_autoloading_holder.hpp"

namespace ui::holders {

G_DEFINE_TYPE(PresetsAutoloadingHolder, presets_autoloading_holder, G_TYPE_OBJECT);

void presets_autoloading_holder_class_init(PresetsAutoloadingHolderClass* klass) {}

void presets_autoloading_holder_init(PresetsAutoloadingHolder* self) {
  /*
    gtk is doing something weird when initializing the structures "_***"
    if we do not do something like the one below we may segfault if info.device and similar are empty
  */

  self->device = " ";
  self->device_profile = " ";
  self->preset_name = " ";
}

auto create(const std::string& device, const std::string& device_profile, const std::string& preset_name)
    -> PresetsAutoloadingHolder* {
  auto* holder = static_cast<PresetsAutoloadingHolder*>(g_object_new(EE_TYPE_PRESETS_AUTOLOADING_HOLDER, nullptr));

  holder->device = device;
  holder->device_profile = device_profile;
  holder->preset_name = preset_name;

  return holder;
}

}  // namespace ui::holders