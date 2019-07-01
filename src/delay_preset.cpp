#include "delay_preset.hpp"

DelayPreset::DelayPreset() : output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.delay")) {}

void DelayPreset::save(boost::property_tree::ptree& root,
                       const std::string& section,
                       const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".delay.state", settings->get_boolean("state"));

  root.put(section + ".delay.input-gain", settings->get_double("input-gain"));

  root.put(section + ".delay.output-gain", settings->get_double("output-gain"));

  root.put(section + ".delay.time-l", settings->get_double("time-l"));
  root.put(section + ".delay.time-r", settings->get_double("time-r"));
}

void DelayPreset::load(boost::property_tree::ptree& root,
                       const std::string& section,
                       const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".delay.state");

  update_key<double>(root, settings, "input-gain", section + ".delay.input-gain");

  update_key<double>(root, settings, "output-gain", section + ".delay.output-gain");

  update_key<double>(root, settings, "time-l", section + ".delay.time-l");

  update_key<double>(root, settings, "time-r", section + ".delay.time-r");
}

void DelayPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  }
}

void DelayPreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  }
}
