#include "multiband_compressor_preset.hpp"

MultibandCompressorPreset::MultibandCompressorPreset()
    : input_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs.multibandcompressor")),
      output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.multibandcompressor")) {}

void MultibandCompressorPreset::save(boost::property_tree::ptree& root,
                                     const std::string& section,
                                     const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".multiband_compressor.state", settings->get_boolean("state"));

  root.put(section + ".multiband_compressor.input-gain", settings->get_double("input-gain"));

  root.put(section + ".multiband_compressor.output-gain", settings->get_double("output-gain"));

  root.put(section + ".multiband_compressor.freq0", settings->get_double("freq0"));

  root.put(section + ".multiband_compressor.freq1", settings->get_double("freq1"));

  root.put(section + ".multiband_compressor.freq2", settings->get_double("freq2"));

  root.put(section + ".multiband_compressor.mode", settings->get_string("mode"));

  // sub band

  root.put(section + ".multiband_compressor.subband.threshold", settings->get_double("threshold0"));

  root.put(section + ".multiband_compressor.subband.ratio", settings->get_double("ratio0"));

  root.put(section + ".multiband_compressor.subband.attack", settings->get_double("attack0"));

  root.put(section + ".multiband_compressor.subband.release", settings->get_double("release0"));

  root.put(section + ".multiband_compressor.subband.makeup", settings->get_double("makeup0"));

  root.put(section + ".multiband_compressor.subband.knee", settings->get_double("knee0"));

  root.put(section + ".multiband_compressor.subband.detection", settings->get_string("detection0"));

  root.put(section + ".multiband_compressor.subband.bypass", settings->get_boolean("bypass0"));

  root.put(section + ".multiband_compressor.subband.solo", settings->get_boolean("solo0"));

  // low band

  root.put(section + ".multiband_compressor.lowband.threshold", settings->get_double("threshold1"));

  root.put(section + ".multiband_compressor.lowband.ratio", settings->get_double("ratio1"));

  root.put(section + ".multiband_compressor.lowband.attack", settings->get_double("attack1"));

  root.put(section + ".multiband_compressor.lowband.release", settings->get_double("release1"));

  root.put(section + ".multiband_compressor.lowband.makeup", settings->get_double("makeup1"));

  root.put(section + ".multiband_compressor.lowband.knee", settings->get_double("knee1"));

  root.put(section + ".multiband_compressor.lowband.detection", settings->get_string("detection1"));

  root.put(section + ".multiband_compressor.lowband.bypass", settings->get_boolean("bypass1"));

  root.put(section + ".multiband_compressor.lowband.solo", settings->get_boolean("solo1"));

  // mid band

  root.put(section + ".multiband_compressor.midband.threshold", settings->get_double("threshold2"));

  root.put(section + ".multiband_compressor.midband.ratio", settings->get_double("ratio2"));

  root.put(section + ".multiband_compressor.midband.attack", settings->get_double("attack2"));

  root.put(section + ".multiband_compressor.midband.release", settings->get_double("release2"));

  root.put(section + ".multiband_compressor.midband.makeup", settings->get_double("makeup2"));

  root.put(section + ".multiband_compressor.midband.knee", settings->get_double("knee2"));

  root.put(section + ".multiband_compressor.midband.detection", settings->get_string("detection2"));

  root.put(section + ".multiband_compressor.midband.bypass", settings->get_boolean("bypass2"));

  root.put(section + ".multiband_compressor.midband.solo", settings->get_boolean("solo2"));

  // high band

  root.put(section + ".multiband_compressor.highband.threshold", settings->get_double("threshold3"));

  root.put(section + ".multiband_compressor.highband.ratio", settings->get_double("ratio3"));

  root.put(section + ".multiband_compressor.highband.attack", settings->get_double("attack3"));

  root.put(section + ".multiband_compressor.highband.release", settings->get_double("release3"));

  root.put(section + ".multiband_compressor.highband.makeup", settings->get_double("makeup3"));

  root.put(section + ".multiband_compressor.highband.knee", settings->get_double("knee3"));

  root.put(section + ".multiband_compressor.highband.detection", settings->get_string("detection3"));

  root.put(section + ".multiband_compressor.highband.bypass", settings->get_boolean("bypass3"));

  root.put(section + ".multiband_compressor.highband.solo", settings->get_boolean("solo3"));
}

