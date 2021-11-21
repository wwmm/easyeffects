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

#include "node_info_holder.hpp"

namespace ui::holders {

G_DEFINE_TYPE(NodeInfoHolder, node_info_holder, G_TYPE_OBJECT);

void node_info_holder_finalize(GObject* object) {
  auto* self = EE_NODE_INFO_HOLDER(object);

  self->info_updated.clear();

  G_OBJECT_CLASS(node_info_holder_parent_class)->finalize(object);
}

void node_info_holder_class_init(NodeInfoHolderClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);

  object_class->finalize = node_info_holder_finalize;
}

void node_info_holder_init(NodeInfoHolder* self) {
  self->id = SPA_ID_INVALID;
  self->device_id = SPA_ID_INVALID;

  /*
    gtk is doing something weird when initializing the structures "_***"
    if we do not do something like the one below we may segfault if info.name and similar are empty
  */

  self->name = " ";
  self->media_class = " ";
}

auto create(const NodeInfo& info) -> NodeInfoHolder* {
  auto* holder = static_cast<NodeInfoHolder*>(g_object_new(EE_TYPE_NODE_INFO_HOLDER, nullptr));

  holder->ts = info.timestamp;
  holder->id = info.id;
  holder->device_id = info.device_id;
  holder->name = info.name;
  holder->media_class = info.media_class;

  return holder;
}

}  // namespace ui::holders