#include "loudness_preset.hpp"

LoudnessPreset::LoudnessPreset()
    : output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.loudness")) {}

void LoudnessPreset::save(boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".loudness.state", settings->get_boolean("state"));

  root.put(section + ".loudness.loudness", settings->get_double("loudness"));

  root.put(section + ".loudness.output", settings->get_double("output"));

  root.put(section + ".loudness.link", settings->get_double("link"));
}

void LoudnessPreset::load(boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".loudness.state");

  update_key<double>(root, settings, "loudness", section + ".loudness.loudness");

  update_key<double>(root, settings, "output", section + ".loudness.output");

  update_key<double>(root, settings, "link", section + ".loudness.link");
}

void LoudnessPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  }
}

void LoudnessPreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  }
}