void MultibandCompressorPreset::load(boost::property_tree::ptree& root,
                                     const std::string& section,
                                     const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".multiband_compressor.state");

  update_key<double>(root, settings, "input-gain", section + ".multiband_compressor.input-gain");

  update_key<double>(root, settings, "output-gain", section + ".multiband_compressor.output-gain");

  update_key<double>(root, settings, "freq0", section + ".multiband_compressor.freq0");

  update_key<double>(root, settings, "freq1", section + ".multiband_compressor.freq1");

  update_key<double>(root, settings, "freq2", section + ".multiband_compressor.freq2");

  update_string_key(root, settings, "mode", section + ".multiband_compressor.mode");

  // sub band

  update_key<double>(root, settings, "threshold0", section + ".multiband_compressor.subband.threshold");

  update_key<double>(root, settings, "ratio0", section + ".multiband_compressor.subband.ratio");

  update_key<double>(root, settings, "attack0", section + ".multiband_compressor.subband.attack");

  update_key<double>(root, settings, "release0", section + ".multiband_compressor.subband.release");

  update_key<double>(root, settings, "makeup0", section + ".multiband_compressor.subband.makeup");

  update_key<double>(root, settings, "knee0", section + ".multiband_compressor.subband.knee");

  update_string_key(root, settings, "detection0", section + ".multiband_compressor.subband.detection");

  update_key<bool>(root, settings, "bypass0", section + ".multiband_compressor.subband.bypass");

  update_key<bool>(root, settings, "solo0", section + ".multiband_compressor.subband.solo");

  // low band

  update_key<double>(root, settings, "threshold1", section + ".multiband_compressor.lowband.threshold");

  update_key<double>(root, settings, "ratio1", section + ".multiband_compressor.lowband.ratio");

  update_key<double>(root, settings, "attack1", section + ".multiband_compressor.lowband.attack");

  update_key<double>(root, settings, "release1", section + ".multiband_compressor.lowband.release");

  update_key<double>(root, settings, "makeup1", section + ".multiband_compressor.lowband.makeup");

  update_key<double>(root, settings, "knee1", section + ".multiband_compressor.lowband.knee");

  update_string_key(root, settings, "detection1", section + ".multiband_compressor.lowband.detection");

  update_key<bool>(root, settings, "bypass1", section + ".multiband_compressor.lowband.bypass");

  update_key<bool>(root, settings, "solo1", section + ".multiband_compressor.lowband.solo");

  // mid band

  update_key<double>(root, settings, "threshold2", section + ".multiband_compressor.midband.threshold");

  update_key<double>(root, settings, "ratio2", section + ".multiband_compressor.midband.ratio");

  update_key<double>(root, settings, "attack2", section + ".multiband_compressor.midband.attack");

  update_key<double>(root, settings, "release2", section + ".multiband_compressor.midband.release");

  update_key<double>(root, settings, "makeup2", section + ".multiband_compressor.midband.makeup");

  update_key<double>(root, settings, "knee2", section + ".multiband_compressor.midband.knee");

  update_string_key(root, settings, "detection2", section + ".multiband_compressor.midband.detection");

  update_key<bool>(root, settings, "bypass2", section + ".multiband_compressor.midband.bypass");

  update_key<bool>(root, settings, "solo2", section + ".multiband_compressor.midband.solo");

  // high band

  update_key<double>(root, settings, "threshold3", section + ".multiband_compressor.highband.threshold");

  update_key<double>(root, settings, "ratio3", section + ".multiband_compressor.highband.ratio");

  update_key<double>(root, settings, "attack3", section + ".multiband_compressor.highband.attack");

  update_key<double>(root, settings, "release3", section + ".multiband_compressor.highband.release");

  update_key<double>(root, settings, "makeup3", section + ".multiband_compressor.highband.makeup");

  update_key<double>(root, settings, "knee3", section + ".multiband_compressor.highband.knee");

  update_string_key(root, settings, "detection3", section + ".multiband_compressor.highband.detection");

  update_key<bool>(root, settings, "bypass3", section + ".multiband_compressor.highband.bypass");

  update_key<bool>(root, settings, "solo3", section + ".multiband_compressor.highband.solo");
}

void MultibandCompressorPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  } else {
    save(root, "input", input_settings);
  }
}

void MultibandCompressorPreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  } else {
    load(root, "input", input_settings);
  }
}
