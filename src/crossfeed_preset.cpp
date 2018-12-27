#include "crossfeed_preset.hpp"

CrossfeedPreset::CrossfeedPreset()
    : output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.crossfeed")) {}

void CrossfeedPreset::save(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".crossfeed.state", settings->get_boolean("state"));

  root.put(section + ".crossfeed.fcut", settings->get_int("fcut"));

  root.put(section + ".crossfeed.feed", settings->get_double("feed"));
}

void CrossfeedPreset::load(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".crossfeed.state");

  update_key<int>(root, settings, "fcut", section + ".crossfeed.fcut");

  update_key<double>(root, settings, "feed", section + ".crossfeed.feed");

  // settings->set_boolean("state",
  //                       root.get<bool>(section + ".crossfeed.state",
  //                                      get_default<bool>(settings,
  //                                      "state")));

  // settings->set_int("fcut", root.get<int>(section + ".crossfeed.fcut",
  //                                         get_default<int>(settings,
  //                                         "fcut")));
  //
  // settings->set_double("feed",
  //                      root.get<double>(section + ".crossfeed.feed",
  //                                       get_default<double>(settings,
  //                                       "feed")));
}

void CrossfeedPreset::write(boost::property_tree::ptree& root) {
  save(root, "output", output_settings);
}

void CrossfeedPreset::read(boost::property_tree::ptree& root) {
  load(root, "output", output_settings);
}
