#include "gate_preset.hpp"

GatePreset::GatePreset()
    : input_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.gate")),
      output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.gate")) {}

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
  settings->set_boolean("state",
                        root.get<bool>(section + ".gate.state",
                                       get_default<bool>(settings, "state")));

  settings->set_string(
      "detection",
      root.get<std::string>(section + ".gate.detection",
                            get_default<std::string>(settings, "detection")));

  settings->set_string(
      "stereo-link",
      root.get<std::string>(section + ".gate.stereo-link",
                            get_default<std::string>(settings, "stereo-link")));

  settings->set_double(
      "range", root.get<double>(section + ".gate.range",
                                get_default<double>(settings, "range")));

  settings->set_double(
      "attack", root.get<double>(section + ".gate.attack",
                                 get_default<double>(settings, "attack")));

  settings->set_double(
      "release", root.get<double>(section + ".gate.release",
                                  get_default<double>(settings, "release")));

  settings->set_double(
      "threshold",
      root.get<double>(section + ".gate.threshold",
                       get_default<double>(settings, "threshold")));

  settings->set_double(
      "ratio", root.get<double>(section + ".gate.ratio",
                                get_default<double>(settings, "ratio")));

  settings->set_double("knee",
                       root.get<double>(section + ".gate.knee",
                                        get_default<double>(settings, "knee")));

  settings->set_double(
      "makeup", root.get<double>(section + ".gate.makeup",
                                 get_default<double>(settings, "makeup")));
}

void GatePreset::write(boost::property_tree::ptree& root) {
  save(root, "input", input_settings);
  save(root, "output", output_settings);
}

void GatePreset::read(boost::property_tree::ptree& root) {
  load(root, "input", input_settings);
  load(root, "output", output_settings);
}
