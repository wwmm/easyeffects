#include <glibmm.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include "presets_manager.hpp"
#include "util.hpp"

namespace fs = boost::filesystem;

PresetsManager::PresetsManager()
    : presets_dir(Glib::get_user_config_dir() + "/PulseEffects"),
      limiter(std::make_unique<LimiterPreset>()) {
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
    boost::property_tree::ptree root;

    limiter->write(root);

    auto out_file = presets_dir / fs::path{name + ".json"};

    boost::property_tree::write_json(out_file.string(), root);

    util::debug(log_tag + "saved preset: " + out_file.string());
}

void PresetsManager::remove(const std::string& name) {
    auto preset_file = presets_dir / fs::path{name + ".json"};

    if (fs::exists(preset_file)) {
        fs::remove(preset_file);

        util::debug(log_tag +
                    "preset file file removed: " + preset_file.string());
    }
}

void PresetsManager::load(const std::string& name) {
    util::debug("load: " + name);
}

void PresetsManager::import(const std::string& name) {
    util::debug("import: " + name);
}
