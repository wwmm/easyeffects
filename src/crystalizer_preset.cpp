#include "crystalizer_preset.hpp"

CrystalizerPreset::CrystalizerPreset()
    : output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.crystalizer")) {}

void CrystalizerPreset::save(boost::property_tree::ptree& root,
                             const std::string& section,
                             const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".crystalizer.state", settings->get_boolean("state"));

  root.put(section + ".crystalizer.input-gain",
           settings->get_double("input-gain"));

  root.put(section + ".crystalizer.output-gain",
           settings->get_double("output-gain"));

  root.put(section + ".crystalizer.intensity-low",
           settings->get_double("intensity-low"));

  root.put(section + ".crystalizer.intensity-mid",
           settings->get_double("intensity-mid"));

  root.put(section + ".crystalizer.intensity-high",
           settings->get_double("intensity-high"));

  root.put(section + ".crystalizer.mute-low",
           settings->get_boolean("mute-low"));

  root.put(section + ".crystalizer.mute-mid",
           settings->get_boolean("mute-mid"));

  root.put(section + ".crystalizer.mute-high",
           settings->get_boolean("mute-high"));

  root.put(section + ".crystalizer.volume-low",
           settings->get_double("volume-low"));

  root.put(section + ".crystalizer.volume-mid",
           settings->get_double("volume-mid"));

  root.put(section + ".crystalizer.volume-high",
           settings->get_double("volume-high"));
}

void CrystalizerPreset::load(boost::property_tree::ptree& root,
                             const std::string& section,
                             const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".crystalizer.state");

  update_key<double>(root, settings, "input-gain",
                     section + ".crystalizer.input-gain");

  update_key<double>(root, settings, "output-gain",
                     section + ".crystalizer.output-gain");

  update_key<double>(root, settings, "intensity-low",
                     section + ".crystalizer.intensity-low");

  update_key<double>(root, settings, "intensity-mid",
                     section + ".crystalizer.intensity-mid");

  update_key<double>(root, settings, "intensity-high",
                     section + ".crystalizer.intensity-high");

  update_key<bool>(root, settings, "mute-low",
                   section + ".crystalizer.mute-low");

  update_key<bool>(root, settings, "mute-mid",
                   section + ".crystalizer.mute-mid");

  update_key<bool>(root, settings, "mute-high",
                   section + ".crystalizer.mute-high");

  update_key<double>(root, settings, "volume-low",
                     section + ".crystalizer.volume-low");

  update_key<double>(root, settings, "volume-mid",
                     section + ".crystalizer.volume-mid");

  update_key<double>(root, settings, "volume-high",
                     section + ".crystalizer.volume-high");
}

void CrystalizerPreset::write(PresetType preset_type,
                              boost::property_tree::ptree& root) {
  save(root, "output", output_settings);
}

void CrystalizerPreset::read(PresetType preset_type,
                             boost::property_tree::ptree& root) {
  load(root, "output", output_settings);
}
