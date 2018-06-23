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

    settings->set_double(
        "input-gain",
        root.get<double>(section + ".multiband_compressor.input-gain",
                         get_default<double>(settings, "input-gain")));

    settings->set_double(
        "output-gain",
        root.get<double>(section + ".multiband_compressor.output-gain",
                         get_default<double>(settings, "output-gain")));

    settings->set_double(
        "freq0", root.get<double>(section + ".multiband_compressor.freq0",
                                  get_default<double>(settings, "freq0")));

    settings->set_double(
        "freq1", root.get<double>(section + ".multiband_compressor.freq1",
                                  get_default<double>(settings, "freq1")));

    settings->set_double(
        "freq2", root.get<double>(section + ".multiband_compressor.freq2",
                                  get_default<double>(settings, "freq2")));

    settings->set_string(
        "mode",
        root.get<std::string>(section + ".multiband_compressor.mode",
                              get_default<std::string>(settings, "mode")));

    // sub band

    settings->set_double(
        "threshold0",
        root.get<double>(section + ".multiband_compressor.threshold0",
                         get_default<double>(settings, "threshold0")));

    settings->set_double(
        "ratio0", root.get<double>(section + ".multiband_compressor.ratio0",
                                   get_default<double>(settings, "ratio0")));

    settings->set_double(
        "attack0", root.get<double>(section + ".multiband_compressor.attack0",
                                    get_default<double>(settings, "attack0")));

    settings->set_double(
        "release0",
        root.get<double>(section + ".multiband_compressor.release0",
                         get_default<double>(settings, "release0")));

    settings->set_double(
        "makeup0", root.get<double>(section + ".multiband_compressor.makeup0",
                                    get_default<double>(settings, "makeup0")));

    settings->set_double(
        "knee0", root.get<double>(section + ".multiband_compressor.knee0",
                                  get_default<double>(settings, "knee0")));

    settings->set_string("detection0",
                         root.get<std::string>(
                             section + ".multiband_compressor.detection0",
                             get_default<std::string>(settings, "detection0")));

    settings->set_boolean(
        "bypass0", root.get<bool>(section + ".multiband_compressor.bypass0",
                                  get_default<bool>(settings, "bypass0")));

    settings->set_boolean(
        "solo0", root.get<bool>(section + ".multiband_compressor.solo0",
                                get_default<bool>(settings, "solo0")));

    // low band

    settings->set_double(
        "threshold1",
        root.get<double>(section + ".multiband_compressor.threshold1",
                         get_default<double>(settings, "threshold1")));

    settings->set_double(
        "ratio1", root.get<double>(section + ".multiband_compressor.ratio1",
                                   get_default<double>(settings, "ratio1")));

    settings->set_double(
        "attack1", root.get<double>(section + ".multiband_compressor.attack1",
                                    get_default<double>(settings, "attack1")));

    settings->set_double(
        "release1",
        root.get<double>(section + ".multiband_compressor.release1",
                         get_default<double>(settings, "release1")));

    settings->set_double(
        "makeup1", root.get<double>(section + ".multiband_compressor.makeup1",
                                    get_default<double>(settings, "makeup1")));

    settings->set_double(
        "knee1", root.get<double>(section + ".multiband_compressor.knee1",
                                  get_default<double>(settings, "knee1")));

    settings->set_string("detection1",
                         root.get<std::string>(
                             section + ".multiband_compressor.detection1",
                             get_default<std::string>(settings, "detection1")));

    settings->set_boolean(
        "bypass1", root.get<bool>(section + ".multiband_compressor.bypass1",
                                  get_default<bool>(settings, "bypass1")));

    settings->set_boolean(
        "solo1", root.get<bool>(section + ".multiband_compressor.solo1",
                                get_default<bool>(settings, "solo1")));

    // mid band

    settings->set_double(
        "threshold2",
        root.get<double>(section + ".multiband_compressor.threshold2",
                         get_default<double>(settings, "threshold2")));

    settings->set_double(
        "ratio2", root.get<double>(section + ".multiband_compressor.ratio2",
                                   get_default<double>(settings, "ratio2")));

    settings->set_double(
        "attack0", root.get<double>(section + ".multiband_compressor.attack0",
                                    get_default<double>(settings, "attack0")));

    settings->set_double(
        "release2",
        root.get<double>(section + ".multiband_compressor.release2",
                         get_default<double>(settings, "release2")));

    settings->set_double(
        "makeup2", root.get<double>(section + ".multiband_compressor.makeup2",
                                    get_default<double>(settings, "makeup2")));

    settings->set_double(
        "knee2", root.get<double>(section + ".multiband_compressor.knee2",
                                  get_default<double>(settings, "knee2")));

    settings->set_string("detection2",
                         root.get<std::string>(
                             section + ".multiband_compressor.detection2",
                             get_default<std::string>(settings, "detection2")));

    settings->set_boolean(
        "bypass2", root.get<bool>(section + ".multiband_compressor.bypass2",
                                  get_default<bool>(settings, "bypass2")));

    settings->set_boolean(
        "solo2", root.get<bool>(section + ".multiband_compressor.solo2",
                                get_default<bool>(settings, "solo2")));

    // high band

    settings->set_double(
        "threshold3",
        root.get<double>(section + ".multiband_compressor.threshold3",
                         get_default<double>(settings, "threshold3")));

    settings->set_double(
        "ratio3", root.get<double>(section + ".multiband_compressor.ratio3",
                                   get_default<double>(settings, "ratio3")));

    settings->set_double(
        "attack3", root.get<double>(section + ".multiband_compressor.attack3",
                                    get_default<double>(settings, "attack3")));

    settings->set_double(
        "release3",
        root.get<double>(section + ".multiband_compressor.release3",
                         get_default<double>(settings, "release3")));

    settings->set_double(
        "makeup3", root.get<double>(section + ".multiband_compressor.makeup3",
                                    get_default<double>(settings, "makeup3")));

    settings->set_double(
        "knee3", root.get<double>(section + ".multiband_compressor.knee3",
                                  get_default<double>(settings, "knee3")));

    settings->set_string("detection3",
                         root.get<std::string>(
                             section + ".multiband_compressor.detection3",
                             get_default<std::string>(settings, "detection3")));

    settings->set_boolean(
        "bypass3", root.get<bool>(section + ".multiband_compressor.bypass3",
                                  get_default<bool>(settings, "bypass3")));

    settings->set_boolean(
        "solo3", root.get<bool>(section + ".multiband_compressor.solo3",
                                get_default<bool>(settings, "solo3")));
}

void MultibandCompressorPreset::write(boost::property_tree::ptree& root) {
    save(root, "input", input_settings);
    save(root, "output", output_settings);
}

void MultibandCompressorPreset::read(boost::property_tree::ptree& root) {
    load(root, "input", input_settings);
    load(root, "output", output_settings);
}
