#include "autogain_preset.hpp"

AutoGainPreset::AutoGainPreset()
    : output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.autogain")) {}

void AutoGainPreset::save(boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".autogain.state", settings->get_boolean("state"));

  root.put(section + ".autogain.detect-silence", settings->get_boolean("detect-silence"));

  root.put(section + ".autogain.input-gain", settings->get_double("input-gain"));

  root.put(section + ".autogain.output-gain", settings->get_double("output-gain"));

  root.put(section + ".autogain.target", settings->get_double("target"));

  root.put(section + ".autogain.weight-m", settings->get_int("weight-m"));

  root.put(section + ".autogain.weight-s", settings->get_int("weight-s"));

  root.put(section + ".autogain.weight-i", settings->get_int("weight-i"));
}

void AutoGainPreset::load(boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".autogain.state");

  update_key<bool>(root, settings, "detect-silence", section + ".autogain.detect-silence");

  update_key<double>(root, settings, "input-gain", section + ".autogain.input-gain");

  update_key<double>(root, settings, "output-gain", section + ".autogain.output-gain");

  update_key<double>(root, settings, "target", section + ".autogain.target");

  update_key<int>(root, settings, "weight-m", section + ".autogain.weight-m");

  update_key<int>(root, settings, "weight-s", section + ".autogain.weight-s");

  update_key<int>(root, settings, "weight-i", section + ".autogain.weight-i");
}

void AutoGainPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  }
}

void AutoGainPreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  }
}
