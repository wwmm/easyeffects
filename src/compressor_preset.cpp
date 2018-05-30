#include "compressor_preset.hpp"

CompressorPreset::CompressorPreset()
    : input_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.compressor")),
      output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.compressor")) {}

void CompressorPreset::save(boost::property_tree::ptree& root,
                            const std::string& section,
                            const Glib::RefPtr<Gio::Settings>& settings) {
    root.put(section + ".compressor.state", settings->get_boolean("state"));

    root.put(section + ".compressor.detection",
             settings->get_string("detection"));

    root.put(section + ".compressor.stereo-link",
             settings->get_string("stereo-link"));

    root.put(section + ".compressor.mix", settings->get_double("mix"));

    root.put(section + ".compressor.attack", settings->get_double("attack"));

    root.put(section + ".compressor.release", settings->get_double("release"));

    root.put(section + ".compressor.threshold",
             settings->get_double("threshold"));

    root.put(section + ".compressor.ratio", settings->get_double("ratio"));

    root.put(section + ".compressor.knee", settings->get_double("knee"));

    root.put(section + ".compressor.makeup", settings->get_double("makeup"));
}

void CompressorPreset::load(boost::property_tree::ptree& root,
                            const std::string& section,
                            const Glib::RefPtr<Gio::Settings>& settings) {
    settings->set_boolean("state",
                          root.get<bool>(section + ".compressor.state",
                                         get_default<bool>(settings, "state")));

    settings->set_string(
        "detection",
        root.get<std::string>(section + ".compressor.detection",
                              get_default<std::string>(settings, "detection")));

    settings->set_string(
        "stereo-link", root.get<std::string>(
                           section + ".compressor.stereo-link",
                           get_default<std::string>(settings, "stereo-link")));

    settings->set_double(
        "mix", root.get<double>(section + ".compressor.mix",
                                get_default<double>(settings, "mix")));

    settings->set_double(
        "attack", root.get<double>(section + ".compressor.attack",
                                   get_default<double>(settings, "attack")));

    settings->set_double(
        "release", root.get<double>(section + ".compressor.release",
                                    get_default<double>(settings, "release")));

    settings->set_double(
        "threshold",
        root.get<double>(section + ".compressor.threshold",
                         get_default<double>(settings, "threshold")));

    settings->set_double(
        "ratio", root.get<double>(section + ".compressor.ratio",
                                  get_default<double>(settings, "ratio")));

    settings->set_double(
        "knee", root.get<double>(section + ".compressor.knee",
                                 get_default<double>(settings, "knee")));

    settings->set_double(
        "makeup", root.get<double>(section + ".compressor.makeup",
                                   get_default<double>(settings, "makeup")));
}

void CompressorPreset::write(boost::property_tree::ptree& root) {
    save(root, "input", input_settings);
    save(root, "output", output_settings);
}

void CompressorPreset::read(boost::property_tree::ptree& root) {
    load(root, "input", input_settings);
    load(root, "output", output_settings);
}
