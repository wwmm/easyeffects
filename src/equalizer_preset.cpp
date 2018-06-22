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

    root.put(section + ".equalizer.input-gain",
             settings->get_double("input-gain"));

    root.put(section + ".equalizer.output-gain",
             settings->get_double("output-gain"));

    for (int n = 0; n < nbands; n++) {
        root.put(section + ".equalizer.band" + std::to_string(n) + ".gain",
                 settings->get_double(
                     std::string("band" + std::to_string(n) + "-gain")));

        root.put(section + ".equalizer.band" + std::to_string(n) + ".frequency",
                 settings->get_double(
                     std::string("band" + std::to_string(n) + "-frequency")));

        root.put(section + ".equalizer.band" + std::to_string(n) + ".width",
                 settings->get_double(
                     std::string("band" + std::to_string(n) + "-width")));

        root.put(section + ".equalizer.band" + std::to_string(n) + ".type",
                 settings->get_string(
                     std::string("band" + std::to_string(n) + "-type")));
    }
}

void EqualizerPreset::load(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
    settings->set_boolean("state",
                          root.get<bool>(section + ".equalizer.state",
                                         get_default<bool>(settings, "state")));

    int nbands = root.get<int>(section + ".equalizer.num-bands",
                               get_default<int>(settings, "num-bands"));

    settings->set_int("num-bands", nbands);

    settings->set_double(
        "input-gain",
        root.get<double>(section + ".equalizer.input-gain",
                         get_default<double>(settings, "input-gain")));

    settings->set_double(
        "output-gain",
        root.get<double>(section + ".equalizer.output-gain",
                         get_default<double>(settings, "output-gain")));

    for (int n = 0; n < nbands; n++) {
        settings->set_double(
            std::string("band" + std::to_string(n) + "-gain"),
            root.get<double>(
                section + ".equalizer.band" + std::to_string(n) + ".gain",
                get_default<double>(
                    settings,
                    std::string("band" + std::to_string(n) + "-gain"))));

        settings->set_double(
            std::string("band" + std::to_string(n) + "-frequency"),
            root.get<double>(
                section + ".equalizer.band" + std::to_string(n) + ".frequency",
                get_default<double>(
                    settings,
                    std::string("band" + std::to_string(n) + "-frequency"))));

        settings->set_double(
            std::string("band" + std::to_string(n) + "-width"),
            root.get<double>(
                section + ".equalizer.band" + std::to_string(n) + ".width",
                get_default<double>(
                    settings,
                    std::string("band" + std::to_string(n) + "-width"))));

        settings->set_string(
            std::string("band" + std::to_string(n) + "-type"),
            root.get<std::string>(
                section + ".equalizer.band" + std::to_string(n) + ".type",
                get_default<std::string>(
                    settings,
                    std::string("band" + std::to_string(n) + "-type"))));
    }
}

void EqualizerPreset::write(boost::property_tree::ptree& root) {
    save(root, "input", input_settings);
    save(root, "output", output_settings);
}

void EqualizerPreset::read(boost::property_tree::ptree& root) {
    load(root, "input", input_settings);
    load(root, "output", output_settings);
}
