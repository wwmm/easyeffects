#include "maximizer_preset.hpp"

MaximizerPreset::MaximizerPreset()
    : output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.maximizer")) {}

void MaximizerPreset::save(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".maximizer.state", settings->get_boolean("state"));

  root.put(section + ".maximizer.release", settings->get_double("release"));

  root.put(section + ".maximizer.ceiling", settings->get_double("ceiling"));

  root.put(section + ".maximizer.threshold", settings->get_double("threshold"));
}

void MaximizerPreset::load(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  settings->set_boolean("state",
                        root.get<bool>(section + ".maximizer.state",
                                       get_default<bool>(settings, "state")));

  settings->set_double(
      "release", root.get<double>(section + ".maximizer.release",
                                  get_default<double>(settings, "release")));

  settings->set_double(
      "ceiling", root.get<double>(section + ".maximizer.ceiling",
                                  get_default<double>(settings, "ceiling")));

  settings->set_double(
      "threshold",
      root.get<double>(section + ".maximizer.threshold",
                       get_default<double>(settings, "threshold")));
}

void MaximizerPreset::write(boost::property_tree::ptree& root) {
  save(root, "output", output_settings);
}

void MaximizerPreset::read(boost::property_tree::ptree& root) {
  load(root, "output", output_settings);
}
