#ifndef DELAY_PRESET_HPP
#define DELAY_PRESET_HPP

#include "plugin_preset_base.hpp"

class DelayPreset : public PluginPresetBase {
 public:
  DelayPreset();

  void write(PresetType preset_type, boost::property_tree::ptree& root) override;
  void read(PresetType preset_type, boost::property_tree::ptree& root) override;

 private:
  Glib::RefPtr<Gio::Settings> output_settings;

  void save(boost::property_tree::ptree& root,
            const std::string& section,
            const Glib::RefPtr<Gio::Settings>& settings) override;
  void load(boost::property_tree::ptree& root,
            const std::string& section,
            const Glib::RefPtr<Gio::Settings>& settings) override;
};

#endif
