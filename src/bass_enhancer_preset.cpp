#include "bass_enhancer_preset.hpp"

BassEnhancerPreset::BassEnhancerPreset()
    : output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.bassenhancer")) {}

void BassEnhancerPreset::save(boost::property_tree::ptree& root,
                              const std::string& section,
                              const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".bass_enhancer.state", settings->get_boolean("state"));

  root.put(section + ".bass_enhancer.input-gain",
           settings->get_double("input-gain"));

  root.put(section + ".bass_enhancer.output-gain",
           settings->get_double("output-gain"));

  root.put(section + ".bass_enhancer.amount", settings->get_double("amount"));

  root.put(section + ".bass_enhancer.harmonics",
           settings->get_double("harmonics"));

  root.put(section + ".bass_enhancer.scope", settings->get_double("scope"));

  root.put(section + ".bass_enhancer.floor", settings->get_double("floor"));

  root.put(section + ".bass_enhancer.blend", settings->get_double("blend"));

  root.put(section + ".bass_enhancer.floor-active",
           settings->get_boolean("floor-active"));

  root.put(section + ".bass_enhancer.listen", settings->get_boolean("listen"));
}

void BassEnhancerPreset::load(boost::property_tree::ptree& root,
                              const std::string& section,
                              const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".bass_enhancer.state");

  update_key<double>(root, settings, "input-gain",
                     section + ".bass_enhancer.input-gain");

  update_key<double>(root, settings, "output-gain",
                     section + ".bass_enhancer.output-gain");

  update_key<double>(root, settings, "amount",
                     section + ".bass_enhancer.amount");

  update_key<double>(root, settings, "harmonics",
                     section + ".bass_enhancer.harmonics");

  update_key<double>(root, settings, "scope", section + ".bass_enhancer.scope");

  update_key<double>(root, settings, "floor", section + ".bass_enhancer.floor");

  update_key<double>(root, settings, "blend", section + ".bass_enhancer.blend");

  update_key<bool>(root, settings, "floor-active",
                   section + ".bass_enhancer.floor-active");

  update_key<bool>(root, settings, "listen", section + ".bass_enhancer.listen");

  // settings->set_boolean("state",
  //                       root.get<bool>(section + ".bass_enhancer.state",
  //                                      get_default<bool>(settings,
  //                                      "state")));

  // settings->set_double(
  //     "input-gain",
  //     root.get<double>(section + ".bass_enhancer.input-gain",
  //                      get_default<double>(settings, "input-gain")));
  //
  // settings->set_double(
  //     "output-gain",
  //     root.get<double>(section + ".bass_enhancer.output-gain",
  //                      get_default<double>(settings, "output-gain")));

  // settings->set_double(
  //     "amount", root.get<double>(section + ".bass_enhancer.amount",
  //                                get_default<double>(settings, "amount")));

  // settings->set_double(
  //     "harmonics",
  //     root.get<double>(section + ".bass_enhancer.harmonics",
  //                      get_default<double>(settings, "harmonics")));

  // settings->set_double(
  //     "scope", root.get<double>(section + ".bass_enhancer.scope",
  //                               get_default<double>(settings, "scope")));

  // settings->set_double(
  //     "floor", root.get<double>(section + ".bass_enhancer.floor",
  //                               get_default<double>(settings, "floor")));

  // settings->set_double(
  //     "blend", root.get<double>(section + ".bass_enhancer.blend",
  //                               get_default<double>(settings, "blend")));

  // settings->set_boolean(
  //     "floor-active",
  //     root.get<bool>(section + ".bass_enhancer.floor-active",
  //                    get_default<bool>(settings, "floor-active")));
  //
  // settings->set_boolean("listen",
  //                       root.get<bool>(section + ".bass_enhancer.listen",
  //                                      get_default<bool>(settings,
  //                                      "listen")));
}

void BassEnhancerPreset::write(boost::property_tree::ptree& root) {
  save(root, "output", output_settings);
}

void BassEnhancerPreset::read(boost::property_tree::ptree& root) {
  load(root, "output", output_settings);
}
