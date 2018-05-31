#include "pitch_preset.hpp"

PitchPreset::PitchPreset()
    : input_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.pitch")) {}

void PitchPreset::save(boost::property_tree::ptree& root,
                       const std::string& section,
                       const Glib::RefPtr<Gio::Settings>& settings) {
    root.put(section + ".pitch.state", settings->get_boolean("state"));

    root.put(section + ".pitch.cents", settings->get_double("cents"));

    root.put(section + ".pitch.semitones", settings->get_int("semitones"));

    root.put(section + ".pitch.octaves", settings->get_int("octaves"));

    root.put(section + ".pitch.crispness", settings->get_int("crispness"));

    root.put(section + ".pitch.formant-preserving",
             settings->get_boolean("formant-preserving"));

    root.put(section + ".pitch.faster", settings->get_boolean("faster"));
}

void PitchPreset::load(boost::property_tree::ptree& root,
                       const std::string& section,
                       const Glib::RefPtr<Gio::Settings>& settings) {
    settings->set_boolean("state",
                          root.get<bool>(section + ".pitch.state",
                                         get_default<bool>(settings, "state")));

    settings->set_double(
        "cents", root.get<double>(section + ".pitch.cents",
                                  get_default<double>(settings, "cents")));

    settings->set_int("semitones",
                      root.get<int>(section + ".pitch.semitones",
                                    get_default<int>(settings, "semitones")));

    settings->set_int("octaves",
                      root.get<int>(section + ".pitch.octaves",
                                    get_default<int>(settings, "octaves")));

    settings->set_int("crispness",
                      root.get<int>(section + ".pitch.crispness",
                                    get_default<int>(settings, "crispness")));

    settings->set_boolean(
        "formant-preserving",
        root.get<bool>(section + ".pitch.formant-preserving",
                       get_default<bool>(settings, "formant-preserving")));

    settings->set_boolean(
        "faster", root.get<bool>(section + ".pitch.faster",
                                 get_default<bool>(settings, "faster")));
}

void PitchPreset::write(boost::property_tree::ptree& root) {
    save(root, "input", input_settings);
}

void PitchPreset::read(boost::property_tree::ptree& root) {
    load(root, "input", input_settings);
}
