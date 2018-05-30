#include "plugin_preset_base.hpp"

PluginPresetBase::PluginPresetBase(const std::string& InputSettings,
                                   const std::string& OutputSettings)
    : input_settings(Gio::Settings::create(InputSettings)),
      output_settings(Gio::Settings::create(OutputSettings)) {}

PluginPresetBase::~PluginPresetBase() {}

void PluginPresetBase::write(boost::property_tree::ptree& root) {
    save(root, "input", input_settings);
    save(root, "output", output_settings);
}

void PluginPresetBase::read(boost::property_tree::ptree& root) {
    load(root, "input", input_settings);
    load(root, "output", output_settings);
}
