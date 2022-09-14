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
#include "pipe_manager.hpp"

namespace ui::holders {

G_BEGIN_DECLS

#define EE_TYPE_CLIENT_INFO_HOLDER (client_info_holder_get_type())

G_DECLARE_FINAL_TYPE(ClientInfoHolder, client_info_holder, EE, CLIENT_INFO_HOLDER, GObject)

G_END_DECLS

struct _ClientInfoHolder {
  GObject parent_instance;

  ClientInfo* info;

  sigc::signal<void(const ClientInfo)> info_updated;
};

auto create(const ClientInfo& info) -> ClientInfoHolder*;

}  // namespace ui::holders