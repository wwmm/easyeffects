#include "equalizer_preset.hpp"
#include "util.hpp"

EqualizerPreset::EqualizerPreset()
    : input_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.equalizer")),
      input_settings_left(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.equalizer.leftchannel")),
      input_settings_right(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.equalizer.rightchannel")),
      output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.equalizer")),
      output_settings_left(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.equalizer.leftchannel")),
      output_settings_right(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.equalizer.rightchannel")) {}

void EqualizerPreset::save(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".equalizer.state", settings->get_boolean("state"));

  int nbands = settings->get_int("num-bands");

  root.put(section + ".equalizer.num-bands", nbands);

  root.put(section + ".equalizer.input-gain",
           settings->get_double("input-gain"));

  root.put(section + ".equalizer.output-gain",
           settings->get_double("output-gain"));

  root.put(section + ".equalizer.split-channels",
           settings->get_boolean("split-channels"));

  //// remove this

  // for (int n = 0; n < nbands; n++) {
  //   root.put(section + ".equalizer.band" + std::to_string(n) + ".gain",
  //            settings->get_double(
  //                std::string("band" + std::to_string(n) + "-gain")));
  //
  //   root.put(section + ".equalizer.band" + std::to_string(n) + ".frequency",
  //            settings->get_double(
  //                std::string("band" + std::to_string(n) + "-frequency")));
  //
  //   root.put(section + ".equalizer.band" + std::to_string(n) + ".width",
  //            settings->get_double(
  //                std::string("band" + std::to_string(n) + "-width")));
  //
  //   root.put(section + ".equalizer.band" + std::to_string(n) + ".type",
  //            settings->get_string(
  //                std::string("band" + std::to_string(n) + "-type")));
  // }

  /////////

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
    root.put(section + ".band" + std::to_string(n) + ".gain",
             settings->get_double(
                 std::string("band" + std::to_string(n) + "-gain")));

    root.put(section + ".band" + std::to_string(n) + ".frequency",
             settings->get_double(
                 std::string("band" + std::to_string(n) + "-frequency")));

    root.put(section + ".band" + std::to_string(n) + ".width",
             settings->get_double(
                 std::string("band" + std::to_string(n) + "-width")));

    root.put(section + ".band" + std::to_string(n) + ".type",
             settings->get_string(
                 std::string("band" + std::to_string(n) + "-type")));
  }
}

void EqualizerPreset::load(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".equalizer.state");

  update_key<int>(root, settings, "num-bands",
                  section + ".equalizer.num-bands");

  update_key<double>(root, settings, "input-gain",
                     section + ".equalizer.input-gain");

  update_key<double>(root, settings, "output-gain",
                     section + ".equalizer.output-gain");

  int nbands = settings->get_int("num-bands");

  // for (int n = 0; n < nbands; n++) {
  //   update_key<double>(
  //       root, settings, std::string("band" + std::to_string(n) + "-gain"),
  //       section + ".equalizer.band" + std::to_string(n) + ".gain");
  //
  //   update_key<double>(
  //       root, settings, std::string("band" + std::to_string(n) +
  //       "-frequency"), section + ".equalizer.band" + std::to_string(n) +
  //       ".frequency");
  //
  //   update_key<double>(
  //       root, settings, std::string("band" + std::to_string(n) + "-width"),
  //       section + ".equalizer.band" + std::to_string(n) + ".width");
  //
  //   update_string_key(
  //       root, settings, std::string("band" + std::to_string(n) + "-type"),
  //       section + ".equalizer.band" + std::to_string(n) + ".type");
  // }

  bool legacy_preset = false;

  try {
    root.get<bool>(section + ".equalizer.split-channels");
  } catch (const boost::property_tree::ptree_error& e) {
    util::warning(log_tag + "old preset format detected");

    legacy_preset = true;
  }

  update_key<bool>(root, settings, "split-channels",
                   section + ".equalizer.split-channels");

  if (!legacy_preset) {
    if (section == std::string("input")) {
      load_channel(root, "input.equalizer.left", input_settings_left, nbands);
      load_channel(root, "input.equalizer.right", input_settings_right, nbands);
    } else if (section == std::string("output")) {
      load_channel(root, "output.equalizer.left", output_settings_left, nbands);
      load_channel(root, "output.equalizer.right", output_settings_right,
                   nbands);
    }
  } else {
    if (section == std::string("input")) {
      load_channel(root, "input.equalizer", input_settings_left, nbands);
      load_channel(root, "input.equalizer", input_settings_right, nbands);
    } else if (section == std::string("output")) {
      load_channel(root, "output.equalizer", output_settings_left, nbands);
      load_channel(root, "output.equalizer", output_settings_right, nbands);
    }
  }
}

void EqualizerPreset::load_channel(boost::property_tree::ptree& root,
                                   const std::string& section,
                                   const Glib::RefPtr<Gio::Settings>& settings,
                                   const int& nbands) {
  for (int n = 0; n < nbands; n++) {
    update_key<double>(root, settings,
                       std::string("band" + std::to_string(n) + "-gain"),
                       section + ".band" + std::to_string(n) + ".gain");

    update_key<double>(root, settings,
                       std::string("band" + std::to_string(n) + "-frequency"),
                       section + ".band" + std::to_string(n) + ".frequency");

    update_key<double>(root, settings,
                       std::string("band" + std::to_string(n) + "-width"),
                       section + ".band" + std::to_string(n) + ".width");

    update_string_key(root, settings,
                      std::string("band" + std::to_string(n) + "-type"),
                      section + ".band" + std::to_string(n) + ".type");
  }
}

void EqualizerPreset::write(boost::property_tree::ptree& root) {
  save(root, "input", input_settings);
  save(root, "output", output_settings);
}

void EqualizerPreset::read(boost::property_tree::ptree& root) {
  load(root, "input", input_settings);
  load(root, "output", output_settings);
}
