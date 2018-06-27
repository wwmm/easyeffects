#include "loudness_preset.hpp"

LoudnessPreset::LoudnessPreset()
    : output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.loudness")) {}

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
    settings->set_boolean("state",
                          root.get<bool>(section + ".loudness.state",
                                         get_default<bool>(settings, "state")));

    settings->set_double(
        "loudness",
        root.get<double>(section + ".loudness.loudness",
                         get_default<double>(settings, "loudness")));

    settings->set_double(
        "output", root.get<double>(section + ".loudness.output",
                                   get_default<double>(settings, "output")));

    settings->set_double(
        "link", root.get<double>(section + ".loudness.link",
                                 get_default<double>(settings, "link")));
}

void LoudnessPreset::write(boost::property_tree::ptree& root) {
    save(root, "output", output_settings);
}

void LoudnessPreset::read(boost::property_tree::ptree& root) {
    load(root, "output", output_settings);
}
