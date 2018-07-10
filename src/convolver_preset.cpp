#include "convolver_preset.hpp"

ConvolverPreset::ConvolverPreset()
    : output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.convolver")) {}

void ConvolverPreset::save(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
    root.put(section + ".convolver.state", settings->get_boolean("state"));

    root.put(section + ".convolver.input-gain",
             settings->get_double("input-gain"));

    root.put(section + ".convolver.output-gain",
             settings->get_double("output-gain"));

    root.put(section + ".convolver.kernel-path",
             settings->get_string("kernel-path"));

    root.put(section + ".convolver.buffersize",
             settings->get_string("buffersize"));

    root.put(section + ".convolver.ir-width", settings->get_int("ir-width"));
}

void ConvolverPreset::load(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
    settings->set_boolean("state",
                          root.get<bool>(section + ".convolver.state",
                                         get_default<bool>(settings, "state")));

    settings->set_double(
        "input-gain",
        root.get<double>(section + ".convolver.input-gain",
                         get_default<double>(settings, "input-gain")));

    settings->set_double(
        "output-gain",
        root.get<double>(section + ".convolver.output-gain",
                         get_default<double>(settings, "output-gain")));

    settings->set_string(
        "kernel-path", root.get<std::string>(
                           section + ".convolver.kernel-path",
                           get_default<std::string>(settings, "kernel-path")));

    settings->set_string("buffersize",
                         root.get<std::string>(
                             section + ".convolver.buffersize",
                             get_default<std::string>(settings, "buffersize")));

    settings->set_int("ir-width",
                      root.get<int>(section + ".convolver.ir-width",
                                    get_default<int>(settings, "ir-width")));
}

void ConvolverPreset::write(boost::property_tree::ptree& root) {
    save(root, "output", output_settings);
}

void ConvolverPreset::read(boost::property_tree::ptree& root) {
    load(root, "output", output_settings);
}
