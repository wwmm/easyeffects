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

#ifndef STREAM_OUTPUT_EFFECTS_UI_HPP
#define STREAM_OUTPUT_EFFECTS_UI_HPP

#include "effects_base_ui.hpp"
#include "stream_output_effects.hpp"

class StreamOutputEffectsUi : public Gtk::Box, public EffectsBaseUi {
 public:
  StreamOutputEffectsUi(BaseObjectType* cobject,
                        const Glib::RefPtr<Gtk::Builder>& refBuilder,
                        StreamOutputEffects* soe_ptr,
                        const std::string& schema);
  StreamOutputEffectsUi(const StreamOutputEffectsUi&) = delete;
  auto operator=(const StreamOutputEffectsUi&) -> StreamOutputEffectsUi& = delete;
  StreamOutputEffectsUi(const StreamOutputEffectsUi&&) = delete;
  auto operator=(const StreamOutputEffectsUi&&) -> StreamOutputEffectsUi& = delete;
  ~StreamOutputEffectsUi() override;

  static auto add_to_stack(Gtk::Stack* stack, StreamOutputEffects* soe_ptr) -> StreamOutputEffectsUi*;

 protected:
  const std::string log_tag = "soe_ui: ";

 private:
  StreamOutputEffects* soe = nullptr;
};

#endif
