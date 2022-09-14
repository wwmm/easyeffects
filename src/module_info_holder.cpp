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

#include "module_info_holder.hpp"

namespace ui::holders {

enum { PROP_0, PROP_ID, PROP_NAME, PROP_DESCRIPTION, PROP_FILENAME };

G_DEFINE_TYPE(ModuleInfoHolder, module_info_holder, G_TYPE_OBJECT);

void module_info_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
  auto* self = EE_MODULE_INFO_HOLDER(object);

  switch (prop_id) {
    case PROP_ID:
      self->info->id = g_value_get_uint(value);
      break;
    case PROP_NAME:
      self->info->name = g_value_get_string(value);
      break;
    case PROP_DESCRIPTION:
      self->info->description = g_value_get_string(value);
      break;
    case PROP_FILENAME:
      self->info->filename = g_value_get_string(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

void module_info_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
  auto* self = EE_MODULE_INFO_HOLDER(object);

  switch (prop_id) {
    case PROP_ID:
      g_value_set_uint(value, self->info->id);
      break;
    case PROP_NAME:
      g_value_set_string(value, self->info->name.c_str());
      break;
    case PROP_DESCRIPTION:
      g_value_set_string(value, self->info->description.c_str());
      break;
    case PROP_FILENAME:
      g_value_set_string(value, self->info->filename.c_str());
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

void module_info_holder_finalize(GObject* object) {
  auto* self = EE_MODULE_INFO_HOLDER(object);

  self->info_updated.clear();

  util::debug(util::to_string(self->info->id) + ", " + self->info->name + " finalized");

  delete self->info;

  G_OBJECT_CLASS(module_info_holder_parent_class)->finalize(object);
}

void module_info_holder_class_init(ModuleInfoHolderClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);

  object_class->finalize = module_info_holder_finalize;
  object_class->set_property = module_info_set_property;
  object_class->get_property = module_info_get_property;

  g_object_class_install_property(
      object_class, PROP_ID,
      g_param_spec_uint("id", "Id", "Id", G_MININT, G_MAXUINT, SPA_ID_INVALID, G_PARAM_READWRITE));

  g_object_class_install_property(object_class, PROP_NAME,
                                  g_param_spec_string("name", "Name", "Name", nullptr, G_PARAM_READWRITE));

  g_object_class_install_property(
      object_class, PROP_DESCRIPTION,
      g_param_spec_string("description", "Description", "Description", nullptr, G_PARAM_READWRITE));

  g_object_class_install_property(
      object_class, PROP_FILENAME,
      g_param_spec_string("file-name", "File Name", "File Name", nullptr, G_PARAM_READWRITE));
}

void module_info_holder_init(ModuleInfoHolder* self) {
  self->info = new ModuleInfo();
}

auto create(const ModuleInfo& info) -> ModuleInfoHolder* {
  auto* holder = static_cast<ModuleInfoHolder*>(g_object_new(EE_TYPE_MODULE_INFO_HOLDER, nullptr));

  holder->info->id = info.id;
  holder->info->name = info.name;
  holder->info->description = info.description;
  holder->info->filename = info.filename;

  return holder;
}

}  // namespace ui::holders
