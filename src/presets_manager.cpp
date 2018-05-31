#include <glibmm.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include "presets_manager.hpp"
#include "util.hpp"

namespace fs = boost::filesystem;

PresetsManager::PresetsManager()
    : presets_dir(Glib::get_user_config_dir() + "/PulseEffects"),
      sie_settings(
          Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs")),
      soe_settings(
          Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs")),
      limiter(std::make_unique<LimiterPreset>()),
      bass_enhancer(std::make_unique<BassEnhancerPreset>()),
      compressor(std::make_unique<CompressorPreset>()),
      crossfeed(std::make_unique<CrossfeedPreset>()),
      deesser(std::make_unique<DeesserPreset>()),
      delay(std::make_unique<DelayPreset>()),
      equalizer(std::make_unique<EqualizerPreset>()),
      exciter(std::make_unique<ExciterPreset>()),
      filter(std::make_unique<FilterPreset>()),
      gate(std::make_unique<GatePreset>()),
      maximizer(std::make_unique<MaximizerPreset>()),
      panorama(std::make_unique<PanoramaPreset>()),
      pitch(std::make_unique<PitchPreset>()),
      reverb(std::make_unique<ReverbPreset>()),
      stereo_enhancer(std::make_unique<StereoEnhancerPreset>()),
      webrtc(std::make_unique<WebrtcPreset>()) {
    auto dir_exists = fs::is_directory(presets_dir);

    if (!dir_exists) {
        if (fs::create_directories(presets_dir)) {
            util::debug(log_tag + "user presets directory created: " +
                        presets_dir.string());
        } else {
            util::warning(log_tag +
                          "failed to create user presets directory: " +
                          presets_dir.string());
        }

    } else {
        util::debug(log_tag + "user preset directory already exists: " +
                    presets_dir.string());
    }
}

PresetsManager::~PresetsManager() {}

std::vector<std::string> PresetsManager::get_names() {
    fs::directory_iterator it{presets_dir};
    std::vector<std::string> names;

    while (it != fs::directory_iterator{}) {
        if (fs::is_regular_file(it->status())) {
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

void PresetsManager::save(const std::string& name) {
    boost::property_tree::ptree root, node_in, node_out;

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
    delay->write(root);
    equalizer->write(root);
    exciter->write(root);
    filter->write(root);
    gate->write(root);
    limiter->write(root);
    maximizer->write(root);
    panorama->write(root);
    pitch->write(root);
    reverb->write(root);
    stereo_enhancer->write(root);
    webrtc->write(root);

    auto output_file = presets_dir / fs::path{name + ".json"};

    boost::property_tree::write_json(output_file.string(), root);

    util::debug(log_tag + "saved preset: " + output_file.string());
}

void PresetsManager::remove(const std::string& name) {
    auto preset_file = presets_dir / fs::path{name + ".json"};

    if (fs::exists(preset_file)) {
        fs::remove(preset_file);

        util::debug(log_tag + "removed preset: " + preset_file.string());
    }
}

void PresetsManager::load(const std::string& name) {
    boost::property_tree::ptree root;
    std::vector<std::string> input_plugins, output_plugins;

    auto input_file = presets_dir / fs::path{name + ".json"};

    boost::property_tree::read_json(input_file.string(), root);

    try {
        for (auto& p : root.get_child("input.plugins_order")) {
            input_plugins.push_back(p.second.data());
        }
    } catch (const boost::property_tree::ptree_error& e) {
        Glib::Variant<std::vector<std::string>> aux;
        soe_settings->get_default_value("plugins", aux);
        input_plugins = aux.get();
    }

    try {
        for (auto& p : root.get_child("output.plugins_order")) {
            output_plugins.push_back(p.second.data());
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
    delay->read(root);
    equalizer->read(root);
    exciter->read(root);
    filter->read(root);
    gate->read(root);
    limiter->read(root);
    maximizer->read(root);
    panorama->read(root);
    pitch->read(root);
    reverb->read(root);
    stereo_enhancer->read(root);
    webrtc->read(root);

    util::debug(log_tag + "loaded preset: " + input_file.string());
}

void PresetsManager::import(const std::string& file_path) {
    fs::path p{file_path};

    if (fs::is_regular_file(p)) {
        if (p.extension().string() == ".json") {
            auto out_path = presets_dir / p.filename();

            fs::copy_file(p, out_path, fs::copy_option::overwrite_if_exists);

            util::debug(log_tag + "imported preset to: " + out_path.string());
        }
    } else {
        util::warning(log_tag + p.string() + " is not a file!");
    }
}
