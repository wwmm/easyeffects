#include "expander_preset.hpp"

ExpanderPreset::ExpanderPreset()
    : output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.expander")) {}

void ExpanderPreset::save(boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
    root.put(section + ".expander.state", settings->get_boolean("state"));

    root.put(section + ".expander.sidechain.mode", settings->get_string("scm"));

    root.put(section + ".expander.sidechain.lookahead",
             settings->get_double("sla"));

    root.put(section + ".expander.sidechain.listen",
             settings->get_boolean("scl"));

    root.put(section + ".expander.sidechain.source",
             settings->get_string("scs"));

    root.put(section + ".expander.sidechain.reactivity",
             settings->get_double("scr"));

    root.put(section + ".expander.sidechain.preamp",
             settings->get_double("scp"));

    root.put(section + ".expander.mode", settings->get_string("em"));

    root.put(section + ".expander.attack-level", settings->get_double("al"));

    root.put(section + ".expander.attack-time", settings->get_double("at"));

    root.put(section + ".expander.release-level", settings->get_double("rrl"));

    root.put(section + ".expander.release-time", settings->get_double("rt"));

    root.put(section + ".expander.ratio", settings->get_double("cr"));

    root.put(section + ".expander.knee", settings->get_double("kn"));

    root.put(section + ".expander.makeup", settings->get_double("mk"));
}

void ExpanderPreset::load(boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
    settings->set_boolean("state",
                          root.get<bool>(section + ".expander.state",
                                         get_default<bool>(settings, "state")));

    settings->set_string("scm", root.get<std::string>(
                                    section + ".expander.sidechain.mode",
                                    get_default<std::string>(settings, "scm")));

    settings->set_double(
        "sla", root.get<double>(section + ".expander.sidechain.lookahead",
                                get_default<double>(settings, "sla")));

    settings->set_boolean("scl",
                          root.get<bool>(section + ".expander.sidechain.listen",
                                         get_default<bool>(settings, "scl")));

    settings->set_string("scs", root.get<std::string>(
                                    section + ".expander.sidechain.source",
                                    get_default<std::string>(settings, "scs")));

    settings->set_double(
        "scr", root.get<double>(section + ".expander.sidechain.reactivity",
                                get_default<double>(settings, "scr")));

    settings->set_double(
        "scp", root.get<double>(section + ".expander.sidechain.preamp",
                                get_default<double>(settings, "scp")));

    settings->set_string(
        "em", root.get<std::string>(section + ".expander.mode",
                                    get_default<std::string>(settings, "em")));

    settings->set_double("al",
                         root.get<double>(section + ".expander.attack-level",
                                          get_default<double>(settings, "al")));

    settings->set_double("at",
                         root.get<double>(section + ".expander.attack-time",
                                          get_default<double>(settings, "at")));

    settings->set_double(
        "rrl", root.get<double>(section + ".expander.release-level",
                                get_default<double>(settings, "rrl")));

    settings->set_double("rt",
                         root.get<double>(section + ".expander.release-time",
                                          get_default<double>(settings, "rt")));

    settings->set_double("cr",
                         root.get<double>(section + ".expander.ratio",
                                          get_default<double>(settings, "cr")));

    settings->set_double("kn",
                         root.get<double>(section + ".expander.knee",
                                          get_default<double>(settings, "kn")));

    settings->set_double("mk",
                         root.get<double>(section + ".expander.makeup",
                                          get_default<double>(settings, "mk")));
}

void ExpanderPreset::write(boost::property_tree::ptree& root) {
    save(root, "output", output_settings);
}

void ExpanderPreset::read(boost::property_tree::ptree& root) {
    load(root, "output", output_settings);
}
