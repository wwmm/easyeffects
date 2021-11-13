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
#include "application.hpp"
#include "preset_type.hpp"

namespace ui::presets_menu {

G_BEGIN_DECLS

#define EE_TYPE_PRESETS_MENU (presets_menu_get_type())

G_DECLARE_FINAL_TYPE(PresetsMenu, presets_menu, EE, PRESETS_MENU, GtkPopover)

G_END_DECLS

auto create() -> PresetsMenu*;

void setup(PresetsMenu* self, app::Application* application);

}  // namespace ui::presets_menu

#include <glibmm/i18n.h>

class PresetsMenuUi : public Gtk::Popover {
 public:
  PresetsMenuUi(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& builder,
                Glib::RefPtr<Gio::Settings> refSettings,
                Application* application);
  PresetsMenuUi(const PresetsMenuUi&) = delete;
  auto operator=(const PresetsMenuUi&) -> PresetsMenuUi& = delete;
  PresetsMenuUi(const PresetsMenuUi&&) = delete;
  auto operator=(const PresetsMenuUi&&) -> PresetsMenuUi& = delete;
  ~PresetsMenuUi() override;

  static auto create(Application* app) -> PresetsMenuUi*;

 private:
  inline static const std::string log_tag = "presets_menu_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Application* app = nullptr;

  Gtk::Stack* stack = nullptr;

  Gtk::Button *import_output = nullptr, *import_input = nullptr;

  Gtk::ListView *output_listview = nullptr, *input_listview = nullptr;

  Gtk::ScrolledWindow *output_scrolled_window = nullptr, *input_scrolled_window = nullptr;

  Glib::RefPtr<Gtk::SelectionModel> stack_model;

  Glib::RefPtr<Gtk::StringList> output_string_list, input_string_list;

  std::vector<sigc::connection> connections;

  void import_preset(PresetType preset_type);
};
