#include "crossfeed_preset.hpp"

CrossfeedPreset::CrossfeedPreset()
    : output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.crossfeed")) {}

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
}

void CrossfeedPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  }
}

void CrossfeedPreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  }
}
