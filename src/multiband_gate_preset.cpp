#include "multiband_gate_preset.hpp"

MultibandGatePreset::MultibandGatePreset()
    : input_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.multibandgate")),
      output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.multibandgate")) {}

void MultibandGatePreset::save(boost::property_tree::ptree& root,
                               const std::string& section,
                               const Glib::RefPtr<Gio::Settings>& settings) {
    root.put(section + ".multiband_gate.state", settings->get_boolean("state"));

    root.put(section + ".multiband_gate.input-gain",
             settings->get_double("input-gain"));

    root.put(section + ".multiband_gate.output-gain",
             settings->get_double("output-gain"));

    root.put(section + ".multiband_gate.freq0", settings->get_double("freq0"));

    root.put(section + ".multiband_gate.freq1", settings->get_double("freq1"));

    root.put(section + ".multiband_gate.freq2", settings->get_double("freq2"));

    root.put(section + ".multiband_gate.mode", settings->get_string("mode"));

    // sub band

    root.put(section + ".multiband_gate.subband.reduction",
             settings->get_double("range0"));

    root.put(section + ".multiband_gate.subband.threshold",
             settings->get_double("threshold0"));

    root.put(section + ".multiband_gate.subband.ratio",
             settings->get_double("ratio0"));

    root.put(section + ".multiband_gate.subband.attack",
             settings->get_double("attack0"));

    root.put(section + ".multiband_gate.subband.release",
             settings->get_double("release0"));

    root.put(section + ".multiband_gate.subband.makeup",
             settings->get_double("makeup0"));

    root.put(section + ".multiband_gate.subband.knee",
             settings->get_double("knee0"));

    root.put(section + ".multiband_gate.subband.detection",
             settings->get_string("detection0"));

    root.put(section + ".multiband_gate.subband.bypass",
             settings->get_boolean("bypass0"));

    root.put(section + ".multiband_gate.subband.solo",
             settings->get_boolean("solo0"));

    // low band

    root.put(section + ".multiband_gate.lowband.reduction",
             settings->get_double("range1"));

    root.put(section + ".multiband_gate.lowband.threshold",
             settings->get_double("threshold1"));

    root.put(section + ".multiband_gate.lowband.ratio",
             settings->get_double("ratio1"));

    root.put(section + ".multiband_gate.lowband.attack",
             settings->get_double("attack1"));

    root.put(section + ".multiband_gate.lowband.release",
             settings->get_double("release1"));

    root.put(section + ".multiband_gate.lowband.makeup",
             settings->get_double("makeup1"));

    root.put(section + ".multiband_gate.lowband.knee",
             settings->get_double("knee1"));

    root.put(section + ".multiband_gate.lowband.detection",
             settings->get_string("detection1"));

    root.put(section + ".multiband_gate.lowband.bypass",
             settings->get_boolean("bypass1"));

    root.put(section + ".multiband_gate.lowband.solo",
             settings->get_boolean("solo1"));

    // mid band

    root.put(section + ".multiband_gate.midband.reduction",
             settings->get_double("range2"));

    root.put(section + ".multiband_gate.midband.threshold",
             settings->get_double("threshold2"));

    root.put(section + ".multiband_gate.midband.ratio",
             settings->get_double("ratio2"));

    root.put(section + ".multiband_gate.midband.attack",
             settings->get_double("attack2"));

    root.put(section + ".multiband_gate.midband.release",
             settings->get_double("release2"));

    root.put(section + ".multiband_gate.midband.makeup",
             settings->get_double("makeup2"));

    root.put(section + ".multiband_gate.midband.knee",
             settings->get_double("knee2"));

    root.put(section + ".multiband_gate.midband.detection",
             settings->get_string("detection2"));

    root.put(section + ".multiband_gate.midband.bypass",
             settings->get_boolean("bypass2"));

    root.put(section + ".multiband_gate.midband.solo",
             settings->get_boolean("solo2"));

    // high band

    root.put(section + ".multiband_gate.highband.reduction",
             settings->get_double("range3"));

    root.put(section + ".multiband_gate.highband.threshold",
             settings->get_double("threshold3"));

    root.put(section + ".multiband_gate.highband.ratio",
             settings->get_double("ratio3"));

    root.put(section + ".multiband_gate.highband.attack",
             settings->get_double("attack3"));

    root.put(section + ".multiband_gate.highband.release",
             settings->get_double("release3"));

    root.put(section + ".multiband_gate.highband.makeup",
             settings->get_double("makeup3"));

    root.put(section + ".multiband_gate.highband.knee",
             settings->get_double("knee3"));

    root.put(section + ".multiband_gate.highband.detection",
             settings->get_string("detection3"));

    root.put(section + ".multiband_gate.highband.bypass",
             settings->get_boolean("bypass3"));

    root.put(section + ".multiband_gate.highband.solo",
             settings->get_boolean("solo3"));
}

