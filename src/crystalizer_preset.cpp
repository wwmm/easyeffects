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

  root.put(section + ".crystalizer.intensity-lower",
           settings->get_double("intensity-lower"));

  root.put(section + ".crystalizer.intensity-higher",
           settings->get_double("intensity-higher"));
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

  update_key<double>(root, settings, "intensity-lower",
                     section + ".crystalizer.intensity-lower");

  update_key<double>(root, settings, "intensity-higher",
                     section + ".crystalizer.intensity-higher");
}

void CrystalizerPreset::write(boost::property_tree::ptree& root) {
  save(root, "output", output_settings);
}

void CrystalizerPreset::read(boost::property_tree::ptree& root) {
  load(root, "output", output_settings);
}
