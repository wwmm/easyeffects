#include "panorama_preset.hpp"

PanoramaPreset::PanoramaPreset()
    : output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.panorama")) {}

void PanoramaPreset::save(boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
    root.put(section + ".panorama.state", settings->get_boolean("state"));

    root.put(section + ".panorama.position", settings->get_double("position"));
}

void PanoramaPreset::load(boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
    settings->set_boolean("state",
                          root.get<bool>(section + ".panorama.state",
                                         get_default<bool>(settings, "state")));

    settings->set_double(
        "position",
        root.get<double>(section + ".panorama.position",
                         get_default<double>(settings, "position")));
}

void PanoramaPreset::write(boost::property_tree::ptree& root) {
    save(root, "output", output_settings);
}

void PanoramaPreset::read(boost::property_tree::ptree& root) {
    load(root, "output", output_settings);
}
