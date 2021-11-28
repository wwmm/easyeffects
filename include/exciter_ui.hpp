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

namespace ui::exciter_box {

G_BEGIN_DECLS

#define EE_TYPE_EXCITER_BOX (exciter_box_get_type())

G_DECLARE_FINAL_TYPE(ExciterBox, exciter_box, EE, EXCITER_BOX, GtkBox)

G_END_DECLS

auto create() -> ExciterBox*;

void setup(ExciterBox* self, std::shared_ptr<Exciter> exciter, const std::string& schema_path);

}  // namespace ui::exciter_box

#include "plugin_ui_base.hpp"

class ExciterUi : public Gtk::Box, public PluginUiBase {
 public:
  ExciterUi(BaseObjectType* cobject,
            const Glib::RefPtr<Gtk::Builder>& builder,
            const std::string& schema,
            const std::string& schema_path);
  ExciterUi(const ExciterUi&) = delete;
  auto operator=(const ExciterUi&) -> ExciterUi& = delete;
  ExciterUi(const ExciterUi&&) = delete;
  auto operator=(const ExciterUi&&) -> ExciterUi& = delete;
  ~ExciterUi() override;

  static auto add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> ExciterUi*;

  void on_new_harmonics_level(const double& value);

  void reset() override;

 private:
  Gtk::LevelBar* harmonics_levelbar = nullptr;

  Gtk::Label* harmonics_levelbar_label = nullptr;

  Gtk::SpinButton *ceil = nullptr, *amount = nullptr, *harmonics = nullptr, *scope = nullptr;

  Gtk::Scale* blend = nullptr;

  Gtk::ToggleButton *ceil_active = nullptr, *listen = nullptr;
};
