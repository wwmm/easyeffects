#include "convolver_preset.hpp"

ConvolverPreset::ConvolverPreset()
    : output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.convolver")) {}

void ConvolverPreset::save(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".convolver.state", settings->get_boolean("state"));

  root.put(section + ".convolver.input-gain", settings->get_double("input-gain"));

  root.put(section + ".convolver.output-gain", settings->get_double("output-gain"));

  root.put(section + ".convolver.kernel-path", settings->get_string("kernel-path"));

  root.put(section + ".convolver.ir-width", settings->get_int("ir-width"));
}

void ConvolverPreset::load(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".convolver.state");

  update_key<double>(root, settings, "input-gain", section + ".convolver.input-gain");

  update_key<double>(root, settings, "output-gain", section + ".convolver.output-gain");

  update_string_key(root, settings, "kernel-path", section + ".convolver.kernel-path");

  update_key<int>(root, settings, "ir-width", section + ".convolver.ir-width");
}

void ConvolverPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  }
}

void ConvolverPreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  }
}
