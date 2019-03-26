#ifndef PLUGIN_PRESET_BASE_HPP
#define PLUGIN_PRESET_BASE_HPP

#include <giomm/settings.h>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include "preset_type.hpp"

class PluginPresetBase {
 public:
  PluginPresetBase() {}
  virtual ~PluginPresetBase() {}

  virtual void write(PresetType preset_type,
                     boost::property_tree::ptree& root) = 0;
  virtual void read(PresetType preset_type,
                    boost::property_tree::ptree& root) = 0;

 protected:
  virtual void save(boost::property_tree::ptree& root,
                    const std::string& section,
                    const Glib::RefPtr<Gio::Settings>& settings) = 0;
  virtual void load(boost::property_tree::ptree& root,
                    const std::string& section,
                    const Glib::RefPtr<Gio::Settings>& settings) = 0;

  template <typename T>
  T get_default(const Glib::RefPtr<Gio::Settings>& settings,
                const std::string& key) {
    Glib::Variant<T> value;

    settings->get_default_value(key, value);

    return value.get();
  }

  template <typename T>
  void update_key(boost::property_tree::ptree& root,
                  const Glib::RefPtr<Gio::Settings>& settings,
                  const std::string& key,
                  const std::string& json_key) {
    Glib::Variant<T> aux;

    settings->get_value(key, aux);

    T current_value = aux.get();

    T new_value = root.get<T>(json_key, get_default<T>(settings, key));

    if (is_different(current_value, new_value)) {
      auto v = Glib::Variant<T>::create(new_value);

      settings->set_value(key, v);
    }
  }

  void update_string_key(boost::property_tree::ptree& root,
                         const Glib::RefPtr<Gio::Settings>& settings,
                         const std::string& key,
                         const std::string& json_key) {
    std::string current_value = settings->get_string(key);

    std::string new_value = root.get<std::string>(
        json_key, get_default<std::string>(settings, key));

    if (current_value != new_value) {
      settings->set_string(key, new_value);
    }
  }

 private:
  template <typename T>
  bool is_different(const T& a, const T& b) {
    return a != b;
  }
};

#endif
