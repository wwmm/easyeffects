#include "maximizer_preset.hpp"

MaximizerPreset::MaximizerPreset()
    : output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.maximizer")) {}

void MaximizerPreset::save(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".maximizer.state", settings->get_boolean("state"));

  root.put(section + ".maximizer.release", settings->get_double("release"));

  root.put(section + ".maximizer.ceiling", settings->get_double("ceiling"));

  root.put(section + ".maximizer.threshold", settings->get_double("threshold"));
}

void MaximizerPreset::load(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".maximizer.state");

  update_key<double>(root, settings, "release", section + ".maximizer.release");

  update_key<double>(root, settings, "ceiling", section + ".maximizer.ceiling");

  update_key<double>(root, settings, "threshold", section + ".maximizer.threshold");
}

void MaximizerPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  }
}

void MaximizerPreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  }
}
