#include "limiter_preset.hpp"

LimiterPreset::LimiterPreset()
    : input_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs.limiter")),
      output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs.limiter")) {}

void LimiterPreset::save(boost::property_tree::ptree& root,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".limiter.state", settings->get_boolean("state"));

  root.put(section + ".limiter.input-gain", settings->get_double("input-gain"));

  root.put(section + ".limiter.limit", settings->get_double("limit"));

  root.put(section + ".limiter.lookahead", settings->get_double("lookahead"));

  root.put(section + ".limiter.release", settings->get_double("release"));

  root.put(section + ".limiter.asc", settings->get_boolean("asc"));

  root.put(section + ".limiter.asc-level", settings->get_double("asc-level"));

  root.put(section + ".limiter.oversampling", settings->get_int("oversampling"));
}

void LimiterPreset::load(boost::property_tree::ptree& root,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".limiter.state");

  update_key<double>(root, settings, "input-gain", section + ".limiter.input-gain");

  update_key<double>(root, settings, "limit", section + ".limiter.limit");

  update_key<double>(root, settings, "lookahead", section + ".limiter.lookahead");

  update_key<double>(root, settings, "release", section + ".limiter.release");

  update_key<bool>(root, settings, "asc", section + ".limiter.asc");

  update_key<double>(root, settings, "asc-level", section + ".limiter.asc-level");

  update_key<int>(root, settings, "oversampling", section + ".limiter.oversampling");
}

void LimiterPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  } else {
    save(root, "input", input_settings);
  }
}

void LimiterPreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  } else {
    load(root, "input", input_settings);
  }
}
