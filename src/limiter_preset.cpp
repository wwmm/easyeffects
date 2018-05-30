#include "limiter_preset.hpp"

LimiterPreset::LimiterPreset()
    : input_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.limiter")),
      output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.limiter")) {}

void LimiterPreset::save(boost::property_tree::ptree& root,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
    root.put(section + ".limiter.state", input_settings->get_boolean("state"));
    root.put(section + ".limiter.input_gain",
             input_settings->get_double("input-gain"));
    root.put(section + ".limiter.limit", input_settings->get_double("limit"));
    root.put(section + ".limiter.lookahead",
             input_settings->get_double("lookahead"));
}

void LimiterPreset::load(boost::property_tree::ptree& root,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {}

void LimiterPreset::write(boost::property_tree::ptree& root) {
    save(root, "input", input_settings);
    save(root, "output", output_settings);
}

void LimiterPreset::read(boost::property_tree::ptree& root) {
    load(root, "input", input_settings);
    load(root, "output", output_settings);
}
