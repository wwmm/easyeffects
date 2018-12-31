#include "presets_manager.hpp"
#include <glibmm.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include "util.hpp"

PresetsManager::PresetsManager()
    : presets_dir(Glib::get_user_config_dir() + "/PulseEffects"),
      settings(Gio::Settings::create("com.github.wwmm.pulseeffects")),
      sie_settings(
          Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs")),
      soe_settings(
          Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs")),
      limiter(std::make_unique<LimiterPreset>()),
      bass_enhancer(std::make_unique<BassEnhancerPreset>()),
      compressor(std::make_unique<CompressorPreset>()),
      crossfeed(std::make_unique<CrossfeedPreset>()),
      deesser(std::make_unique<DeesserPreset>()),
      equalizer(std::make_unique<EqualizerPreset>()),
      exciter(std::make_unique<ExciterPreset>()),
      filter(std::make_unique<FilterPreset>()),
      gate(std::make_unique<GatePreset>()),
      maximizer(std::make_unique<MaximizerPreset>()),
      pitch(std::make_unique<PitchPreset>()),
      reverb(std::make_unique<ReverbPreset>()),
      webrtc(std::make_unique<WebrtcPreset>()),
      multiband_compressor(std::make_unique<MultibandCompressorPreset>()),
      loudness(std::make_unique<LoudnessPreset>()),
      multiband_gate(std::make_unique<MultibandGatePreset>()),
      stereo_tools(std::make_unique<StereoToolsPreset>()),
      convolver(std::make_unique<ConvolverPreset>()),
      crystalizer(std::make_unique<CrystalizerPreset>()),
      autogain(std::make_unique<AutoGainPreset>()) {
  auto dir_exists = boost::filesystem::is_directory(presets_dir);

  if (!dir_exists) {
    if (boost::filesystem::create_directories(presets_dir)) {
      util::debug(log_tag +
                  "user presets directory created: " + presets_dir.string());
    } else {
      util::warning(log_tag + "failed to create user presets directory: " +
                    presets_dir.string());
    }

  } else {
    util::debug(log_tag + "user preset directory already exists: " +
                presets_dir.string());
  }
}

PresetsManager::~PresetsManager() {
  util::debug(log_tag + "destroyed");
}

std::vector<std::string> PresetsManager::get_names() {
  boost::filesystem::directory_iterator it{presets_dir};
  std::vector<std::string> names;

  while (it != boost::filesystem::directory_iterator{}) {
    if (boost::filesystem::is_regular_file(it->status())) {
      if (it->path().extension().string() == ".json") {
        names.push_back(it->path().stem().string());
      }
    }

    it++;
  }

  return names;
}

void PresetsManager::add(const std::string& name) {
  bool add_preset = true;

  for (auto p : get_names()) {
    if (p == name) {
      add_preset = false;
    }
  }

  if (add_preset) {
    save(name);
  }
}

void PresetsManager::save_general_settings(boost::property_tree::ptree& root) {
  boost::property_tree::ptree node_in;
  Glib::Variant<std::vector<double>> aux;
  std::vector<std::string> blacklist;

  // spectrum keys

  root.put("spectrum.show", settings->get_boolean("show-spectrum"));
  root.put("spectrum.n-points", settings->get_int("spectrum-n-points"));
  root.put("spectrum.height", settings->get_int("spectrum-height"));
  root.put("spectrum.use-custom-color",
           settings->get_boolean("use-custom-color"));
  root.put("spectrum.fill", settings->get_boolean("spectrum-fill"));
  root.put("spectrum.scale", settings->get_double("spectrum-scale"));
  root.put("spectrum.exponent", settings->get_double("spectrum-exponent"));
  root.put("spectrum.sampling-freq",
           settings->get_int("spectrum-sampling-freq"));

  settings->get_value("spectrum-color", aux);

  for (auto& p : aux.get()) {
    boost::property_tree::ptree node;
    node.put("", p);
    node_in.push_back(std::make_pair("", node));
  }

  root.add_child("spectrum.color", node_in);

  // input blacklist

  blacklist = settings->get_string_array("blacklist-in");

  node_in.clear();

  for (auto& p : blacklist) {
    boost::property_tree::ptree node;
    node.put("", p);
    node_in.push_back(std::make_pair("", node));
  }

  root.add_child("input.blacklist", node_in);

  // output blacklist

  blacklist = settings->get_string_array("blacklist-out");

  node_in.clear();

  for (auto& p : blacklist) {
    boost::property_tree::ptree node;
    node.put("", p);
    node_in.push_back(std::make_pair("", node));
  }

  root.add_child("output.blacklist", node_in);
}

void PresetsManager::load_general_settings(boost::property_tree::ptree& root) {
  std::vector<std::string> blacklist;

  // spectrum keys

  update_key<bool>(root, settings, "show-spectrum", "spectrum.show");

  update_key<int>(root, settings, "spectrum-n-points", "spectrum.n-points");

  update_key<int>(root, settings, "spectrum-height", "spectrum.height");

  update_key<bool>(root, settings, "use-custom-color",
                   "spectrum.use-custom-color");

  update_key<bool>(root, settings, "spectrum-fill", "spectrum.fill");

  update_key<double>(root, settings, "spectrum-scale", "spectrum.scale");

  update_key<double>(root, settings, "spectrum-exponent", "spectrum.exponent");

  update_key<int>(root, settings, "spectrum-sampling-freq",
                  "spectrum.sampling-freq");

  try {
    std::vector<double> spectrum_color;

    for (auto& p : root.get_child("spectrum.color")) {
      spectrum_color.push_back(p.second.get<double>(""));
    }

    auto v = Glib::Variant<std::vector<double>>::create(spectrum_color);

    settings->set_value("spectrum-color", v);
  } catch (const boost::property_tree::ptree_error& e) {
    settings->reset("spectrum-color");
  }

  // input blacklist

  try {
    for (auto& p : root.get_child("input.blacklist")) {
      blacklist.push_back(p.second.data());
    }

    settings->set_string_array("blacklist-in", blacklist);
  } catch (const boost::property_tree::ptree_error& e) {
    settings->reset("blacklist-in");
  }

  // output blacklist

  blacklist.clear();

  try {
    for (auto& p : root.get_child("output.blacklist")) {
      blacklist.push_back(p.second.data());
    }

    settings->set_string_array("blacklist-out", blacklist);
  } catch (const boost::property_tree::ptree_error& e) {
    settings->reset("blacklist-out");
  }
}

void PresetsManager::save(const std::string& name) {
  boost::property_tree::ptree root, node_in, node_out;

  save_general_settings(root);

  std::vector<std::string> input_plugins =
      soe_settings->get_string_array("plugins");

  std::vector<std::string> output_plugins =
      sie_settings->get_string_array("plugins");

  for (auto& p : input_plugins) {
    boost::property_tree::ptree node;
    node.put("", p);
    node_in.push_back(std::make_pair("", node));
  }

  for (auto& p : output_plugins) {
    boost::property_tree::ptree node;
    node.put("", p);
    node_out.push_back(std::make_pair("", node));
  }

  root.add_child("input.plugins_order", node_in);
  root.add_child("output.plugins_order", node_out);

  bass_enhancer->write(root);
  compressor->write(root);
  crossfeed->write(root);
  deesser->write(root);
  equalizer->write(root);
  exciter->write(root);
  filter->write(root);
  gate->write(root);
  limiter->write(root);
  maximizer->write(root);
  pitch->write(root);
  reverb->write(root);
  webrtc->write(root);
  multiband_compressor->write(root);
  loudness->write(root);
  multiband_gate->write(root);
  stereo_tools->write(root);
  convolver->write(root);
  crystalizer->write(root);
  autogain->write(root);

  auto output_file = presets_dir / boost::filesystem::path{name + ".json"};

  boost::property_tree::write_json(output_file.string(), root);

  util::debug(log_tag + "saved preset: " + output_file.string());
}

void PresetsManager::remove(const std::string& name) {
  auto preset_file = presets_dir / boost::filesystem::path{name + ".json"};

  if (boost::filesystem::exists(preset_file)) {
    boost::filesystem::remove(preset_file);

    util::debug(log_tag + "removed preset: " + preset_file.string());
  }
}

void PresetsManager::load(const std::string& name) {
  boost::property_tree::ptree root;
  std::vector<std::string> input_plugins, output_plugins;

  auto input_file = presets_dir / boost::filesystem::path{name + ".json"};

  boost::property_tree::read_json(input_file.string(), root);

  load_general_settings(root);

  try {
    Glib::Variant<std::vector<std::string>> aux;
    soe_settings->get_default_value("plugins", aux);

    for (auto& p : root.get_child("input.plugins_order")) {
      auto value = p.second.data();

      for (auto v : aux.get()) {
        if (v == value) {
          input_plugins.push_back(value);

          break;
        }
      }
    }

    for (auto v : aux.get()) {
      if (std::find(input_plugins.begin(), input_plugins.end(), v) ==
          input_plugins.end()) {
        input_plugins.push_back(v);
      }
    }
  } catch (const boost::property_tree::ptree_error& e) {
    Glib::Variant<std::vector<std::string>> aux;
    soe_settings->get_default_value("plugins", aux);
    input_plugins = aux.get();
  }

  try {
    Glib::Variant<std::vector<std::string>> aux;
    sie_settings->get_default_value("plugins", aux);

    for (auto& p : root.get_child("output.plugins_order")) {
      auto value = p.second.data();

      for (auto v : aux.get()) {
        if (v == value) {
          output_plugins.push_back(value);

          break;
        }
      }
    }

    for (auto v : aux.get()) {
      if (std::find(output_plugins.begin(), output_plugins.end(), v) ==
          output_plugins.end()) {
        output_plugins.push_back(v);
      }
    }
  } catch (const boost::property_tree::ptree_error& e) {
    Glib::Variant<std::vector<std::string>> aux;
    sie_settings->get_default_value("plugins", aux);
    output_plugins = aux.get();
  }

  soe_settings->set_string_array("plugins", input_plugins);
  sie_settings->set_string_array("plugins", output_plugins);

  bass_enhancer->read(root);
  compressor->read(root);
  crossfeed->read(root);
  deesser->read(root);
  equalizer->read(root);
  exciter->read(root);
  filter->read(root);
  gate->read(root);
  limiter->read(root);
  maximizer->read(root);
  pitch->read(root);
  reverb->read(root);
  webrtc->read(root);
  multiband_compressor->read(root);
  loudness->read(root);
  multiband_gate->read(root);
  stereo_tools->read(root);
  convolver->read(root);
  crystalizer->read(root);
  autogain->read(root);

  util::debug(log_tag + "loaded preset: " + input_file.string());
}

void PresetsManager::import(const std::string& file_path) {
  boost::filesystem::path p{file_path};

  if (boost::filesystem::is_regular_file(p)) {
    if (p.extension().string() == ".json") {
      auto out_path = presets_dir / p.filename();

      boost::filesystem::copy_file(
          p, out_path, boost::filesystem::copy_option::overwrite_if_exists);

      util::debug(log_tag + "imported preset to: " + out_path.string());
    }
  } else {
    util::warning(log_tag + p.string() + " is not a file!");
  }
}
