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
#include "plugin_ui_base.hpp"

class RNNoiseUi : public Gtk::Box, public PluginUiBase {
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

  static auto add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> RNNoiseUi*;

  void reset() override;

 private:
  std::string log_tag = "rnnoise_ui: ";
  Glib::ustring default_model_name;

  Gtk::Scale *input_gain = nullptr, *output_gain = nullptr;

  Gtk::Button* import_model = nullptr;

  Gtk::Frame* model_list_frame = nullptr;

  Gtk::ListView* listview = nullptr;

  Gtk::Label* active_model_name = nullptr;

  Glib::RefPtr<Gtk::StringList> string_list;

  std::filesystem::path model_dir;

  void setup_listview();

  void on_import_model_clicked();

  void import_model_file(const std::string& file_path);

  auto get_model_names() -> std::vector<std::string>;

  void remove_model_file(const std::string& name);

  void set_active_model_label();
};

#endif
