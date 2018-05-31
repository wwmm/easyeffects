#include "stereo_enhancer_preset.hpp"

StereoEnhancerPreset::StereoEnhancerPreset()
    : output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.stereoenhancer")) {}

void StereoEnhancerPreset::save(boost::property_tree::ptree& root,
                                const std::string& section,
                                const Glib::RefPtr<Gio::Settings>& settings) {
    root.put(section + ".stereo_enhancer.state",
             settings->get_boolean("state"));

    root.put(section + ".stereo_enhancer.input-gain",
             settings->get_double("input-gain"));

    root.put(section + ".stereo_enhancer.output-gain",
             settings->get_double("output-gain"));

    root.put(section + ".stereo_enhancer.middle-source",
             settings->get_string("middle-source"));

    root.put(section + ".stereo_enhancer.side-gain",
             settings->get_double("side-gain"));

    root.put(section + ".stereo_enhancer.middle-phase",
             settings->get_boolean("middle-phase"));

    root.put(section + ".stereo_enhancer.left-phase",
             settings->get_boolean("left-phase"));

    root.put(section + ".stereo_enhancer.left-balance",
             settings->get_double("left-balance"));

    root.put(section + ".stereo_enhancer.left-delay",
             settings->get_double("left-delay"));

    root.put(section + ".stereo_enhancer.left-gain",
             settings->get_double("left-gain"));

    root.put(section + ".stereo_enhancer.right-phase",
             settings->get_boolean("right-phase"));

    root.put(section + ".stereo_enhancer.right-balance",
             settings->get_double("right-balance"));

    root.put(section + ".stereo_enhancer.right-delay",
             settings->get_double("right-delay"));

    root.put(section + ".stereo_enhancer.right-gain",
             settings->get_double("right-gain"));
}

void StereoEnhancerPreset::load(boost::property_tree::ptree& root,
                                const std::string& section,
                                const Glib::RefPtr<Gio::Settings>& settings) {
    settings->set_boolean("state",
                          root.get<bool>(section + ".stereo_enhancer.state",
                                         get_default<bool>(settings, "state")));

    settings->set_double(
        "input-gain",
        root.get<double>(section + ".stereo_enhancer.input-gain",
                         get_default<double>(settings, "input-gain")));

    settings->set_double(
        "output-gain",
        root.get<double>(section + ".stereo_enhancer.output-gain",
                         get_default<double>(settings, "output-gain")));

    settings->set_string(
        "middle-source",
        root.get<std::string>(
            section + ".stereo_enhancer.middle-source",
            get_default<std::string>(settings, "middle-source")));

    settings->set_double(
        "side-gain",
        root.get<double>(section + ".stereo_enhancer.side-gain",
                         get_default<double>(settings, "side-gain")));

    settings->set_boolean(
        "middle-phase",
        root.get<bool>(section + ".stereo_enhancer.middle-phase",
                       get_default<bool>(settings, "middle-phase")));

    settings->set_boolean(
        "left-phase",
        root.get<bool>(section + ".stereo_enhancer.left-phase",
                       get_default<bool>(settings, "left-phase")));

    settings->set_double(
        "left-balance",
        root.get<double>(section + ".stereo_enhancer.left-balance",
                         get_default<double>(settings, "left-balance")));

    settings->set_double(
        "left-delay",
        root.get<double>(section + ".stereo_enhancer.left-delay",
                         get_default<double>(settings, "left-delay")));

    settings->set_double(
        "left-gain",
        root.get<double>(section + ".stereo_enhancer.left-gain",
                         get_default<double>(settings, "left-gain")));

    settings->set_boolean(
        "right-phase",
        root.get<bool>(section + ".stereo_enhancer.right-phase",
                       get_default<bool>(settings, "right-phase")));

    settings->set_double(
        "right-balance",
        root.get<double>(section + ".stereo_enhancer.right-balance",
                         get_default<double>(settings, "right-balance")));

    settings->set_double(
        "right-delay",
        root.get<double>(section + ".stereo_enhancer.right-delay",
                         get_default<double>(settings, "right-delay")));

    settings->set_double(
        "right-gain",
        root.get<double>(section + ".stereo_enhancer.right-gain",
                         get_default<double>(settings, "right-gain")));
}

void StereoEnhancerPreset::write(boost::property_tree::ptree& root) {
    save(root, "output", output_settings);
}

void StereoEnhancerPreset::read(boost::property_tree::ptree& root) {
    load(root, "output", output_settings);
}
