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

  root.put(section + ".crystalizer.split-frequency",
           settings->get_double("split-frequency"));

  root.put(section + ".crystalizer.intensity-low",
           settings->get_double("intensity-low"));

  root.put(section + ".crystalizer.intensity-mid",
           settings->get_double("intensity-mid"));

  root.put(section + ".crystalizer.intensity-high",
           settings->get_double("intensity-high"));
}

void CrystalizerPreset::load(boost::property_tree::ptree& root,
                             const std::string& section,
                             const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".crystalizer.state");

  update_key<double>(root, settings, "input-gain",
                     section + ".crystalizer.input-gain");

  update_key<double>(root, settings, "output-gain",
                     section + ".crystalizer.output-gain");

  update_key<double>(root, settings, "split-frequency",
                     section + ".crystalizer.split-frequency");

  update_key<double>(root, settings, "intensity-low",
                     section + ".crystalizer.intensity-low");

  update_key<double>(root, settings, "intensity-mid",
                     section + ".crystalizer.intensity-mid");

  update_key<double>(root, settings, "intensity-high",
                     section + ".crystalizer.intensity-high");
}

void CrystalizerPreset::write(boost::property_tree::ptree& root) {
  save(root, "output", output_settings);
}

void CrystalizerPreset::read(boost::property_tree::ptree& root) {
  load(root, "output", output_settings);
}
