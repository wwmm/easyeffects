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

#ifndef RNNOISE_UI_HPP
#define RNNOISE_UI_HPP

#include <glibmm/i18n.h>
#include <filesystem>
#include "glibmm/miscutils.h"
#include "gtkmm/dialog.h"
#include "plugin_ui_base.hpp"

class RNNoiseUi : public Gtk::Grid, public PluginUiBase {
 public:
  RNNoiseUi(BaseObjectType* cobject,
            const Glib::RefPtr<Gtk::Builder>& builder,
            const std::string& schema,
            const std::string& schema_path);
  RNNoiseUi(const RNNoiseUi&) = delete;
  auto operator=(const RNNoiseUi&) -> RNNoiseUi& = delete;
  RNNoiseUi(const RNNoiseUi&&) = delete;
  auto operator=(const RNNoiseUi&&) -> RNNoiseUi& = delete;
  ~RNNoiseUi() override;

  void reset() override;

 private:
  std::string log_tag = "rnnoise_ui: ";

  Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain;

  Gtk::Button* import_model = nullptr;
  Gtk::ListBox* model_listbox = nullptr;

  std::filesystem::path model_dir;

  void on_import_model_clicked();

  static auto on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) -> int;

  void import_model_file(const std::string& file_path);

  void populate_model_listbox();

  auto get_model_names() -> std::vector<std::string>;

  void remove_model_file(const std::string& name);
};

#endif
