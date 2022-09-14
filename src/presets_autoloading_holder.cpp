/*
 *  Copyright © 2017-2023 Wellington Wallace
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

#include "presets_autoloading_holder.hpp"

namespace ui::holders {

G_DEFINE_TYPE(PresetsAutoloadingHolder, presets_autoloading_holder, G_TYPE_OBJECT);

void presets_autoloading_holder_finalize(GObject* object) {
  auto* self = EE_PRESETS_AUTOLOADING_HOLDER(object);

  util::debug(self->data->device + ", " + self->data->preset_name + " finalized");

  delete self->data;

  G_OBJECT_CLASS(presets_autoloading_holder_parent_class)->finalize(object);
}

void presets_autoloading_holder_class_init(PresetsAutoloadingHolderClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);

  object_class->finalize = presets_autoloading_holder_finalize;
}

void presets_autoloading_holder_init(PresetsAutoloadingHolder* self) {
  self->data = new PresetsAutoLoadingData();
}

auto create(const std::string& device,
            const std::string& device_description,
            const std::string& device_profile,
            const std::string& preset_name) -> PresetsAutoloadingHolder* {
  auto* holder = static_cast<PresetsAutoloadingHolder*>(g_object_new(EE_TYPE_PRESETS_AUTOLOADING_HOLDER, nullptr));

  holder->data->device = device;
  holder->data->device_description = device_description;
  holder->data->device_profile = device_profile;
  holder->data->preset_name = preset_name;

  return holder;
}

}  // namespace ui::holders