/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef STREAM_INPUT_EFFECTS_UI_HPP
#define STREAM_INPUT_EFFECTS_UI_HPP

#include "effects_base_ui.hpp"
#include "stream_input_effects.hpp"

class StreamInputEffectsUi : public Gtk::Box, public EffectsBaseUi {
 public:
  StreamInputEffectsUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& refBuilder,
                       StreamInputEffects* sie_ptr,
                       const std::string& schema);
  StreamInputEffectsUi(const StreamInputEffectsUi&) = delete;
  auto operator=(const StreamInputEffectsUi&) -> StreamInputEffectsUi& = delete;
  StreamInputEffectsUi(const StreamInputEffectsUi&&) = delete;
  auto operator=(const StreamInputEffectsUi&&) -> StreamInputEffectsUi& = delete;
  ~StreamInputEffectsUi() override;

  static auto add_to_stack(Gtk::Stack* stack, StreamInputEffects* sie_ptr) -> StreamInputEffectsUi*;

 protected:
  std::string log_tag = "sie_ui: ";

 private:
  StreamInputEffects* sie = nullptr;

  void level_meters_connections();
  void up_down_connections();
};

#endif
