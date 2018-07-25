#include "autogain_preset.hpp"

AutoGainPreset::AutoGainPreset()
    : output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.autogain")) {}

void AutoGainPreset::save(boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
    root.put(section + ".autogain.state", settings->get_boolean("state"));

    root.put(section + ".autogain.input-gain",
             settings->get_double("input-gain"));

    root.put(section + ".autogain.output-gain",
             settings->get_double("output-gain"));

    root.put(section + ".autogain.target", settings->get_double("target"));

    root.put(section + ".autogain.window", settings->get_double("window"));
}

void AutoGainPreset::load(boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
    settings->set_boolean("state",
                          root.get<bool>(section + ".autogain.state",
                                         get_default<bool>(settings, "state")));

    settings->set_double(
        "input-gain",
        root.get<double>(section + ".autogain.input-gain",
                         get_default<double>(settings, "input-gain")));

    settings->set_double(
        "output-gain",
        root.get<double>(section + ".autogain.output-gain",
                         get_default<double>(settings, "output-gain")));

    settings->set_double(
        "target", root.get<double>(section + ".autogain.target",
                                   get_default<double>(settings, "target")));

    settings->set_double(
        "window", root.get<double>(section + ".autogain.window",
                                   get_default<double>(settings, "window")));
}

void AutoGainPreset::write(boost::property_tree::ptree& root) {
    save(root, "output", output_settings);
}

void AutoGainPreset::read(boost::property_tree::ptree& root) {
    load(root, "output", output_settings);
}
