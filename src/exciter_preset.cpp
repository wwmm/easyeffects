#include "exciter_preset.hpp"

ExciterPreset::ExciterPreset()
    : output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.exciter")) {}

void ExciterPreset::save(boost::property_tree::ptree& root,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
    root.put(section + ".exciter.state", settings->get_boolean("state"));

    root.put(section + ".exciter.input-gain",
             settings->get_double("input-gain"));

    root.put(section + ".exciter.output-gain",
             settings->get_double("output-gain"));

    root.put(section + ".exciter.amount", settings->get_double("amount"));

    root.put(section + ".exciter.harmonics", settings->get_double("harmonics"));

    root.put(section + ".exciter.scope", settings->get_double("scope"));

    root.put(section + ".exciter.ceil", settings->get_double("ceil"));

    root.put(section + ".exciter.blend", settings->get_double("blend"));

    root.put(section + ".exciter.ceil-active",
             settings->get_boolean("ceil-active"));

    root.put(section + ".exciter.listen", settings->get_boolean("listen"));
}

void ExciterPreset::load(boost::property_tree::ptree& root,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
    settings->set_boolean("state",
                          root.get<bool>(section + ".exciter.state",
                                         get_default<bool>(settings, "state")));

    settings->set_double(
        "input-gain",
        root.get<double>(section + ".exciter.input-gain",
                         get_default<double>(settings, "input-gain")));

    settings->set_double(
        "output-gain",
        root.get<double>(section + ".exciter.output-gain",
                         get_default<double>(settings, "output-gain")));

    settings->set_double(
        "amount", root.get<double>(section + ".exciter.amount",
                                   get_default<double>(settings, "amount")));

    settings->set_double(
        "harmonics",
        root.get<double>(section + ".exciter.harmonics",
                         get_default<double>(settings, "harmonics")));

    settings->set_double(
        "scope", root.get<double>(section + ".exciter.scope",
                                  get_default<double>(settings, "scope")));

    settings->set_double(
        "ceil", root.get<double>(section + ".exciter.ceil",
                                 get_default<double>(settings, "ceil")));

    settings->set_double(
        "blend", root.get<double>(section + ".exciter.blend",
                                  get_default<double>(settings, "blend")));

    settings->set_boolean(
        "ceil-active",
        root.get<bool>(section + ".exciter.ceil-active",
                       get_default<bool>(settings, "ceil-active")));

    settings->set_boolean(
        "listen", root.get<bool>(section + ".exciter.listen",
                                 get_default<bool>(settings, "listen")));
}

void ExciterPreset::write(boost::property_tree::ptree& root) {
    save(root, "output", output_settings);
}

void ExciterPreset::read(boost::property_tree::ptree& root) {
    load(root, "output", output_settings);
}
