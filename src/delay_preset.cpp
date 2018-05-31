#include "delay_preset.hpp"

DelayPreset::DelayPreset()
    : output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.delay")) {}

void DelayPreset::save(boost::property_tree::ptree& root,
                       const std::string& section,
                       const Glib::RefPtr<Gio::Settings>& settings) {
    root.put(section + ".delay.state", settings->get_boolean("state"));

    root.put(section + ".delay.m-l", settings->get_int("m-l"));

    root.put(section + ".delay.cm-l", settings->get_double("cm-l"));

    root.put(section + ".delay.m-r", settings->get_int("m-r"));

    root.put(section + ".delay.cm-r", settings->get_double("cm-r"));

    root.put(section + ".delay.temperature",
             settings->get_double("temperature"));
}

void DelayPreset::load(boost::property_tree::ptree& root,
                       const std::string& section,
                       const Glib::RefPtr<Gio::Settings>& settings) {
    settings->set_boolean("state",
                          root.get<bool>(section + ".delay.state",
                                         get_default<bool>(settings, "state")));

    settings->set_int("m-l", root.get<int>(section + ".delay.m-l",
                                           get_default<int>(settings, "m-l")));

    settings->set_double(
        "cm-l", root.get<double>(section + ".delay.cm-l",
                                 get_default<double>(settings, "cm-l")));

    settings->set_int("m-r", root.get<int>(section + ".delay.m-r",
                                           get_default<int>(settings, "m-r")));

    settings->set_double(
        "cm-r", root.get<double>(section + ".delay.cm-r",
                                 get_default<double>(settings, "cm-r")));

    settings->set_double(
        "temperature",
        root.get<double>(section + ".delay.temperature",
                         get_default<double>(settings, "temperature")));
}

void DelayPreset::write(boost::property_tree::ptree& root) {
    save(root, "output", output_settings);
}

void DelayPreset::read(boost::property_tree::ptree& root) {
    load(root, "output", output_settings);
}
