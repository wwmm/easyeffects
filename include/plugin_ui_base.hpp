#ifndef PLUGIN_UI_BASE_HPP
#define PLUGIN_UI_BASE_HPP

#include <giomm/settings.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
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
#include "preset_type.hpp"

class PluginUiBase {
 public:
  PluginUiBase(const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name);
  PluginUiBase(const PluginUiBase&) = delete;
  auto operator=(const PluginUiBase&) -> PluginUiBase& = delete;
  PluginUiBase(const PluginUiBase&&) = delete;
  auto operator=(const PluginUiBase &&) -> PluginUiBase& = delete;
  virtual ~PluginUiBase();

  std::string name;

  // preset_type to let the UI know which pipeline it belongs to
  // initialized to "output"; to be changed to "input" for soe_ui properties
  PresetType preset_type = PresetType::output;

  Gtk::Box* listbox_control = nullptr;
  Gtk::Button *plugin_up = nullptr, *plugin_down = nullptr;

  void on_new_input_level(const std::array<double, 2>& peak);
  void on_new_output_level(const std::array<double, 2>& peak);
  void on_new_input_level_db(const std::array<double, 2>& peak);
  void on_new_output_level_db(const std::array<double, 2>& peak);
  static auto level_to_str(const double& value, const int& places) -> std::string;

  // reset plugin method
  virtual void reset() = 0;

 protected:
  Glib::RefPtr<Gio::Settings> settings;

  Gtk::Button* reset_button = nullptr;
  Gtk::Switch* enable = nullptr;
  Gtk::Box* controls = nullptr;
  Gtk::Image* img_state = nullptr;

  Gtk::LevelBar *input_level_left = nullptr, *input_level_right = nullptr;
  Gtk::LevelBar *output_level_left = nullptr, *output_level_right = nullptr;
  Gtk::Label *input_level_left_label = nullptr, *input_level_right_label = nullptr;
  Gtk::Label *output_level_left_label = nullptr, *output_level_right_label = nullptr;

  bool input_saturated = false;

  std::vector<sigc::connection> connections;

  static void get_object(const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& name,
                         Glib::RefPtr<Gtk::Adjustment>& object) {
    object = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object(name));
  }

  // reimplemented templates from plugin_preset_base without passing boost ptree
  // using an empty root will rely on default value
  template <typename T>
  auto get_default(const Glib::RefPtr<Gio::Settings>& settings, const std::string& key) -> T {
    Glib::Variant<T> value;

    settings->get_default_value(key, value);

    return value.get();
  }

  template <typename T>
  void update_default_key(const Glib::RefPtr<Gio::Settings>& settings,
                  const std::string& key,
                  const std::string& json_key) {
    boost::property_tree::ptree root;
    Glib::Variant<T> aux;

    settings->get_value(key, aux);

    T current_value = aux.get();

    T new_value = root.get<T>(json_key, get_default<T>(settings, key));

    if (is_different(current_value, new_value)) {
      auto v = Glib::Variant<T>::create(new_value);

      settings->set_value(key, v);
    }
  }

  void update_default_string_key(const Glib::RefPtr<Gio::Settings>& settings,
                         const std::string& key,
                         const std::string& json_key) {
    boost::property_tree::ptree root;
    std::string current_value = settings->get_string(key);

    std::string new_value = root.get<std::string>(json_key, get_default<std::string>(settings, key));

    if (current_value != new_value) {
      settings->set_string(key, new_value);
    }
  }

  template <typename T>
  auto is_different(const T& a, const T& b) -> bool {
    return a != b;
  }

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
