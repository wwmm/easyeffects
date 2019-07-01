#include "stereo_tools_preset.hpp"

StereoToolsPreset::StereoToolsPreset()
    : output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.stereotools")) {}

void StereoToolsPreset::save(boost::property_tree::ptree& root,
                             const std::string& section,
                             const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".stereo_tools.state", settings->get_boolean("state"));

  root.put(section + ".stereo_tools.input-gain", settings->get_double("input-gain"));

  root.put(section + ".stereo_tools.output-gain", settings->get_double("output-gain"));

  root.put(section + ".stereo_tools.balance-in", settings->get_double("balance-in"));

  root.put(section + ".stereo_tools.balance-out", settings->get_double("balance-out"));

  root.put(section + ".stereo_tools.softclip", settings->get_boolean("softclip"));

  root.put(section + ".stereo_tools.mutel", settings->get_boolean("mutel"));

  root.put(section + ".stereo_tools.muter", settings->get_boolean("muter"));

  root.put(section + ".stereo_tools.phasel", settings->get_boolean("phasel"));

  root.put(section + ".stereo_tools.phaser", settings->get_boolean("phaser"));

  root.put(section + ".stereo_tools.mode", settings->get_string("mode"));

  root.put(section + ".stereo_tools.side-level", settings->get_double("slev"));

  root.put(section + ".stereo_tools.side-balance", settings->get_double("sbal"));

  root.put(section + ".stereo_tools.middle-level", settings->get_double("mlev"));

  root.put(section + ".stereo_tools.middle-panorama", settings->get_double("mpan"));

  root.put(section + ".stereo_tools.stereo-base", settings->get_double("stereo-base"));

  root.put(section + ".stereo_tools.delay", settings->get_double("delay"));

  root.put(section + ".stereo_tools.sc-level", settings->get_double("sc-level"));

  root.put(section + ".stereo_tools.stereo-phase", settings->get_double("stereo-phase"));
}

void StereoToolsPreset::load(boost::property_tree::ptree& root,
                             const std::string& section,
                             const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".stereo_tools.state");

  update_key<double>(root, settings, "input-gain", section + ".stereo_tools.input-gain");

  update_key<double>(root, settings, "output-gain", section + ".stereo_tools.output-gain");

  update_key<double>(root, settings, "balance-in", section + ".stereo_tools.balance-in");

  update_key<double>(root, settings, "balance-out", section + ".stereo_tools.balance-out");

  update_key<bool>(root, settings, "softclip", section + ".stereo_tools.softclip");

  update_key<bool>(root, settings, "mutel", section + ".stereo_tools.mutel");

  update_key<bool>(root, settings, "muter", section + ".stereo_tools.muter");

  update_key<bool>(root, settings, "phasel", section + ".stereo_tools.phasel");

  update_key<bool>(root, settings, "phaser", section + ".stereo_tools.phaser");

  update_string_key(root, settings, "mode", section + ".stereo_tools.mode");

  update_key<double>(root, settings, "slev", section + ".stereo_tools.side-level");

  update_key<double>(root, settings, "sbal", section + ".stereo_tools.side-balance");

  update_key<double>(root, settings, "mlev", section + ".stereo_tools.middle-level");

  update_key<double>(root, settings, "mpan", section + ".stereo_tools.middle-panorama");

  update_key<double>(root, settings, "stereo-base", section + ".stereo_tools.stereo-base");

  update_key<double>(root, settings, "delay", section + ".stereo_tools.delay");

  update_key<double>(root, settings, "sc-level", section + ".stereo_tools.sc-level");

  update_key<double>(root, settings, "stereo-phase", section + ".stereo_tools.stereo-phase");
}

void StereoToolsPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  }
}

void StereoToolsPreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  }
}
