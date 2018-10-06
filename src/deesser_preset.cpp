#include "deesser_preset.hpp"

DeesserPreset::DeesserPreset()
    : input_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.deesser")),
      output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.deesser")) {}

void DeesserPreset::save(boost::property_tree::ptree& root,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".deesser.state", settings->get_boolean("state"));

  root.put(section + ".deesser.detection", settings->get_string("detection"));

  root.put(section + ".deesser.mode", settings->get_string("mode"));

  root.put(section + ".deesser.threshold", settings->get_double("threshold"));

  root.put(section + ".deesser.ratio", settings->get_double("ratio"));

  root.put(section + ".deesser.laxity", settings->get_int("laxity"));

  root.put(section + ".deesser.makeup", settings->get_double("makeup"));

  root.put(section + ".deesser.f1-freq", settings->get_double("f1-freq"));

  root.put(section + ".deesser.f2-freq", settings->get_double("f2-freq"));

  root.put(section + ".deesser.f1-level", settings->get_double("f1-level"));

  root.put(section + ".deesser.f2-level", settings->get_double("f2-level"));

  root.put(section + ".deesser.f2-q", settings->get_double("f2-q"));

  root.put(section + ".deesser.sc-listen", settings->get_boolean("sc-listen"));
}

void DeesserPreset::load(boost::property_tree::ptree& root,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  settings->set_boolean("state",
                        root.get<bool>(section + ".deesser.state",
                                       get_default<bool>(settings, "state")));

  settings->set_string(
      "detection",
      root.get<std::string>(section + ".deesser.detection",
                            get_default<std::string>(settings, "detection")));

  settings->set_string("mode", root.get<std::string>(
                                   section + ".deesser.mode",
                                   get_default<std::string>(settings, "mode")));

  settings->set_double(
      "threshold",
      root.get<double>(section + ".deesser.threshold",
                       get_default<double>(settings, "threshold")));

  settings->set_double(
      "ratio", root.get<double>(section + ".deesser.ratio",
                                get_default<double>(settings, "ratio")));

  settings->set_int("laxity",
                    root.get<int>(section + ".deesser.laxity",
                                  get_default<int>(settings, "laxity")));

  settings->set_double(
      "makeup", root.get<double>(section + ".deesser.makeup",
                                 get_default<double>(settings, "makeup")));

  settings->set_double(
      "f1-freq", root.get<double>(section + ".deesser.f1-freq",
                                  get_default<double>(settings, "f1-freq")));

  settings->set_double(
      "f2-freq", root.get<double>(section + ".deesser.f2-freq",
                                  get_default<double>(settings, "f2-freq")));

  settings->set_double(
      "f1-level", root.get<double>(section + ".deesser.f1-level",
                                   get_default<double>(settings, "f1-level")));

  settings->set_double(
      "f2-level", root.get<double>(section + ".deesser.f2-level",
                                   get_default<double>(settings, "f2-level")));

  settings->set_double("f2-q",
                       root.get<double>(section + ".deesser.f2-q",
                                        get_default<double>(settings, "f2-q")));

  settings->set_boolean(
      "sc-listen", root.get<bool>(section + ".deesser.sc-listen",
                                  get_default<bool>(settings, "sc-listen")));
}

void DeesserPreset::write(boost::property_tree::ptree& root) {
  save(root, "input", input_settings);
  save(root, "output", output_settings);
}

void DeesserPreset::read(boost::property_tree::ptree& root) {
  load(root, "input", input_settings);
  load(root, "output", output_settings);
}
