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

#pragma once

#include <gtk/gtk.h>
#include "util.hpp"

namespace ui::holders {

G_BEGIN_DECLS

#define EE_TYPE_PRESETS_AUTOLOADING_HOLDER (presets_autoloading_holder_get_type())

G_DECLARE_FINAL_TYPE(PresetsAutoloadingHolder, presets_autoloading_holder, EE, PRESETS_AUTOLOADING_HOLDER, GObject)

G_END_DECLS

struct PresetsAutoLoadingData {
 public:
  std::string device, device_description, device_profile, preset_name;
};

struct _PresetsAutoloadingHolder {
  GObject parent_instance;

  PresetsAutoLoadingData* data;
};

auto create(const std::string& device,
            const std::string& device_description,
            const std::string& device_profile,
            const std::string& preset_name) -> PresetsAutoloadingHolder*;

}  // namespace ui::holders