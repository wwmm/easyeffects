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
  update_key<bool>(root, settings, "state", section + ".compressor.state");

  update_string_key(root, settings, "detection",
                    section + ".compressor.detection");

  update_string_key(root, settings, "stereo-link",
                    section + ".compressor.stereo-link");

  update_key<double>(root, settings, "mix", section + ".compressor.mix");

  update_key<double>(root, settings, "attack", section + ".compressor.attack");

  update_key<double>(root, settings, "release",
                     section + ".compressor.release");

  update_key<double>(root, settings, "threshold",
                     section + ".compressor.threshold");

  update_key<double>(root, settings, "ratio", section + ".compressor.ratio");

  update_key<double>(root, settings, "knee", section + ".compressor.knee");

  update_key<double>(root, settings, "makeup", section + ".compressor.makeup");
}

void CompressorPreset::write(PresetType preset_type,
                             boost::property_tree::ptree& root) {
  save(root, "input", input_settings);
  save(root, "output", output_settings);
}

void CompressorPreset::read(PresetType preset_type,
                            boost::property_tree::ptree& root) {
  load(root, "input", input_settings);
  load(root, "output", output_settings);
}
