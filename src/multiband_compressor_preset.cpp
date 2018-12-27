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

  root.put(section + ".multiband_compressor.subband.threshold",
           settings->get_double("threshold0"));

  root.put(section + ".multiband_compressor.subband.ratio",
           settings->get_double("ratio0"));

  root.put(section + ".multiband_compressor.subband.attack",
           settings->get_double("attack0"));

  root.put(section + ".multiband_compressor.subband.release",
           settings->get_double("release0"));

  root.put(section + ".multiband_compressor.subband.makeup",
           settings->get_double("makeup0"));

  root.put(section + ".multiband_compressor.subband.knee",
           settings->get_double("knee0"));

  root.put(section + ".multiband_compressor.subband.detection",
           settings->get_string("detection0"));

  root.put(section + ".multiband_compressor.subband.bypass",
           settings->get_boolean("bypass0"));

  root.put(section + ".multiband_compressor.subband.solo",
           settings->get_boolean("solo0"));

  // low band

  root.put(section + ".multiband_compressor.lowband.threshold",
           settings->get_double("threshold1"));

  root.put(section + ".multiband_compressor.lowband.ratio",
           settings->get_double("ratio1"));

  root.put(section + ".multiband_compressor.lowband.attack",
           settings->get_double("attack1"));

  root.put(section + ".multiband_compressor.lowband.release",
           settings->get_double("release1"));

  root.put(section + ".multiband_compressor.lowband.makeup",
           settings->get_double("makeup1"));

  root.put(section + ".multiband_compressor.lowband.knee",
           settings->get_double("knee1"));

  root.put(section + ".multiband_compressor.lowband.detection",
           settings->get_string("detection1"));

  root.put(section + ".multiband_compressor.lowband.bypass",
           settings->get_boolean("bypass1"));

  root.put(section + ".multiband_compressor.lowband.solo",
           settings->get_boolean("solo1"));

  // mid band

  root.put(section + ".multiband_compressor.midband.threshold",
           settings->get_double("threshold2"));

  root.put(section + ".multiband_compressor.midband.ratio",
           settings->get_double("ratio2"));

  root.put(section + ".multiband_compressor.midband.attack",
           settings->get_double("attack2"));

  root.put(section + ".multiband_compressor.midband.release",
           settings->get_double("release2"));

  root.put(section + ".multiband_compressor.midband.makeup",
           settings->get_double("makeup2"));

  root.put(section + ".multiband_compressor.midband.knee",
           settings->get_double("knee2"));

  root.put(section + ".multiband_compressor.midband.detection",
           settings->get_string("detection2"));

  root.put(section + ".multiband_compressor.midband.bypass",
           settings->get_boolean("bypass2"));

  root.put(section + ".multiband_compressor.midband.solo",
           settings->get_boolean("solo2"));

  // high band

  root.put(section + ".multiband_compressor.highband.threshold",
           settings->get_double("threshold3"));

  root.put(section + ".multiband_compressor.highband.ratio",
           settings->get_double("ratio3"));

  root.put(section + ".multiband_compressor.highband.attack",
           settings->get_double("attack3"));

  root.put(section + ".multiband_compressor.highband.release",
           settings->get_double("release3"));

  root.put(section + ".multiband_compressor.highband.makeup",
           settings->get_double("makeup3"));

  root.put(section + ".multiband_compressor.highband.knee",
           settings->get_double("knee3"));

  root.put(section + ".multiband_compressor.highband.detection",
           settings->get_string("detection3"));

  root.put(section + ".multiband_compressor.highband.bypass",
           settings->get_boolean("bypass3"));

  root.put(section + ".multiband_compressor.highband.solo",
           settings->get_boolean("solo3"));
}

