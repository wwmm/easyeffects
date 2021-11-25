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

#pragma once

#include "pipe_manager.hpp"

// we had issues with holders, therefore for safety we pass arguments as value

class NodeInfoHolder : public Glib::Object {
 public:
  long ts;

  uint id = SPA_ID_INVALID;

  uint device_id = SPA_ID_INVALID;

  std::string name;

  std::string media_class;

  static auto create(NodeInfo info) -> Glib::RefPtr<NodeInfoHolder>;

  sigc::signal<void(const NodeInfo)> info_updated;

 protected:
  NodeInfoHolder(NodeInfo info);
};
