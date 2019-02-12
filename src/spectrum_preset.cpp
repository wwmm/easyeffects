#include "spectrum_preset.hpp"

SpectrumPreset::SpectrumPreset()
    : settings(Gio::Settings::create("com.github.wwmm.pulseeffects")) {}

void SpectrumPreset::save(boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
  boost::property_tree::ptree node_in;
  Glib::Variant<std::vector<double>> aux;

  root.put("spectrum.show", settings->get_boolean("show-spectrum"));

  root.put("spectrum.n-points", settings->get_int("spectrum-n-points"));

  root.put("spectrum.height", settings->get_int("spectrum-height"));

  root.put("spectrum.use-custom-color",
           settings->get_boolean("use-custom-color"));

  root.put("spectrum.fill", settings->get_boolean("spectrum-fill"));

  root.put("spectrum.border", settings->get_boolean("spectrum-border"));

  root.put("spectrum.scale", settings->get_double("spectrum-scale"));

  root.put("spectrum.exponent", settings->get_double("spectrum-exponent"));

  root.put("spectrum.sampling-freq",
           settings->get_int("spectrum-sampling-freq"));

  root.put("spectrum.line-width", settings->get_double("spectrum-line-width"));

  settings->get_value("spectrum-color", aux);

  for (auto& p : aux.get()) {
    boost::property_tree::ptree node;
    node.put("", p);
    node_in.push_back(std::make_pair("", node));
  }

  root.add_child("spectrum.color", node_in);
}

void SpectrumPreset::load(boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "show-spectrum", "spectrum.show");

  update_key<int>(root, settings, "spectrum-n-points", "spectrum.n-points");

  update_key<int>(root, settings, "spectrum-height", "spectrum.height");

  update_key<bool>(root, settings, "use-custom-color",
                   "spectrum.use-custom-color");

  update_key<bool>(root, settings, "spectrum-fill", "spectrum.fill");

  update_key<bool>(root, settings, "spectrum-border", "spectrum.border");

  update_key<double>(root, settings, "spectrum-scale", "spectrum.scale");

  update_key<double>(root, settings, "spectrum-exponent", "spectrum.exponent");

  update_key<int>(root, settings, "spectrum-sampling-freq",
                  "spectrum.sampling-freq");

  update_key<double>(root, settings, "spectrum-line-width",
                     "spectrum.line-width");

  try {
    std::vector<double> spectrum_color;

    for (auto& p : root.get_child("spectrum.color")) {
      spectrum_color.push_back(p.second.get<double>(""));
    }

    auto v = Glib::Variant<std::vector<double>>::create(spectrum_color);

    settings->set_value("spectrum-color", v);
  } catch (const boost::property_tree::ptree_error& e) {
    settings->reset("spectrum-color");
  }
}

void SpectrumPreset::write(PresetType preset_type,
                           boost::property_tree::ptree& root) {
  save(root, "", settings);
}

void SpectrumPreset::read(PresetType preset_type,
                          boost::property_tree::ptree& root) {
  load(root, "", settings);
}