void MultibandCompressorPreset::load(
    boost::property_tree::ptree& root,
    const std::string& section,
    const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state",
                   section + ".multiband_compressor.state");

  update_key<double>(root, settings, "input-gain",
                     section + ".multiband_compressor.input-gain");

  update_key<double>(root, settings, "output-gain",
                     section + ".multiband_compressor.output-gain");

  update_key<double>(root, settings, "freq0",
                     section + ".multiband_compressor.freq0");

  update_key<double>(root, settings, "freq1",
                     section + ".multiband_compressor.freq1");

  update_key<double>(root, settings, "freq2",
                     section + ".multiband_compressor.freq2");

  update_string_key(root, settings, "mode",
                    section + ".multiband_compressor.mode");

  // sub band

  update_key<double>(root, settings, "threshold0",
                     section + ".multiband_compressor.threshold0");

  update_key<double>(root, settings, "ratio0",
                     section + ".multiband_compressor.ratio0");

  // settings->set_double(
  //     "threshold0",
  //     root.get<double>(section + ".multiband_compressor.subband.threshold",
  //                      get_default<double>(settings, "threshold0")));

  // settings->set_double(
  //     "ratio0",
  //     root.get<double>(section + ".multiband_compressor.subband.ratio",
  //                      get_default<double>(settings, "ratio0")));

  settings->set_double(
      "attack0",
      root.get<double>(section + ".multiband_compressor.subband.attack",
                       get_default<double>(settings, "attack0")));

  settings->set_double(
      "release0",
      root.get<double>(section + ".multiband_compressor.subband.release",
                       get_default<double>(settings, "release0")));

  settings->set_double(
      "makeup0",
      root.get<double>(section + ".multiband_compressor.subband.makeup",
                       get_default<double>(settings, "makeup0")));

  settings->set_double(
      "knee0", root.get<double>(section + ".multiband_compressor.subband.knee",
                                get_default<double>(settings, "knee0")));

  settings->set_string(
      "detection0",
      root.get<std::string>(section + ".multiband_compressor.subband.detection",
                            get_default<std::string>(settings, "detection0")));

  settings->set_boolean(
      "bypass0",
      root.get<bool>(section + ".multiband_compressor.subband.bypass",
                     get_default<bool>(settings, "bypass0")));

  settings->set_boolean(
      "solo0", root.get<bool>(section + ".multiband_compressor.subband.solo",
                              get_default<bool>(settings, "solo0")));

  // low band

  settings->set_double(
      "threshold1",
      root.get<double>(section + ".multiband_compressor.lowband.threshold",
                       get_default<double>(settings, "threshold1")));

  settings->set_double(
      "ratio1",
      root.get<double>(section + ".multiband_compressor.lowband.ratio",
                       get_default<double>(settings, "ratio1")));

  settings->set_double(
      "attack1",
      root.get<double>(section + ".multiband_compressor.lowband.attack",
                       get_default<double>(settings, "attack1")));

  settings->set_double(
      "release1",
      root.get<double>(section + ".multiband_compressor.lowband.release",
                       get_default<double>(settings, "release1")));

  settings->set_double(
      "makeup1",
      root.get<double>(section + ".multiband_compressor.lowband.makeup",
                       get_default<double>(settings, "makeup1")));

  settings->set_double(
      "knee1", root.get<double>(section + ".multiband_compressor.lowband.knee",
                                get_default<double>(settings, "knee1")));

  settings->set_string(
      "detection1",
      root.get<std::string>(section + ".multiband_compressor.lowband.detection",
                            get_default<std::string>(settings, "detection1")));

  settings->set_boolean(
      "bypass1",
      root.get<bool>(section + ".multiband_compressor.lowband.bypass",
                     get_default<bool>(settings, "bypass1")));

  settings->set_boolean(
      "solo1", root.get<bool>(section + ".multiband_compressor.lowband.solo",
                              get_default<bool>(settings, "solo1")));

  // mid band

  settings->set_double(
      "threshold2",
      root.get<double>(section + ".multiband_compressor.midband.threshold",
                       get_default<double>(settings, "threshold2")));

  settings->set_double(
      "ratio2",
      root.get<double>(section + ".multiband_compressor.midband.ratio",
                       get_default<double>(settings, "ratio2")));

  settings->set_double(
      "attack2",
      root.get<double>(section + ".multiband_compressor.midband.attack",
                       get_default<double>(settings, "attack2")));

  settings->set_double(
      "release2",
      root.get<double>(section + ".multiband_compressor.midband.release",
                       get_default<double>(settings, "release2")));

  settings->set_double(
      "makeup2",
      root.get<double>(section + ".multiband_compressor.midband.makeup",
                       get_default<double>(settings, "makeup2")));

  settings->set_double(
      "knee2", root.get<double>(section + ".multiband_compressor.midband.knee",
                                get_default<double>(settings, "knee2")));

  settings->set_string(
      "detection2",
      root.get<std::string>(section + ".multiband_compressor.midband.detection",
                            get_default<std::string>(settings, "detection2")));

  settings->set_boolean(
      "bypass2",
      root.get<bool>(section + ".multiband_compressor.midband.bypass",
                     get_default<bool>(settings, "bypass2")));

  settings->set_boolean(
      "solo2", root.get<bool>(section + ".multiband_compressor.midband.solo",
                              get_default<bool>(settings, "solo2")));

  // high band

  settings->set_double(
      "threshold3",
      root.get<double>(section + ".multiband_compressor.highband.threshold",
                       get_default<double>(settings, "threshold3")));

  settings->set_double(
      "ratio3",
      root.get<double>(section + ".multiband_compressor.highband.ratio",
                       get_default<double>(settings, "ratio3")));

  settings->set_double(
      "attack3",
      root.get<double>(section + ".multiband_compressor.highband.attack",
                       get_default<double>(settings, "attack3")));

  settings->set_double(
      "release3",
      root.get<double>(section + ".multiband_compressor.highband.release",
                       get_default<double>(settings, "release3")));

  settings->set_double(
      "makeup3",
      root.get<double>(section + ".multiband_compressor.highband.makeup",
                       get_default<double>(settings, "makeup3")));

  settings->set_double(
      "knee3", root.get<double>(section + ".multiband_compressor.highband.knee",
                                get_default<double>(settings, "knee3")));

  settings->set_string("detection3",
                       root.get<std::string>(
                           section + ".multiband_compressor.highband.detection",
                           get_default<std::string>(settings, "detection3")));

  settings->set_boolean(
      "bypass3",
      root.get<bool>(section + ".multiband_compressor.highband.bypass",
                     get_default<bool>(settings, "bypass3")));

  settings->set_boolean(
      "solo3", root.get<bool>(section + ".multiband_compressor.highband.solo",
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