void MultibandGatePreset::load(boost::property_tree::ptree& root,
                               const std::string& section,
                               const Glib::RefPtr<Gio::Settings>& settings) {
    settings->set_boolean("state",
                          root.get<bool>(section + ".multiband_gate.state",
                                         get_default<bool>(settings, "state")));

    settings->set_double(
        "input-gain",
        root.get<double>(section + ".multiband_gate.input-gain",
                         get_default<double>(settings, "input-gain")));

    settings->set_double(
        "output-gain",
        root.get<double>(section + ".multiband_gate.output-gain",
                         get_default<double>(settings, "output-gain")));

    settings->set_double(
        "freq0", root.get<double>(section + ".multiband_gate.freq0",
                                  get_default<double>(settings, "freq0")));

    settings->set_double(
        "freq1", root.get<double>(section + ".multiband_gate.freq1",
                                  get_default<double>(settings, "freq1")));

    settings->set_double(
        "freq2", root.get<double>(section + ".multiband_gate.freq2",
                                  get_default<double>(settings, "freq2")));

    settings->set_string(
        "mode",
        root.get<std::string>(section + ".multiband_gate.mode",
                              get_default<std::string>(settings, "mode")));

    // sub band

    settings->set_double(
        "range0",
        root.get<double>(section + ".multiband_gate.subband.reduction",
                         get_default<double>(settings, "range0")));

    settings->set_double(
        "threshold0",
        root.get<double>(section + ".multiband_gate.subband.threshold",
                         get_default<double>(settings, "threshold0")));

    settings->set_double(
        "ratio0", root.get<double>(section + ".multiband_gate.subband.ratio",
                                   get_default<double>(settings, "ratio0")));

    settings->set_double(
        "attack0", root.get<double>(section + ".multiband_gate.subband.attack",
                                    get_default<double>(settings, "attack0")));

    settings->set_double(
        "release0",
        root.get<double>(section + ".multiband_gate.subband.release",
                         get_default<double>(settings, "release0")));

    settings->set_double(
        "makeup0", root.get<double>(section + ".multiband_gate.subband.makeup",
                                    get_default<double>(settings, "makeup0")));

    settings->set_double(
        "knee0", root.get<double>(section + ".multiband_gate.subband.knee",
                                  get_default<double>(settings, "knee0")));

    settings->set_string("detection0",
                         root.get<std::string>(
                             section + ".multiband_gate.subband.detection",
                             get_default<std::string>(settings, "detection0")));

    settings->set_boolean(
        "bypass0", root.get<bool>(section + ".multiband_gate.subband.bypass",
                                  get_default<bool>(settings, "bypass0")));

    settings->set_boolean(
        "solo0", root.get<bool>(section + ".multiband_gate.subband.solo",
                                get_default<bool>(settings, "solo0")));

    // low band

    settings->set_double(
        "range1",
        root.get<double>(section + ".multiband_gate.lowband.reduction",
                         get_default<double>(settings, "range1")));

    settings->set_double(
        "threshold1",
        root.get<double>(section + ".multiband_gate.lowband.threshold",
                         get_default<double>(settings, "threshold1")));

    settings->set_double(
        "ratio1", root.get<double>(section + ".multiband_gate.lowband.ratio",
                                   get_default<double>(settings, "ratio1")));

    settings->set_double(
        "attack1", root.get<double>(section + ".multiband_gate.lowband.attack",
                                    get_default<double>(settings, "attack1")));

    settings->set_double(
        "release1",
        root.get<double>(section + ".multiband_gate.lowband.release",
                         get_default<double>(settings, "release1")));

    settings->set_double(
        "makeup1", root.get<double>(section + ".multiband_gate.lowband.makeup",
                                    get_default<double>(settings, "makeup1")));

    settings->set_double(
        "knee1", root.get<double>(section + ".multiband_gate.lowband.knee",
                                  get_default<double>(settings, "knee1")));

    settings->set_string("detection1",
                         root.get<std::string>(
                             section + ".multiband_gate.lowband.detection",
                             get_default<std::string>(settings, "detection1")));

    settings->set_boolean(
        "bypass1", root.get<bool>(section + ".multiband_gate.lowband.bypass",
                                  get_default<bool>(settings, "bypass1")));

    settings->set_boolean(
        "solo1", root.get<bool>(section + ".multiband_gate.lowband.solo",
                                get_default<bool>(settings, "solo1")));

    // mid band

    settings->set_double(
        "range2",
        root.get<double>(section + ".multiband_gate.midband.reduction",
                         get_default<double>(settings, "range2")));

    settings->set_double(
        "threshold2",
        root.get<double>(section + ".multiband_gate.midband.threshold",
                         get_default<double>(settings, "threshold2")));

    settings->set_double(
        "ratio2", root.get<double>(section + ".multiband_gate.midband.ratio",
                                   get_default<double>(settings, "ratio2")));

    settings->set_double(
        "attack2", root.get<double>(section + ".multiband_gate.midband.attack",
                                    get_default<double>(settings, "attack2")));

    settings->set_double(
        "release2",
        root.get<double>(section + ".multiband_gate.midband.release",
                         get_default<double>(settings, "release2")));

    settings->set_double(
        "makeup2", root.get<double>(section + ".multiband_gate.midband.makeup",
                                    get_default<double>(settings, "makeup2")));

    settings->set_double(
        "knee2", root.get<double>(section + ".multiband_gate.midband.knee",
                                  get_default<double>(settings, "knee2")));

    settings->set_string("detection2",
                         root.get<std::string>(
                             section + ".multiband_gate.midband.detection",
                             get_default<std::string>(settings, "detection2")));

    settings->set_boolean(
        "bypass2", root.get<bool>(section + ".multiband_gate.midband.bypass",
                                  get_default<bool>(settings, "bypass2")));

    settings->set_boolean(
        "solo2", root.get<bool>(section + ".multiband_gate.midband.solo",
                                get_default<bool>(settings, "solo2")));

    // high band

    settings->set_double(
        "range3",
        root.get<double>(section + ".multiband_gate.highband.reduction",
                         get_default<double>(settings, "range3")));

    settings->set_double(
        "threshold3",
        root.get<double>(section + ".multiband_gate.highband.threshold",
                         get_default<double>(settings, "threshold3")));

    settings->set_double(
        "ratio3", root.get<double>(section + ".multiband_gate.highband.ratio",
                                   get_default<double>(settings, "ratio3")));

    settings->set_double(
        "attack3", root.get<double>(section + ".multiband_gate.highband.attack",
                                    get_default<double>(settings, "attack3")));

    settings->set_double(
        "release3",
        root.get<double>(section + ".multiband_gate.highband.release",
                         get_default<double>(settings, "release3")));

    settings->set_double(
        "makeup3", root.get<double>(section + ".multiband_gate.highband.makeup",
                                    get_default<double>(settings, "makeup3")));

    settings->set_double(
        "knee3", root.get<double>(section + ".multiband_gate.highband.knee",
                                  get_default<double>(settings, "knee3")));

    settings->set_string("detection3",
                         root.get<std::string>(
                             section + ".multiband_gate.highband.detection",
                             get_default<std::string>(settings, "detection3")));

    settings->set_boolean(
        "bypass3", root.get<bool>(section + ".multiband_gate.highband.bypass",
                                  get_default<bool>(settings, "bypass3")));

    settings->set_boolean(
        "solo3", root.get<bool>(section + ".multiband_gate.highband.solo",
                                get_default<bool>(settings, "solo3")));
}

void MultibandGatePreset::write(boost::property_tree::ptree& root) {
    save(root, "input", input_settings);
    save(root, "output", output_settings);
}

void MultibandGatePreset::read(boost::property_tree::ptree& root) {
    load(root, "input", input_settings);
    load(root, "output", output_settings);
}
