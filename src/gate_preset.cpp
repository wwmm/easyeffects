#include "gate_preset.hpp"

GatePreset::GatePreset()
    : input_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs.gate")),
      output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.gate")) {}

void GatePreset::save(boost::property_tree::ptree& root,
                      const std::string& section,
                      const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".gate.state", settings->get_boolean("state"));

  root.put(section + ".gate.detection", settings->get_string("detection"));

  root.put(section + ".gate.stereo-link", settings->get_string("stereo-link"));

  root.put(section + ".gate.range", settings->get_double("range"));

  root.put(section + ".gate.attack", settings->get_double("attack"));

  root.put(section + ".gate.release", settings->get_double("release"));

  root.put(section + ".gate.threshold", settings->get_double("threshold"));

  root.put(section + ".gate.ratio", settings->get_double("ratio"));

  root.put(section + ".gate.knee", settings->get_double("knee"));

  root.put(section + ".gate.makeup", settings->get_double("makeup"));
}

void GatePreset::load(boost::property_tree::ptree& root,
                      const std::string& section,
                      const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".gate.state");

  update_string_key(root, settings, "detection", section + ".gate.detection");

  update_string_key(root, settings, "stereo-link", section + ".gate.stereo-link");

  update_key<double>(root, settings, "range", section + ".gate.range");

  update_key<double>(root, settings, "attack", section + ".gate.attack");

  update_key<double>(root, settings, "release", section + ".gate.release");

  update_key<double>(root, settings, "threshold", section + ".gate.threshold");

  update_key<double>(root, settings, "ratio", section + ".gate.ratio");

  update_key<double>(root, settings, "knee", section + ".gate.knee");

  update_key<double>(root, settings, "makeup", section + ".gate.makeup");
}

void GatePreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  } else {
    save(root, "input", input_settings);
  }
}

void GatePreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  } else {
    load(root, "input", input_settings);
  }
}
