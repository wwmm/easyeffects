#include "spectrum_preset.hpp"

SpectrumPreset::SpectrumPreset() : settings(Gio::Settings::create("com.github.wwmm.pulseeffects.spectrum")) {}

void SpectrumPreset::save(boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
  boost::property_tree::ptree node_in;
  Glib::Variant<std::vector<double>> aux;

  root.put("spectrum.show", settings->get_boolean("show"));

  root.put("spectrum.n-points", settings->get_int("n-points"));

  root.put("spectrum.height", settings->get_int("height"));

  root.put("spectrum.use-custom-color", settings->get_boolean("use-custom-color"));

  root.put("spectrum.fill", settings->get_boolean("fill"));

  root.put("spectrum.show-bar-border", settings->get_boolean("show-bar-border"));

  root.put("spectrum.scale", settings->get_double("scale"));

  root.put("spectrum.exponent", settings->get_double("exponent"));

  root.put("spectrum.sampling-freq", settings->get_int("sampling-freq"));

  root.put("spectrum.line-width", settings->get_double("line-width"));

  root.put("spectrum.type", settings->get_string("type"));

  settings->get_value("color", aux);

  for (auto& p : aux.get()) {
    boost::property_tree::ptree node;
    node.put("", p);
    node_in.push_back(std::make_pair("", node));
  }

  root.add_child("spectrum.color", node_in);

  // background color

  node_in.clear();

  settings->get_value("gradient-color", aux);

  for (auto& p : aux.get()) {
    boost::property_tree::ptree node;
    node.put("", p);
    node_in.push_back(std::make_pair("", node));
  }

  root.add_child("spectrum.gradient-color", node_in);
}

void SpectrumPreset::load(boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "show", "spectrum.show");

  update_key<int>(root, settings, "n-points", "spectrum.n-points");

  update_key<int>(root, settings, "height", "spectrum.height");

  update_key<bool>(root, settings, "use-custom-color", "spectrum.use-custom-color");

  update_key<bool>(root, settings, "fill", "spectrum.fill");

  update_key<bool>(root, settings, "show-bar-border", "spectrum.show-bar-border");

  update_key<double>(root, settings, "scale", "spectrum.scale");

  update_key<double>(root, settings, "exponent", "spectrum.exponent");

  update_key<int>(root, settings, "sampling-freq", "spectrum.sampling-freq");

  update_key<double>(root, settings, "line-width", "spectrum.line-width");

  update_string_key(root, settings, "type", "spectrum.type");

  try {
    std::vector<double> color;

    for (auto& p : root.get_child("spectrum.color")) {
      color.push_back(p.second.get<double>(""));
    }

    auto v = Glib::Variant<std::vector<double>>::create(color);

    settings->set_value("color", v);
  } catch (const boost::property_tree::ptree_error& e) {
    settings->reset("color");
  }

  // background color

  try {
    std::vector<double> color;

    for (auto& p : root.get_child("spectrum.gradient-color")) {
      color.push_back(p.second.get<double>(""));
    }

    auto v = Glib::Variant<std::vector<double>>::create(color);

    settings->set_value("gradient-color", v);
  } catch (const boost::property_tree::ptree_error& e) {
    settings->reset("gradient-color");
  }
}

void SpectrumPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  save(root, "", settings);
}

void SpectrumPreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  load(root, "", settings);
}
