#include "limiter_preset.hpp"

LimiterPreset::LimiterPreset(const boost::property_tree::ptree& treeroot)
    : root(treeroot),
      input_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.limiter")),
      output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.limiter")) {}

void LimiterPreset::save_input_settings() {}

void LimiterPreset::save_output_settings() {}

void LimiterPreset::load_input_settings() {}

void LimiterPreset::load_output_settings() {}

void LimiterPreset::save() {}

void LimiterPreset::load() {}
