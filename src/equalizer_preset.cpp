#include "equalizer_preset.hpp"
#include "util.hpp"

EqualizerPreset::EqualizerPreset()
    : input_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs.equalizer")),
      input_settings_left(Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs.equalizer.leftchannel")),
      input_settings_right(Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs.equalizer.rightchannel")),
      output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.equalizer")),
      output_settings_left(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.equalizer.leftchannel")),
      output_settings_right(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.equalizer.rightchannel")) {}

void EqualizerPreset::save(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".equalizer.state", settings->get_boolean("state"));

  root.put(section + ".equalizer.mode", settings->get_string("mode"));

  int nbands = settings->get_int("num-bands");

  root.put(section + ".equalizer.num-bands", nbands);

  root.put(section + ".equalizer.input-gain", settings->get_double("input-gain"));

  root.put(section + ".equalizer.output-gain", settings->get_double("output-gain"));

  root.put(section + ".equalizer.split-channels", settings->get_boolean("split-channels"));

  if (section == std::string("input")) {
    save_channel(root, "input.equalizer.left", input_settings_left, nbands);
    save_channel(root, "input.equalizer.right", input_settings_right, nbands);
  } else if (section == std::string("output")) {
    save_channel(root, "output.equalizer.left", output_settings_left, nbands);
    save_channel(root, "output.equalizer.right", output_settings_right, nbands);
  }
}

void EqualizerPreset::save_channel(boost::property_tree::ptree& root,
                                   const std::string& section,
                                   const Glib::RefPtr<Gio::Settings>& settings,
                                   const int& nbands) {
  for (int n = 0; n < nbands; n++) {
    root.put(section + ".band" + std::to_string(n) + ".type",
             settings->get_string(std::string("band" + std::to_string(n) + "-type")));

    root.put(section + ".band" + std::to_string(n) + ".mode",
             settings->get_string(std::string("band" + std::to_string(n) + "-mode")));

    root.put(section + ".band" + std::to_string(n) + ".slope",
             settings->get_string(std::string("band" + std::to_string(n) + "-slope")));

    root.put(section + ".band" + std::to_string(n) + ".solo",
             settings->get_boolean(std::string("band" + std::to_string(n) + "-solo")));

    root.put(section + ".band" + std::to_string(n) + ".mute",
             settings->get_boolean(std::string("band" + std::to_string(n) + "-mute")));

    root.put(section + ".band" + std::to_string(n) + ".gain",
             settings->get_double(std::string("band" + std::to_string(n) + "-gain")));

    root.put(section + ".band" + std::to_string(n) + ".frequency",
             settings->get_double(std::string("band" + std::to_string(n) + "-frequency")));

    root.put(section + ".band" + std::to_string(n) + ".q",
             settings->get_double(std::string("band" + std::to_string(n) + "-q")));
  }
}

void EqualizerPreset::load(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".equalizer.state");

  update_string_key(root, settings, "mode", section + ".equalizer.mode");

  update_key<int>(root, settings, "num-bands", section + ".equalizer.num-bands");

  update_key<double>(root, settings, "input-gain", section + ".equalizer.input-gain");

  update_key<double>(root, settings, "output-gain", section + ".equalizer.output-gain");

  int nbands = settings->get_int("num-bands");

  update_key<bool>(root, settings, "split-channels", section + ".equalizer.split-channels");

  if (section == std::string("input")) {
    load_channel(root, "input.equalizer.left", input_settings_left, nbands);
    load_channel(root, "input.equalizer.right", input_settings_right, nbands);
  } else if (section == std::string("output")) {
    load_channel(root, "output.equalizer.left", output_settings_left, nbands);
    load_channel(root, "output.equalizer.right", output_settings_right, nbands);
  }
}

void EqualizerPreset::load_channel(boost::property_tree::ptree& root,
                                   const std::string& section,
                                   const Glib::RefPtr<Gio::Settings>& settings,
                                   const int& nbands) {
  for (int n = 0; n < nbands; n++) {
    update_string_key(root, settings, std::string("band" + std::to_string(n) + "-type"),
                      section + ".band" + std::to_string(n) + ".type");

    update_string_key(root, settings, std::string("band" + std::to_string(n) + "-mode"),
                      section + ".band" + std::to_string(n) + ".mode");

    update_string_key(root, settings, std::string("band" + std::to_string(n) + "-slope"),
                      section + ".band" + std::to_string(n) + ".slope");

    update_key<bool>(root, settings, std::string("band" + std::to_string(n) + "-solo"),
                     section + ".band" + std::to_string(n) + ".solo");

    update_key<bool>(root, settings, std::string("band" + std::to_string(n) + "-mute"),
                     section + ".band" + std::to_string(n) + ".mute");

    update_key<double>(root, settings, std::string("band" + std::to_string(n) + "-gain"),
                       section + ".band" + std::to_string(n) + ".gain");

    update_key<double>(root, settings, std::string("band" + std::to_string(n) + "-frequency"),
                       section + ".band" + std::to_string(n) + ".frequency");

    update_key<double>(root, settings, std::string("band" + std::to_string(n) + "-q"),
                       section + ".band" + std::to_string(n) + ".q");
  }
}

void EqualizerPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  } else {
    save(root, "input", input_settings);
  }
}

void EqualizerPreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  } else {
    load(root, "input", input_settings);
  }
}
