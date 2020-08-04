#include "loudness_preset.hpp"

LoudnessPreset::LoudnessPreset()
    : output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.loudness",
                                            "/com/github/wwmm/pulseeffects/sinkinputs/loudness/")) {}

void LoudnessPreset::save(boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".loudness.state", settings->get_boolean("state"));

  root.put(section + ".loudness.fft", settings->get_string("fft"));

  root.put(section + ".loudness.std", settings->get_string("std"));

  root.put(section + ".loudness.volume", settings->get_double("volume"));
}

void LoudnessPreset::load(const boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".loudness.state");

  update_string_key(root, settings, "fft", section + ".loudness.fft");

  update_string_key(root, settings, "std", section + ".loudness.std");

  update_key<double>(root, settings, "volume", section + ".loudness.volume");
}

void LoudnessPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  }
}

void LoudnessPreset::read(PresetType preset_type, const boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  }
}
