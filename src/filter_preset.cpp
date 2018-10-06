#include "filter_preset.hpp"

FilterPreset::FilterPreset()
    : input_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.filter")),
      output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.filter")) {}

void FilterPreset::save(boost::property_tree::ptree& root,
                        const std::string& section,
                        const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".filter.state", settings->get_boolean("state"));

  root.put(section + ".filter.input-gain", settings->get_double("input-gain"));

  root.put(section + ".filter.output-gain",
           settings->get_double("output-gain"));

  root.put(section + ".filter.frequency", settings->get_double("frequency"));

  root.put(section + ".filter.resonance", settings->get_double("resonance"));

  root.put(section + ".filter.mode", settings->get_string("mode"));

  root.put(section + ".filter.inertia", settings->get_double("inertia"));
}

void FilterPreset::load(boost::property_tree::ptree& root,
                        const std::string& section,
                        const Glib::RefPtr<Gio::Settings>& settings) {
  settings->set_boolean("state",
                        root.get<bool>(section + ".filter.state",
                                       get_default<bool>(settings, "state")));

  settings->set_double(
      "input-gain",
      root.get<double>(section + ".filter.input-gain",
                       get_default<double>(settings, "input-gain")));

  settings->set_double(
      "output-gain",
      root.get<double>(section + ".filter.output-gain",
                       get_default<double>(settings, "output-gain")));

  settings->set_double(
      "frequency",
      root.get<double>(section + ".filter.frequency",
                       get_default<double>(settings, "frequency")));

  settings->set_double(
      "resonance",
      root.get<double>(section + ".filter.resonance",
                       get_default<double>(settings, "resonance")));

  settings->set_string("mode", root.get<std::string>(
                                   section + ".filter.mode",
                                   get_default<std::string>(settings, "mode")));

  settings->set_double(
      "inertia", root.get<double>(section + ".filter.inertia",
                                  get_default<double>(settings, "inertia")));
}

void FilterPreset::write(boost::property_tree::ptree& root) {
  save(root, "input", input_settings);
  save(root, "output", output_settings);
}

void FilterPreset::read(boost::property_tree::ptree& root) {
  load(root, "input", input_settings);
  load(root, "output", output_settings);
}
