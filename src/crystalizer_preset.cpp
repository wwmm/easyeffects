#include "crystalizer_preset.hpp"

CrystalizerPreset::CrystalizerPreset()
    : output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.crystalizer")) {}

void CrystalizerPreset::save(boost::property_tree::ptree& root,
                             const std::string& section,
                             const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".crystalizer.state", settings->get_boolean("state"));

  root.put(section + ".crystalizer.aggressive", settings->get_boolean("aggressive"));

  root.put(section + ".crystalizer.input-gain", settings->get_double("input-gain"));

  root.put(section + ".crystalizer.output-gain", settings->get_double("output-gain"));

  for (int n = 0; n < 13; n++) {
    root.put(section + ".crystalizer.band" + std::to_string(n) + ".intensity",
             settings->get_double("intensity-band" + std::to_string(n)));

    root.put(section + ".crystalizer.band" + std::to_string(n) + ".mute",
             settings->get_boolean("mute-band" + std::to_string(n)));

    root.put(section + ".crystalizer.band" + std::to_string(n) + ".bypass",
             settings->get_boolean("bypass-band" + std::to_string(n)));
  }
}

void CrystalizerPreset::load(boost::property_tree::ptree& root,
                             const std::string& section,
                             const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".crystalizer.state");

  update_key<bool>(root, settings, "aggressive", section + ".crystalizer.aggressive");

  update_key<double>(root, settings, "input-gain", section + ".crystalizer.input-gain");

  update_key<double>(root, settings, "output-gain", section + ".crystalizer.output-gain");

  for (int n = 0; n < 13; n++) {
    update_key<double>(root, settings, "intensity-band" + std::to_string(n),
                       section + ".crystalizer.band" + std::to_string(n) + ".intensity");

    update_key<bool>(root, settings, "mute-band" + std::to_string(n),
                     section + ".crystalizer.band" + std::to_string(n) + ".mute");

    update_key<bool>(root, settings, "bypass-band" + std::to_string(n),
                     section + ".crystalizer.band" + std::to_string(n) + ".bypass");
  }
}

void CrystalizerPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  }
}

void CrystalizerPreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  }
}
