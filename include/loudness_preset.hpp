#ifndef LOUDNESS_PRESET_HPP
#define LOUDNESS_PRESET_HPP

#include "plugin_preset_base.hpp"

class LoudnessPreset : public PluginPresetBase {
 public:
  LoudnessPreset();

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
