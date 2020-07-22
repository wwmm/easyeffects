#include "compressor_preset.hpp"

CompressorPreset::CompressorPreset()
    : input_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs.compressor")),
      output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.compressor")) {}

void CompressorPreset::save(boost::property_tree::ptree& root,
                            const std::string& section,
                            const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".compressor.state", settings->get_boolean("state"));

  root.put(section + ".compressor.input-gain", settings->get_double("input-gain"));

  root.put(section + ".compressor.output-gain", settings->get_double("output-gain"));

  root.put(section + ".compressor.mode", settings->get_string("mode"));

  root.put(section + ".compressor.attack", settings->get_double("attack"));

  root.put(section + ".compressor.release", settings->get_double("release"));

  root.put(section + ".compressor.release-threshold", settings->get_double("release-threshold"));

  root.put(section + ".compressor.threshold", settings->get_double("threshold"));

  root.put(section + ".compressor.ratio", settings->get_double("ratio"));

  root.put(section + ".compressor.knee", settings->get_double("knee"));

  root.put(section + ".compressor.makeup", settings->get_double("makeup"));

  root.put(section + ".compressor.boost-threshold", settings->get_double("boost-threshold"));

  root.put(section + ".compressor.sidechain.listen", settings->get_boolean("sidechain-listen"));

  root.put(section + ".compressor.sidechain.type", settings->get_string("sidechain-type"));

  root.put(section + ".compressor.sidechain.mode", settings->get_string("sidechain-mode"));

  root.put(section + ".compressor.sidechain.source", settings->get_string("sidechain-source"));

  root.put(section + ".compressor.sidechain.preamp", settings->get_double("sidechain-preamp"));

  root.put(section + ".compressor.sidechain.reactivity", settings->get_double("sidechain-reactivity"));

  root.put(section + ".compressor.sidechain.lookahead", settings->get_double("sidechain-lookahead"));

  root.put(section + ".compressor.hpf-mode", settings->get_string("hpf-mode"));

  root.put(section + ".compressor.hpf-frequency", settings->get_double("hpf-frequency"));

  root.put(section + ".compressor.lpf-mode", settings->get_string("lpf-mode"));

  root.put(section + ".compressor.lpf-frequency", settings->get_double("lpf-frequency"));
}

void CompressorPreset::load(const boost::property_tree::ptree& root,
                            const std::string& section,
                            const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".compressor.state");

  update_key<double>(root, settings, "input-gain", section + ".compressor.input-gain");

  update_key<double>(root, settings, "output-gain", section + ".compressor.output-gain");

  update_string_key(root, settings, "mode", section + ".compressor.mode");

  update_key<double>(root, settings, "attack", section + ".compressor.attack");

  update_key<double>(root, settings, "release", section + ".compressor.release");

  update_key<double>(root, settings, "release-threshold", section + ".compressor.release-threshold");

  update_key<double>(root, settings, "threshold", section + ".compressor.threshold");

  update_key<double>(root, settings, "ratio", section + ".compressor.ratio");

  update_key<double>(root, settings, "knee", section + ".compressor.knee");

  update_key<double>(root, settings, "makeup", section + ".compressor.makeup");

  update_key<double>(root, settings, "boost-threshold", section + ".compressor.boost-threshold");

  update_key<bool>(root, settings, "sidechain-listen", section + ".compressor.sidechain.listen");

  update_string_key(root, settings, "sidechain-type", section + ".compressor.sidechain.type");

  update_string_key(root, settings, "sidechain-mode", section + ".compressor.sidechain.mode");

  update_string_key(root, settings, "sidechain-source", section + ".compressor.sidechain.source");

  update_key<double>(root, settings, "sidechain-preamp", section + ".compressor.sidechain.preamp");

  update_key<double>(root, settings, "sidechain-reactivity", section + ".compressor.sidechain.reactivity");

  update_key<double>(root, settings, "sidechain-lookahead", section + ".compressor.sidechain.lookahead");

  update_string_key(root, settings, "hpf-mode", section + ".compressor.hpf-mode");

  update_key<double>(root, settings, "hpf-frequency", section + ".compressor.hpf-frequency");

  update_string_key(root, settings, "lpf-mode", section + ".compressor.lpf-mode");

  update_key<double>(root, settings, "lpf-frequency", section + ".compressor.lpf-frequency");
}

void CompressorPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  } else {
    save(root, "input", input_settings);
  }
}

void CompressorPreset::read(PresetType preset_type, const boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  } else {
    load(root, "input", input_settings);
  }
}
