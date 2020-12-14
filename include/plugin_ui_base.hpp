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

#ifndef PLUGIN_UI_BASE_HPP
#define PLUGIN_UI_BASE_HPP

#include <giomm/settings.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/dialog.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/filechoosernative.h>
#include <gtkmm/frame.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/levelbar.h>
#include <gtkmm/listbox.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/scale.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/stack.h>
#include <gtkmm/stackswitcher.h>
#include <gtkmm/switch.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/window.h>
#include <array>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "util.hpp"

class PluginUiBase {
 public:
  PluginUiBase(const Glib::RefPtr<Gtk::Builder>& builder, const std::string& schema, const std::string& schema_path);
  PluginUiBase(const PluginUiBase&) = delete;
  auto operator=(const PluginUiBase&) -> PluginUiBase& = delete;
  PluginUiBase(const PluginUiBase&&) = delete;
  auto operator=(const PluginUiBase &&) -> PluginUiBase& = delete;
  virtual ~PluginUiBase();

  std::string name;

  Gtk::Box* listbox_control = nullptr;
  Gtk::Button *plugin_up = nullptr, *plugin_down = nullptr;

  void on_new_input_level(const std::array<double, 2>& peak);
  void on_new_output_level(const std::array<double, 2>& peak);
  void on_new_input_level_db(const std::array<double, 2>& peak);
  void on_new_output_level_db(const std::array<double, 2>& peak);

  auto level_to_localized_string(const double& value, const int& places) -> std::string;
  auto level_to_localized_string(const float& value, const int& places) -> std::string;
  auto string_to_float(const std::string& value) -> float;

  // reset plugin method
  virtual void reset() = 0;

 protected:
  Glib::RefPtr<Gio::Settings> settings;

  Gtk::Button* reset_button = nullptr;
  Gtk::CheckButton* enable = nullptr;
  Gtk::Box* controls = nullptr;

  Gtk::LevelBar *input_level_left = nullptr, *input_level_right = nullptr;
  Gtk::LevelBar *output_level_left = nullptr, *output_level_right = nullptr;
  Gtk::Label *input_level_left_label = nullptr, *input_level_right_label = nullptr;
  Gtk::Label *output_level_left_label = nullptr, *output_level_right_label = nullptr;

  std::vector<sigc::connection> connections;

  static void get_object(const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& name,
                         Glib::RefPtr<Gtk::Adjustment>& object) {
    object = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object(name));
  }

  template <typename T>
  auto level_to_localized_string_showpos(const T& value, const int& places) -> std::string {
    std::ostringstream msg;

    msg.imbue(global_locale);
    msg.precision(places);

    msg << ((value > 0.0) ? "+" : "") << std::fixed << value;

    return msg.str();
  }

 private:
  std::locale global_locale;
  std::locale c_locale = std::locale();

  template <typename T1, typename T2, typename T3, typename T4>
  void update_level(const T1& w_left,
                    const T2& w_left_label,
                    const T3& w_right,
                    const T4& w_right_label,
                    const std::array<double, 2>& peak) {
    auto left = peak[0];
    auto right = peak[1];
    auto left_db = util::linear_to_db(left);
    auto right_db = util::linear_to_db(right);

    if (left_db >= -99.0) {
      w_left->set_value(left);
      w_left_label->set_text(level_to_localized_string(left_db, 0));
    } else {
      w_left->set_value(0.0);
      w_left_label->set_text("-99");
    }

    if (right_db >= -99.0) {
      w_right->set_value(right);
      w_right_label->set_text(level_to_localized_string(right_db, 0));
    } else {
      w_right->set_value(0.0);
      w_right_label->set_text("-99");
    }
  }

  template <typename T1, typename T2, typename T3, typename T4>
  void update_level_db(const T1& w_left,
                       const T2& w_left_label,
                       const T3& w_right,
                       const T4& w_right_label,
                       const std::array<double, 2>& peak) {
    auto left = peak[0];
    auto right = peak[1];

    if (left >= -99.0) {
      auto db_value = util::db_to_linear(left);

      if (db_value < 0.0) {
        db_value = 0.0;
      } else if (db_value > 1.0) {
        db_value = 1.0;
      }

      w_left->set_value(db_value);
      w_left_label->set_text(level_to_localized_string(left, 0));
    } else {
      w_left->set_value(0.0);
      w_left_label->set_text("-99");
    }

    if (right >= -99.0) {
      auto db_value = util::db_to_linear(right);

      if (db_value < 0.0) {
        db_value = 0.0;
      } else if (db_value > 1.0) {
        db_value = 1.0;
      }

      w_right->set_value(db_value);
      w_right_label->set_text(level_to_localized_string(right, 0));
    } else {
      w_right->set_value(0.0);
      w_right_label->set_text("-99");
    }
  }
};

#endif
