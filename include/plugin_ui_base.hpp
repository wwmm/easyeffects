#ifndef PLUGIN_UI_BASE_HPP
#define PLUGIN_UI_BASE_HPP

#include <giomm/settings.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/levelbar.h>
#include <gtkmm/switch.h>
#include <array>
#include "util.hpp"

class PluginUiBase {
 public:
  PluginUiBase(const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name);
  virtual ~PluginUiBase();

  std::string name;

  Gtk::Box* listbox_control;
  Gtk::Button *plugin_up, *plugin_down;

  void on_new_input_level(const std::array<double, 2>& peak);
  void on_new_output_level(const std::array<double, 2>& peak);
  void on_new_input_level_db(const std::array<double, 2>& peak);
  void on_new_output_level_db(const std::array<double, 2>& peak);

 protected:
  Glib::RefPtr<Gio::Settings> settings;

  Gtk::Switch* enable;
  Gtk::Box* controls;
  Gtk::Image* img_state;

  Gtk::LevelBar *input_level_left, *input_level_right;
  Gtk::LevelBar *output_level_left, *output_level_right;
  Gtk::Label *input_level_left_label, *input_level_right_label;
  Gtk::Label *output_level_left_label, *output_level_right_label;

  bool input_saturated = false;

  std::vector<sigc::connection> connections;

  void get_object(const Glib::RefPtr<Gtk::Builder>& builder,
                  const std::string& name,
                  Glib::RefPtr<Gtk::Adjustment>& object) {
    object = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object(name));
  }

  std::string level_to_str(const double& value, const int& places);

 private:
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

    if (left_db >= -99) {
      w_left->set_value(left);
      w_left_label->set_text(level_to_str(left_db, 0));
    } else {
      w_left->set_value(0);
      w_left_label->set_text("-99");
    }

    if (right_db >= -99) {
      w_right->set_value(right);
      w_right_label->set_text(level_to_str(right_db, 0));
    } else {
      w_right->set_value(0);
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

    if (left >= -99) {
      auto db_value = util::db_to_linear(left);

      if (db_value < 0) {
        db_value = 0;
      } else if (db_value > 1) {
        db_value = 1;
      }

      w_left->set_value(db_value);
      w_left_label->set_text(level_to_str(left, 0));
    } else {
      w_left->set_value(0);
      w_left_label->set_text("-99");
    }

    if (right >= -99) {
      auto db_value = util::db_to_linear(right);

      if (db_value < 0) {
        db_value = 0;
      } else if (db_value > 1) {
        db_value = 1;
      }

      w_right->set_value(db_value);
      w_right_label->set_text(level_to_str(right, 0));
    } else {
      w_right->set_value(0);
      w_right_label->set_text("-99");
    }
  }
};

#endif
