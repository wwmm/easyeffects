#include "filter_preset.hpp"

FilterPreset::FilterPreset()
    : input_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs.filter")),
      output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.filter")) {}

void FilterPreset::save(boost::property_tree::ptree& root,
                        const std::string& section,
                        const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".filter.state", settings->get_boolean("state"));

  root.put(section + ".filter.input-gain", settings->get_double("input-gain"));

  root.put(section + ".filter.output-gain", settings->get_double("output-gain"));

  root.put(section + ".filter.frequency", settings->get_double("frequency"));

  root.put(section + ".filter.resonance", settings->get_double("resonance"));

  root.put(section + ".filter.mode", settings->get_string("mode"));

  root.put(section + ".filter.inertia", settings->get_double("inertia"));
}

void FilterPreset::load(boost::property_tree::ptree& root,
                        const std::string& section,
                        const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".filter.state");

  update_key<double>(root, settings, "input-gain", section + ".filter.input-gain");

  update_key<double>(root, settings, "output-gain", section + ".filter.output-gain");

  update_key<double>(root, settings, "frequency", section + ".filter.frequency");

  update_key<double>(root, settings, "resonance", section + ".filter.resonance");

  update_string_key(root, settings, "mode", section + ".filter.mode");

  update_key<double>(root, settings, "inertia", section + ".filter.inertia");
}

void FilterPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  } else {
    save(root, "input", input_settings);
  }
}

void FilterPreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  } else {
    load(root, "input", input_settings);
  }
}
