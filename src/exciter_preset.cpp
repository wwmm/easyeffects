#include "exciter_preset.hpp"

ExciterPreset::ExciterPreset()
    : output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.exciter")) {}

void ExciterPreset::save(boost::property_tree::ptree& root,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".exciter.state", settings->get_boolean("state"));

  root.put(section + ".exciter.input-gain", settings->get_double("input-gain"));

  root.put(section + ".exciter.output-gain", settings->get_double("output-gain"));

  root.put(section + ".exciter.amount", settings->get_double("amount"));

  root.put(section + ".exciter.harmonics", settings->get_double("harmonics"));

  root.put(section + ".exciter.scope", settings->get_double("scope"));

  root.put(section + ".exciter.ceil", settings->get_double("ceil"));

  root.put(section + ".exciter.blend", settings->get_double("blend"));

  root.put(section + ".exciter.ceil-active", settings->get_boolean("ceil-active"));

  root.put(section + ".exciter.listen", settings->get_boolean("listen"));
}

void ExciterPreset::load(boost::property_tree::ptree& root,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".exciter.state");

  update_key<double>(root, settings, "input-gain", section + ".exciter.input-gain");

  update_key<double>(root, settings, "output-gain", section + ".exciter.output-gain");

  update_key<double>(root, settings, "amount", section + ".exciter.amount");

  update_key<double>(root, settings, "harmonics", section + ".exciter.harmonics");

  update_key<double>(root, settings, "scope", section + ".exciter.scope");

  update_key<double>(root, settings, "ceil", section + ".exciter.ceil");

  update_key<double>(root, settings, "blend", section + ".exciter.blend");

  update_key<bool>(root, settings, "ceil-active", section + ".exciter.ceil-active");

  update_key<bool>(root, settings, "listen", section + ".exciter.listen");
}

void ExciterPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  }
}

void ExciterPreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  }
}
