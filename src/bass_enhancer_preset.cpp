#include "bass_enhancer_preset.hpp"

BassEnhancerPreset::BassEnhancerPreset()
    : output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.bassenhancer")) {}

void BassEnhancerPreset::save(boost::property_tree::ptree& root,
                              const std::string& section,
                              const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".bass_enhancer.state", settings->get_boolean("state"));

  root.put(section + ".bass_enhancer.input-gain", settings->get_double("input-gain"));

  root.put(section + ".bass_enhancer.output-gain", settings->get_double("output-gain"));

  root.put(section + ".bass_enhancer.amount", settings->get_double("amount"));

  root.put(section + ".bass_enhancer.harmonics", settings->get_double("harmonics"));

  root.put(section + ".bass_enhancer.scope", settings->get_double("scope"));

  root.put(section + ".bass_enhancer.floor", settings->get_double("floor"));

  root.put(section + ".bass_enhancer.blend", settings->get_double("blend"));

  root.put(section + ".bass_enhancer.floor-active", settings->get_boolean("floor-active"));

  root.put(section + ".bass_enhancer.listen", settings->get_boolean("listen"));
}

void BassEnhancerPreset::load(boost::property_tree::ptree& root,
                              const std::string& section,
                              const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".bass_enhancer.state");

  update_key<double>(root, settings, "input-gain", section + ".bass_enhancer.input-gain");

  update_key<double>(root, settings, "output-gain", section + ".bass_enhancer.output-gain");

  update_key<double>(root, settings, "amount", section + ".bass_enhancer.amount");

  update_key<double>(root, settings, "harmonics", section + ".bass_enhancer.harmonics");

  update_key<double>(root, settings, "scope", section + ".bass_enhancer.scope");

  update_key<double>(root, settings, "floor", section + ".bass_enhancer.floor");

  update_key<double>(root, settings, "blend", section + ".bass_enhancer.blend");

  update_key<bool>(root, settings, "floor-active", section + ".bass_enhancer.floor-active");

  update_key<bool>(root, settings, "listen", section + ".bass_enhancer.listen");
}

void BassEnhancerPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  }
}

void BassEnhancerPreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  }
}
