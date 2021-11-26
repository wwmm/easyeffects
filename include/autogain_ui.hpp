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

#include <adwaita.h>
#include "effects_base.hpp"
#include "ui_helpers.hpp"

namespace ui::autogain_box {

G_BEGIN_DECLS

#define EE_TYPE_AUTOGAIN_BOX (autogain_box_get_type())

G_DECLARE_FINAL_TYPE(AutogainBox, autogain_box, EE, AUTOGAIN_BOX, GtkBox)

G_END_DECLS

auto create() -> AutogainBox*;

void setup(AutogainBox* self, std::shared_ptr<AutoGain> autogain, const std::string& schema_path);

}  // namespace ui::autogain_box

#include "plugin_ui_base.hpp"

class AutoGainUi : public Gtk::Box, public PluginUiBase {
 public:
  AutoGainUi(BaseObjectType* cobject,
             const Glib::RefPtr<Gtk::Builder>& builder,
             const std::string& schema,
             const std::string& schema_path);
  AutoGainUi(const AutoGainUi&) = delete;
  auto operator=(const AutoGainUi&) -> AutoGainUi& = delete;
  AutoGainUi(const AutoGainUi&&) = delete;
  auto operator=(const AutoGainUi&&) -> AutoGainUi& = delete;
  ~AutoGainUi() override;

  static auto add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> AutoGainUi*;

  void reset() override;
};
