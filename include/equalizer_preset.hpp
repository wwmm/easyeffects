#ifndef EQUALIZER_PRESET_HPP
#define EQUALIZER_PRESET_HPP

#include "plugin_preset_base.hpp"

class EqualizerPreset : public PluginPresetBase {
 public:
  EqualizerPreset();

  void write(PresetType preset_type, boost::property_tree::ptree& root) override;
  void read(PresetType preset_type, boost::property_tree::ptree& root) override;

 private:
  std::string log_tag = "equalizer_preset: ";

  Glib::RefPtr<Gio::Settings> input_settings, input_settings_left, input_settings_right, output_settings,
      output_settings_left, output_settings_right;

  void save(boost::property_tree::ptree& root,
            const std::string& section,
            const Glib::RefPtr<Gio::Settings>& settings) override;
  void load(boost::property_tree::ptree& root,
            const std::string& section,
            const Glib::RefPtr<Gio::Settings>& settings) override;

  void save_channel(boost::property_tree::ptree& root,
                    const std::string& section,
                    const Glib::RefPtr<Gio::Settings>& settings,
                    const int& nbands);

  void load_channel(boost::property_tree::ptree& root,
                    const std::string& section,
                    const Glib::RefPtr<Gio::Settings>& settings,
                    const int& nbands);

  void load_legacy_preset();
};

#endif
