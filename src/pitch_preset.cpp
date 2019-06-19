#include "pitch_preset.hpp"

PitchPreset::PitchPreset()
    : input_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs.pitch")),
      output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.pitch")) {}

void PitchPreset::save(boost::property_tree::ptree& root,
                       const std::string& section,
                       const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".pitch.state", settings->get_boolean("state"));

  root.put(section + ".pitch.input-gain", settings->get_double("input-gain"));

  root.put(section + ".pitch.output-gain", settings->get_double("output-gain"));

  root.put(section + ".pitch.cents", settings->get_double("cents"));

  root.put(section + ".pitch.semitones", settings->get_int("semitones"));

  root.put(section + ".pitch.octaves", settings->get_int("octaves"));

  root.put(section + ".pitch.crispness", settings->get_int("crispness"));

  root.put(section + ".pitch.formant-preserving", settings->get_boolean("formant-preserving"));

  root.put(section + ".pitch.faster", settings->get_boolean("faster"));
}

void PitchPreset::load(boost::property_tree::ptree& root,
                       const std::string& section,
                       const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".pitch.state");

  update_key<double>(root, settings, "input-gain", section + ".pitch.input-gain");

  update_key<double>(root, settings, "output-gain", section + ".pitch.output-gain");

  update_key<double>(root, settings, "cents", section + ".pitch.cents");

  update_key<int>(root, settings, "semitones", section + ".pitch.semitones");

  update_key<int>(root, settings, "octaves", section + ".pitch.octaves");

  update_key<int>(root, settings, "crispness", section + ".pitch.crispness");

  update_key<bool>(root, settings, "formant-preserving", section + ".pitch.formant-preserving");

  update_key<bool>(root, settings, "faster", section + ".pitch.faster");
}

void PitchPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  } else {
    save(root, "input", input_settings);
  }
}

void PitchPreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  } else {
    load(root, "input", input_settings);
  }
}
