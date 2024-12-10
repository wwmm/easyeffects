/*
 *  Copyright © 2017-2024 Wellington Wallace
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

#include <QString>
#include "effects_base.hpp"
#include "pw_manager.hpp"
#include "pw_objects.hpp"

class StreamInputEffects : public EffectsBase {
 public:
  StreamInputEffects(pw::Manager* pipe_manager);
  StreamInputEffects(const StreamInputEffects&) = delete;
  auto operator=(const StreamInputEffects&) -> StreamInputEffects& = delete;
  StreamInputEffects(const StreamInputEffects&&) = delete;
  auto operator=(const StreamInputEffects&&) -> StreamInputEffects& = delete;
  ~StreamInputEffects() override = default;

  void set_bypass(const bool& state);

  void set_listen_to_mic(const bool& state);

 private:
  bool bypass = false;

  void connect_filters(const bool& bypass = false);

  void disconnect_filters();

  auto apps_want_to_play() -> bool;

  void on_link_changed(pw::LinkInfo link_info);
};