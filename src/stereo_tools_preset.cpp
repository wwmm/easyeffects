#include "stereo_tools_preset.hpp"

StereoToolsPreset::StereoToolsPreset()
    : output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.stereotools")) {}

void StereoToolsPreset::save(boost::property_tree::ptree& root,
                             const std::string& section,
                             const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".stereo_tools.state", settings->get_boolean("state"));

  root.put(section + ".stereo_tools.input-gain",
           settings->get_double("input-gain"));

  root.put(section + ".stereo_tools.output-gain",
           settings->get_double("output-gain"));

  root.put(section + ".stereo_tools.balance-in",
           settings->get_double("balance-in"));

  root.put(section + ".stereo_tools.balance-out",
           settings->get_double("balance-out"));

  root.put(section + ".stereo_tools.softclip",
           settings->get_boolean("softclip"));

  root.put(section + ".stereo_tools.mutel", settings->get_boolean("mutel"));

  root.put(section + ".stereo_tools.muter", settings->get_boolean("muter"));

  root.put(section + ".stereo_tools.phasel", settings->get_boolean("phasel"));

  root.put(section + ".stereo_tools.phaser", settings->get_boolean("phaser"));

  root.put(section + ".stereo_tools.mode", settings->get_string("mode"));

  root.put(section + ".stereo_tools.side-level", settings->get_double("slev"));

  root.put(section + ".stereo_tools.side-balance",
           settings->get_double("sbal"));

  root.put(section + ".stereo_tools.middle-level",
           settings->get_double("mlev"));

  root.put(section + ".stereo_tools.middle-panorama",
           settings->get_double("mpan"));

  root.put(section + ".stereo_tools.stereo-base",
           settings->get_double("stereo-base"));

  root.put(section + ".stereo_tools.delay", settings->get_double("delay"));

  root.put(section + ".stereo_tools.sc-level",
           settings->get_double("sc-level"));

  root.put(section + ".stereo_tools.stereo-phase",
           settings->get_double("stereo-phase"));
}

void StereoToolsPreset::load(boost::property_tree::ptree& root,
                             const std::string& section,
                             const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".stereo_tools.state");

  update_key<double>(root, settings, "input-gain",
                     section + ".stereo_tools.input-gain");

  update_key<double>(root, settings, "output-gain",
                     section + ".stereo_tools.output-gain");

  update_key<double>(root, settings, "balance-in",
                     section + ".stereo_tools.balance-in");

  update_key<double>(root, settings, "balance-out",
                     section + ".stereo_tools.balance-out");

  update_key<bool>(root, settings, "softclip",
                   section + ".stereo_tools.softclip");

  update_key<bool>(root, settings, "mutel", section + ".stereo_tools.mutel");

  update_key<bool>(root, settings, "muter", section + ".stereo_tools.muter");

  update_key<bool>(root, settings, "phasel", section + ".stereo_tools.phasel");

  update_key<bool>(root, settings, "phaser", section + ".stereo_tools.phaser");

  // settings->set_boolean("state",
  //                       root.get<bool>(section + ".stereo_tools.state",
  //                                      get_default<bool>(settings,
  //                                      "state")));

  // settings->set_double(
  //     "input-gain",
  //     root.get<double>(section + ".stereo_tools.input-gain",
  //                      get_default<double>(settings, "input-gain")));
  //
  // settings->set_double(
  //     "output-gain",
  //     root.get<double>(section + ".stereo_tools.output-gain",
  //                      get_default<double>(settings, "output-gain")));

  // settings->set_double(
  //     "balance-in",
  //     root.get<double>(section + ".stereo_tools.balance-in",
  //                      get_default<double>(settings, "balance-in")));

  // settings->set_double(
  //     "balance-out",
  //     root.get<double>(section + ".stereo_tools.balance-out",
  //                      get_default<double>(settings, "balance-out")));

  // settings->set_boolean(
  //     "softclip", root.get<bool>(section + ".stereo_tools.softclip",
  //                                get_default<bool>(settings, "softclip")));

  // settings->set_boolean("mutel",
  //                       root.get<bool>(section + ".stereo_tools.mutel",
  //                                      get_default<bool>(settings,
  //                                      "mutel")));

  // settings->set_boolean("muter",
  //                       root.get<bool>(section + ".stereo_tools.muter",
  //                                      get_default<bool>(settings,
  //                                      "muter")));

  // settings->set_boolean("phasel",
  //                       root.get<bool>(section + ".stereo_tools.phasel",
  //                                      get_default<bool>(settings,
  //                                      "phasel")));
  //
  // settings->set_boolean("phaser",
  //                       root.get<bool>(section + ".stereo_tools.phaser",
  //                                      get_default<bool>(settings,
  //                                      "phaser")));

  settings->set_string("mode", root.get<std::string>(
                                   section + ".stereo_tools.mode",
                                   get_default<std::string>(settings, "mode")));

  settings->set_double("slev",
                       root.get<double>(section + ".stereo_tools.side-level",
                                        get_default<double>(settings, "slev")));

  settings->set_double("sbal",
                       root.get<double>(section + ".stereo_tools.side-balance",
                                        get_default<double>(settings, "sbal")));

  settings->set_double("mlev",
                       root.get<double>(section + ".stereo_tools.middle-level",
                                        get_default<double>(settings, "mlev")));

  settings->set_double(
      "mpan", root.get<double>(section + ".stereo_tools.middle-panorama",
                               get_default<double>(settings, "mpan")));

  settings->set_double(
      "stereo-base",
      root.get<double>(section + ".stereo_tools.stereo-base",
                       get_default<double>(settings, "stereo-base")));

  settings->set_double(
      "delay", root.get<double>(section + ".stereo_tools.delay",
                                get_default<double>(settings, "delay")));

  settings->set_double(
      "sc-level", root.get<double>(section + ".stereo_tools.sc-level",
                                   get_default<double>(settings, "sc-level")));

  settings->set_double(
      "stereo-phase",
      root.get<double>(section + ".stereo_tools.stereo-phase",
                       get_default<double>(settings, "stereo-phase")));
}

void StereoToolsPreset::write(boost::property_tree::ptree& root) {
  save(root, "output", output_settings);
}

void StereoToolsPreset::read(boost::property_tree::ptree& root) {
  load(root, "output", output_settings);
}
