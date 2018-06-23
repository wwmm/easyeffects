#include "multiband_compressor_preset.hpp"

MultibandCompressorPreset::MultibandCompressorPreset()
    : input_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.multibandcompressor")),
      output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.multibandcompressor")) {}

void MultibandCompressorPreset::save(
    boost::property_tree::ptree& root,
    const std::string& section,
    const Glib::RefPtr<Gio::Settings>& settings) {
    root.put(section + ".multiband_compressor.state",
             settings->get_boolean("state"));

    root.put(section + ".multiband_compressor.input-gain",
             settings->get_double("input-gain"));

    root.put(section + ".multiband_compressor.output-gain",
             settings->get_double("output-gain"));

    root.put(section + ".multiband_compressor.freq0",
             settings->get_double("freq0"));

    root.put(section + ".multiband_compressor.freq1",
             settings->get_double("freq1"));

    root.put(section + ".multiband_compressor.freq2",
             settings->get_double("freq2"));

    root.put(section + ".multiband_compressor.mode",
             settings->get_string("mode"));

    // sub band

    root.put(section + ".multiband_compressor.threshold0",
             settings->get_double("threshold0"));

    root.put(section + ".multiband_compressor.ratio0",
             settings->get_double("ratio0"));

    root.put(section + ".multiband_compressor.attack0",
             settings->get_double("attack0"));

    root.put(section + ".multiband_compressor.release0",
             settings->get_double("release0"));

    root.put(section + ".multiband_compressor.makeup0",
             settings->get_double("makeup0"));

    root.put(section + ".multiband_compressor.knee0",
             settings->get_double("knee0"));

    root.put(section + ".multiband_compressor.detection0",
             settings->get_string("detection0"));

    root.put(section + ".multiband_compressor.bypass0",
             settings->get_boolean("bypass0"));

    root.put(section + ".multiband_compressor.solo0",
             settings->get_boolean("solo0"));

    // low band

    root.put(section + ".multiband_compressor.threshold1",
             settings->get_double("threshold1"));

    root.put(section + ".multiband_compressor.ratio1",
             settings->get_double("ratio1"));

    root.put(section + ".multiband_compressor.attack1",
             settings->get_double("attack1"));

    root.put(section + ".multiband_compressor.release1",
             settings->get_double("release1"));

    root.put(section + ".multiband_compressor.makeup1",
             settings->get_double("makeup1"));

    root.put(section + ".multiband_compressor.knee1",
             settings->get_double("knee1"));

    root.put(section + ".multiband_compressor.detection1",
             settings->get_string("detection1"));

    root.put(section + ".multiband_compressor.bypass1",
             settings->get_boolean("bypass1"));

    root.put(section + ".multiband_compressor.solo1",
             settings->get_boolean("solo1"));

    // mid band

    root.put(section + ".multiband_compressor.threshold2",
             settings->get_double("threshold2"));

    root.put(section + ".multiband_compressor.ratio2",
             settings->get_double("ratio2"));

    root.put(section + ".multiband_compressor.attack2",
             settings->get_double("attack2"));

    root.put(section + ".multiband_compressor.release2",
             settings->get_double("release2"));

    root.put(section + ".multiband_compressor.makeup2",
             settings->get_double("makeup2"));

    root.put(section + ".multiband_compressor.knee2",
             settings->get_double("knee2"));

    root.put(section + ".multiband_compressor.detection2",
             settings->get_string("detection2"));

    root.put(section + ".multiband_compressor.bypass2",
             settings->get_boolean("bypass2"));

    root.put(section + ".multiband_compressor.solo2",
             settings->get_boolean("solo2"));

    // high band

    root.put(section + ".multiband_compressor.threshold3",
             settings->get_double("threshold3"));

    root.put(section + ".multiband_compressor.ratio3",
             settings->get_double("ratio3"));

    root.put(section + ".multiband_compressor.attack3",
             settings->get_double("attack3"));

    root.put(section + ".multiband_compressor.release3",
             settings->get_double("release3"));

    root.put(section + ".multiband_compressor.makeup3",
             settings->get_double("makeup3"));

    root.put(section + ".multiband_compressor.knee3",
             settings->get_double("knee3"));

    root.put(section + ".multiband_compressor.detection3",
             settings->get_string("detection3"));

    root.put(section + ".multiband_compressor.bypass3",
             settings->get_boolean("bypass3"));

    root.put(section + ".multiband_compressor.solo3",
             settings->get_boolean("solo3"));
}

void MultibandCompressorPreset::load(
    boost::property_tree::ptree& root,
    const std::string& section,
    const Glib::RefPtr<Gio::Settings>& settings) {
    settings->set_boolean(
        "state", root.get<bool>(section + ".multiband_compressor.state",
                                get_default<bool>(settings, "state")));

    settings->set_string(
        "detection",
        root.get<std::string>(section + ".multiband_compressor.detection",
                              get_default<std::string>(settings, "detection")));

    settings->set_string(
        "stereo-link", root.get<std::string>(
                           section + ".multiband_compressor.stereo-link",
                           get_default<std::string>(settings, "stereo-link")));

    settings->set_double(
        "mix", root.get<double>(section + ".multiband_compressor.mix",
                                get_default<double>(settings, "mix")));

    settings->set_double(
        "attack", root.get<double>(section + ".multiband_compressor.attack",
                                   get_default<double>(settings, "attack")));

    settings->set_double(
        "release", root.get<double>(section + ".multiband_compressor.release",
                                    get_default<double>(settings, "release")));

    settings->set_double(
        "threshold",
        root.get<double>(section + ".multiband_compressor.threshold",
                         get_default<double>(settings, "threshold")));

    settings->set_double(
        "ratio", root.get<double>(section + ".multiband_compressor.ratio",
                                  get_default<double>(settings, "ratio")));

    settings->set_double(
        "knee", root.get<double>(section + ".multiband_compressor.knee",
                                 get_default<double>(settings, "knee")));

    settings->set_double(
        "makeup", root.get<double>(section + ".multiband_compressor.makeup",
                                   get_default<double>(settings, "makeup")));
}

void MultibandCompressorPreset::write(boost::property_tree::ptree& root) {
    save(root, "input", input_settings);
    save(root, "output", output_settings);
}

void MultibandCompressorPreset::read(boost::property_tree::ptree& root) {
    load(root, "input", input_settings);
    load(root, "output", output_settings);
}
