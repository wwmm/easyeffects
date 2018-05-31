#include "equalizer_preset.hpp"

EqualizerPreset::EqualizerPreset()
    : input_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.equalizer")),
      output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.equalizer")) {}

void EqualizerPreset::save(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
    root.put(section + ".equalizer.state", settings->get_boolean("state"));

    int nbands = settings->get_int("num-bands");

    root.put(section + ".equalizer.num-bands", nbands);

    for (int n = 0; n < nbands; n++) {
        root.put(section + ".equalizer.band" + std::to_string(n) + ".gain",
                 settings->get_double(
                     std::string("band" + std::to_string(n) + "-gain")));
    }
}

void EqualizerPreset::load(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
    settings->set_boolean("state",
                          root.get<bool>(section + ".equalizer.state",
                                         get_default<bool>(settings, "state")));

    settings->set_string(
        "detection",
        root.get<std::string>(section + ".equalizer.detection",
                              get_default<std::string>(settings, "detection")));

    settings->set_string(
        "stereo-link", root.get<std::string>(
                           section + ".equalizer.stereo-link",
                           get_default<std::string>(settings, "stereo-link")));

    settings->set_double(
        "mix", root.get<double>(section + ".equalizer.mix",
                                get_default<double>(settings, "mix")));

    settings->set_double(
        "attack", root.get<double>(section + ".equalizer.attack",
                                   get_default<double>(settings, "attack")));

    settings->set_double(
        "release", root.get<double>(section + ".equalizer.release",
                                    get_default<double>(settings, "release")));

    settings->set_double(
        "threshold",
        root.get<double>(section + ".equalizer.threshold",
                         get_default<double>(settings, "threshold")));

    settings->set_double(
        "ratio", root.get<double>(section + ".equalizer.ratio",
                                  get_default<double>(settings, "ratio")));

    settings->set_double(
        "knee", root.get<double>(section + ".equalizer.knee",
                                 get_default<double>(settings, "knee")));

    settings->set_double(
        "makeup", root.get<double>(section + ".equalizer.makeup",
                                   get_default<double>(settings, "makeup")));
}

void EqualizerPreset::write(boost::property_tree::ptree& root) {
    save(root, "input", input_settings);
    save(root, "output", output_settings);
}

void EqualizerPreset::read(boost::property_tree::ptree& root) {
    load(root, "input", input_settings);
    load(root, "output", output_settings);
}
