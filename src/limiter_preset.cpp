#include "limiter_preset.hpp"

LimiterPreset::LimiterPreset()
    : input_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.limiter")),
      output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.limiter")) {}

void LimiterPreset::save(boost::property_tree::ptree& root,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
    root.put(section + ".limiter.state", settings->get_boolean("state"));

    root.put(section + ".limiter.input_gain",
             settings->get_double("input-gain"));

    root.put(section + ".limiter.limit", settings->get_double("limit"));

    root.put(section + ".limiter.lookahead", settings->get_double("lookahead"));

    root.put(section + ".limiter.release", settings->get_double("release"));

    root.put(section + ".limiter.asc", settings->get_boolean("asc"));

    root.put(section + ".limiter.asc_level", settings->get_double("asc-level"));

    root.put(section + ".limiter.oversampling",
             settings->get_int("oversampling"));

    // autovolume

    root.put(section + ".limiter.autovolume.state",
             settings->get_boolean("autovolume-state"));

    root.put(section + ".limiter.autovolume.window",
             settings->get_double("autovolume-window"));

    root.put(section + ".limiter.autovolume.target",
             settings->get_int("autovolume-target"));

    root.put(section + ".limiter.autovolume.tolerance",
             settings->get_int("autovolume-tolerance"));

    root.put(section + ".limiter.autovolume.threshold",
             settings->get_int("autovolume-threshold"));
}

void LimiterPreset::load(boost::property_tree::ptree& root,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
    settings->set_boolean("state", root.get<bool>(section + ".limiter.state"));
}

void LimiterPreset::write(boost::property_tree::ptree& root) {
    save(root, "input", input_settings);
    save(root, "output", output_settings);
}

void LimiterPreset::read(boost::property_tree::ptree& root) {
    load(root, "input", input_settings);
    load(root, "output", output_settings);
}
