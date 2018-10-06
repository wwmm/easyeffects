#ifndef PLUGIN_PRESET_BASE_HPP
#define PLUGIN_PRESET_BASE_HPP

#include <giomm/settings.h>
#include <boost/property_tree/ptree.hpp>

class PluginPresetBase {
 public:
  PluginPresetBase() {}
  virtual ~PluginPresetBase() {}

  virtual void write(boost::property_tree::ptree& root) = 0;
  virtual void read(boost::property_tree::ptree& root) = 0;

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
};

#endif
