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

#include "client_info_holder.hpp"

namespace ui::holders {

enum { PROP_0, PROP_ID, PROP_NAME, PROP_API, PROP_ACCESS };

G_DEFINE_TYPE(ClientInfoHolder, client_info_holder, G_TYPE_OBJECT);

void client_info_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
  auto* self = EE_CLIENT_INFO_HOLDER(object);

  switch (prop_id) {
    case PROP_ID:
      self->id = g_value_get_uint(value);
      break;
    case PROP_NAME:
      self->name = g_value_get_string(value);
      break;
    case PROP_API:
      self->api = g_value_get_string(value);
      break;
    case PROP_ACCESS:
      self->access = g_value_get_string(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

void client_info_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
  auto* self = EE_CLIENT_INFO_HOLDER(object);

  switch (prop_id) {
    case PROP_ID:
      g_value_set_uint(value, self->id);
      break;
    case PROP_NAME:
      g_value_set_string(value, self->name.c_str());
      break;
    case PROP_API:
      g_value_set_string(value, self->api.c_str());
      break;
    case PROP_ACCESS:
      g_value_set_string(value, self->access.c_str());
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

void client_info_holder_finalize(GObject* object) {
  auto* self = EE_CLIENT_INFO_HOLDER(object);

  self->info_updated.clear();

  G_OBJECT_CLASS(client_info_holder_parent_class)->finalize(object);
}

void client_info_holder_class_init(ClientInfoHolderClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);

  object_class->finalize = client_info_holder_finalize;
  object_class->set_property = client_info_set_property;
  object_class->get_property = client_info_get_property;

  g_object_class_install_property(
      object_class, PROP_ID,
      g_param_spec_uint("id", "Id", "Id", G_MININT, G_MAXUINT, SPA_ID_INVALID, G_PARAM_READWRITE));

  g_object_class_install_property(object_class, PROP_NAME,
                                  g_param_spec_string("name", "Name", "Name", nullptr, G_PARAM_READWRITE));

  g_object_class_install_property(object_class, PROP_API,
                                  g_param_spec_string("api", "API", "API", nullptr, G_PARAM_READWRITE));

  g_object_class_install_property(object_class, PROP_ACCESS,
                                  g_param_spec_string("access", "Access", "Access", nullptr, G_PARAM_READWRITE));
}

void client_info_holder_init(ClientInfoHolder* self) {
  self->id = SPA_ID_INVALID;

  /*
    gtk is doing something weird when initializing the structures "_***"
    if we do not do something like the one below we may segfault if info.name and similar are empty
  */

  self->name = " ";
  self->api = " ";
  self->access = " ";
}

auto create(const ClientInfo& info) -> ClientInfoHolder* {
  auto* holder = static_cast<ClientInfoHolder*>(g_object_new(EE_TYPE_CLIENT_INFO_HOLDER, nullptr));

  holder->id = info.id;
  holder->name = info.name;
  holder->api = info.api;
  holder->access = info.access;

  return holder;
}

}  // namespace ui::holders