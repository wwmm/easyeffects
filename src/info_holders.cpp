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

#include "info_holders.hpp"

#include <utility>

NodeInfoHolder::NodeInfoHolder(NodeInfo info)
    : Glib::ObjectBase(typeid(NodeInfoHolder)),
      Glib::Object(),
      ts(info.timestamp),
      id(info.id),
      device_id(info.device_id),
      name(info.name),
      media_class(info.media_class) {}

auto NodeInfoHolder::create(NodeInfo info) -> Glib::RefPtr<NodeInfoHolder> {
  return Glib::make_refptr_for_instance<NodeInfoHolder>(new NodeInfoHolder(std::move(info)));
}
