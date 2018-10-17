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

  root.put(section + ".crystalizer.intensity",
           settings->get_double("intensity"));
}

void CrystalizerPreset::load(boost::property_tree::ptree& root,
                             const std::string& section,
                             const Glib::RefPtr<Gio::Settings>& settings) {
  settings->set_boolean("state",
                        root.get<bool>(section + ".crystalizer.state",
                                       get_default<bool>(settings, "state")));

  settings->set_double(
      "input-gain",
      root.get<double>(section + ".crystalizer.input-gain",
                       get_default<double>(settings, "input-gain")));

  settings->set_double(
      "output-gain",
      root.get<double>(section + ".crystalizer.output-gain",
                       get_default<double>(settings, "output-gain")));

  settings->set_double(
      "intensity",
      root.get<double>(section + ".crystalizer.intensity",
                       get_default<double>(settings, "intensity")));
}

void CrystalizerPreset::write(boost::property_tree::ptree& root) {
  save(root, "output", output_settings);
}

void CrystalizerPreset::read(boost::property_tree::ptree& root) {
  load(root, "output", output_settings);
}
