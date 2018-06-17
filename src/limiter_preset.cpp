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

    root.put(section + ".limiter.input-gain",
             settings->get_double("input-gain"));

    root.put(section + ".limiter.limit", settings->get_double("limit"));

    root.put(section + ".limiter.lookahead", settings->get_double("lookahead"));

    root.put(section + ".limiter.release", settings->get_double("release"));

    root.put(section + ".limiter.asc", settings->get_boolean("asc"));

    root.put(section + ".limiter.asc-level", settings->get_double("asc-level"));

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
    settings->set_boolean("state",
                          root.get<bool>(section + ".limiter.state",
                                         get_default<bool>(settings, "state")));

    settings->set_double(
        "input-gain",
        root.get<double>(section + ".limiter.input-gain",
                         get_default<double>(settings, "input-gain")));

    settings->set_double(
        "limit", root.get<double>(section + ".limiter.limit",
                                  get_default<double>(settings, "limit")));

    settings->set_double(
        "lookahead",
        root.get<double>(section + ".limiter.lookahead",
                         get_default<double>(settings, "lookahead")));

    settings->set_double(
        "release", root.get<double>(section + ".limiter.release",
                                    get_default<double>(settings, "release")));

    settings->set_boolean("asc",
                          root.get<bool>(section + ".limiter.asc",
                                         get_default<bool>(settings, "asc")));

    settings->set_double(
        "asc-level",
        root.get<double>(section + ".limiter.asc-level",
                         get_default<double>(settings, "asc-level")));

    settings->set_int(
        "oversampling",
        root.get<int>(section + ".limiter.oversampling",
                      get_default<int>(settings, "oversampling")));

    // autovolume

    bool enable_autovolume =
        root.get<bool>(section + ".limiter.autovolume.state",
                       get_default<bool>(settings, "autovolume-state"));

    double autovolume_window =
        root.get<double>(section + ".limiter.autovolume.window",
                         get_default<double>(settings, "autovolume-window"));

    int autovolume_target =
        root.get<int>(section + ".limiter.autovolume.target",
                      get_default<int>(settings, "autovolume-target"));

    int autovolume_tolerance =
        root.get<int>(section + ".limiter.autovolume.tolerance",
                      get_default<int>(settings, "autovolume-tolerance"));

    int autovolume_threshold =
        root.get<int>(section + ".limiter.autovolume.threshold",
                      get_default<int>(settings, "autovolume-threshold"));

    settings->set_boolean("autovolume-state", enable_autovolume);
    settings->set_double("autovolume-window", autovolume_window);
    settings->set_int("autovolume-target", autovolume_target);
    settings->set_int("autovolume-tolerance", autovolume_tolerance);
    settings->set_int("autovolume-threshold", autovolume_threshold);

    if (enable_autovolume) {
        settings->set_double("release", autovolume_window);
        settings->set_double("limit", autovolume_target + autovolume_tolerance);
    }
}

void LimiterPreset::write(boost::property_tree::ptree& root) {
    save(root, "input", input_settings);
    save(root, "output", output_settings);
}

void LimiterPreset::read(boost::property_tree::ptree& root) {
    load(root, "input", input_settings);
    load(root, "output", output_settings);
}
