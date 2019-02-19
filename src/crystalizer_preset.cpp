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

  root.put(section + ".crystalizer.freq1", settings->get_double("freq1"));

  root.put(section + ".crystalizer.freq2", settings->get_double("freq2"));

  root.put(section + ".crystalizer.freq3", settings->get_double("freq3"));

  root.put(section + ".crystalizer.intensity-band0",
           settings->get_double("intensity-band0"));

  root.put(section + ".crystalizer.intensity-band1",
           settings->get_double("intensity-band1"));

  root.put(section + ".crystalizer.intensity-band2",
           settings->get_double("intensity-band2"));

  root.put(section + ".crystalizer.intensity-band3",
           settings->get_double("intensity-band3"));

  root.put(section + ".crystalizer.mute-band0",
           settings->get_boolean("mute-band0"));

  root.put(section + ".crystalizer.mute-band1",
           settings->get_boolean("mute-band1"));

  root.put(section + ".crystalizer.mute-band2",
           settings->get_boolean("mute-band2"));

  root.put(section + ".crystalizer.mute-band3",
           settings->get_boolean("mute-band3"));
}

void CrystalizerPreset::load(boost::property_tree::ptree& root,
                             const std::string& section,
                             const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".crystalizer.state");

  update_key<double>(root, settings, "input-gain",
                     section + ".crystalizer.input-gain");

  update_key<double>(root, settings, "output-gain",
                     section + ".crystalizer.output-gain");

  update_key<double>(root, settings, "freq1", section + ".crystalizer.freq1");

  update_key<double>(root, settings, "freq2", section + ".crystalizer.freq2");

  update_key<double>(root, settings, "freq3", section + ".crystalizer.freq3");

  update_key<double>(root, settings, "intensity-band0",
                     section + ".crystalizer.intensity-band0");

  update_key<double>(root, settings, "intensity-band1",
                     section + ".crystalizer.intensity-band1");

  update_key<double>(root, settings, "intensity-band2",
                     section + ".crystalizer.intensity-band2");

  update_key<double>(root, settings, "intensity-band3",
                     section + ".crystalizer.intensity-band3");

  update_key<bool>(root, settings, "mute-band0",
                   section + ".crystalizer.mute-band0");

  update_key<bool>(root, settings, "mute-band1",
                   section + ".crystalizer.mute-band1");

  update_key<bool>(root, settings, "mute-band2",
                   section + ".crystalizer.mute-band2");

  update_key<bool>(root, settings, "mute-band3",
                   section + ".crystalizer.mute-band3");
}

void CrystalizerPreset::write(PresetType preset_type,
                              boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  }
}

void CrystalizerPreset::read(PresetType preset_type,
                             boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  }
}
