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

#include "node_info_holder.hpp"

namespace ui::holders {

using namespace std::string_literals;

enum { PROP_0, PROP_SERIAL, PROP_ID, PROP_DEVICE_ID, PROP_NAME, PROP_MEDIA_CLASS, PROP_DESCRIPTION, PROP_ICON_NAME };

G_DEFINE_TYPE(NodeInfoHolder, node_info_holder, G_TYPE_OBJECT);

void node_info_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
  auto* self = EE_NODE_INFO_HOLDER(object);

  switch (prop_id) {
    case PROP_SERIAL:
      self->info->serial = g_value_get_uint64(value);
      break;
    case PROP_ID:
      self->info->id = g_value_get_uint(value);
      break;
    case PROP_DEVICE_ID:
      self->info->device_id = g_value_get_uint(value);
      break;
    case PROP_NAME:
      self->info->name = g_value_get_string(value);
      break;
    case PROP_MEDIA_CLASS:
      self->info->media_class = g_value_get_string(value);
      break;
    case PROP_DESCRIPTION:
      self->info->description = g_value_get_string(value);
      break;
    case PROP_ICON_NAME:
      self->icon_name = g_value_get_string(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

void node_info_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
  auto* self = EE_NODE_INFO_HOLDER(object);

  switch (prop_id) {
    case PROP_SERIAL:
      g_value_set_uint64(value, self->info->serial);
      break;
    case PROP_ID:
      g_value_set_uint(value, self->info->id);
      break;
    case PROP_DEVICE_ID:
      g_value_set_uint(value, self->info->device_id);
      break;
    case PROP_NAME:
      g_value_set_string(value, self->info->name.c_str());
      break;
    case PROP_MEDIA_CLASS:
      g_value_set_string(value, self->info->media_class.c_str());
      break;
    case PROP_DESCRIPTION:
      g_value_set_string(value, self->info->description.c_str());
      break;
    case PROP_ICON_NAME:
      g_value_set_string(value, self->icon_name.c_str());
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

void node_info_holder_finalize(GObject* object) {
  auto* self = EE_NODE_INFO_HOLDER(object);

  self->info_updated.clear();

  util::debug(util::to_string(self->info->id) + ", " + self->info->name + " finalized");

  delete self->info;

  G_OBJECT_CLASS(node_info_holder_parent_class)->finalize(object);
}

void node_info_holder_class_init(NodeInfoHolderClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);

  object_class->finalize = node_info_holder_finalize;
  object_class->set_property = node_info_set_property;
  object_class->get_property = node_info_get_property;

  // Using G_MININT64 as minimum parameter causes
  // assertion 'default_value >= minimum && default_value <= maximum' failed
  g_object_class_install_property(object_class, PROP_SERIAL,
                                  g_param_spec_uint64("serial", "Serial", "Serial", SPA_ID_INVALID, G_MAXUINT64,
                                                      SPA_ID_INVALID, G_PARAM_READWRITE));

  g_object_class_install_property(
      object_class, PROP_ID,
      g_param_spec_uint("id", "Id", "Id", G_MININT, G_MAXUINT, SPA_ID_INVALID, G_PARAM_READWRITE));

  g_object_class_install_property(
      object_class, PROP_DEVICE_ID,
      g_param_spec_uint("device-id", "Device Id", "Device Id", G_MININT, G_MAXUINT, SPA_ID_INVALID, G_PARAM_READWRITE));

  g_object_class_install_property(object_class, PROP_NAME,
                                  g_param_spec_string("name", "Name", "Name", nullptr, G_PARAM_READWRITE));

  g_object_class_install_property(
      object_class, PROP_MEDIA_CLASS,
      g_param_spec_string("media-class", "Media Class", "Media Class", nullptr, G_PARAM_READWRITE));

  g_object_class_install_property(
      object_class, PROP_DESCRIPTION,
      g_param_spec_string("description", "Description", "Description", nullptr, G_PARAM_READWRITE));

  g_object_class_install_property(
      object_class, PROP_ICON_NAME,
      g_param_spec_string("icon-name", "Icon Name", "Icon Name", nullptr, G_PARAM_READWRITE));
}

void node_info_holder_init(NodeInfoHolder* self) {
  self->info = new NodeInfo();
}

auto create(const NodeInfo& info) -> NodeInfoHolder* {
  auto* holder = static_cast<NodeInfoHolder*>(g_object_new(EE_TYPE_NODE_INFO_HOLDER, nullptr));

  *holder->info = info;

  if (info.media_class == tags::pipewire::media_class::sink || info.name.starts_with("ee_soe")) {
    holder->icon_name = "audio-speakers-symbolic";
  } else if (info.media_class == tags::pipewire::media_class::source ||
             info.media_class == tags::pipewire::media_class::virtual_source || info.name.starts_with("ee_sie")) {
    holder->icon_name = "audio-input-microphone-symbolic";
  }

  return holder;
}

}  // namespace ui::holders
