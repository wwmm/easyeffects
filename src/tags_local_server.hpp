/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

namespace tags::local_server {

inline constexpr auto server_name = "EasyEffectsServer";

inline constexpr auto quit_app = "quit_app\n";

inline constexpr auto show_window = "show_window\n";

inline constexpr auto hide_window = "hide_window\n";

inline constexpr auto load_preset = "load_preset";

inline constexpr auto global_bypass = "global_bypass";

inline constexpr auto get_global_bypass = "get_global_bypass\n";

inline constexpr auto set_property = "set_property";

inline constexpr auto get_property = "get_property";

inline constexpr auto get_last_loaded_preset = "get_last_loaded_preset";

}  // namespace tags::local_server
