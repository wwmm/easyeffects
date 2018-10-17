#include "reverb_preset.hpp"

ReverbPreset::ReverbPreset()
    : input_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.reverb")),
      output_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sinkinputs.reverb")) {}

void ReverbPreset::save(boost::property_tree::ptree& root,
                        const std::string& section,
                        const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".reverb.state", settings->get_boolean("state"));

  root.put(section + ".reverb.input-gain", settings->get_double("input-gain"));

  root.put(section + ".reverb.output-gain",
           settings->get_double("output-gain"));

  root.put(section + ".reverb.room-size", settings->get_string("room-size"));

  root.put(section + ".reverb.decay-time", settings->get_double("decay-time"));

  root.put(section + ".reverb.hf-damp", settings->get_double("hf-damp"));

  root.put(section + ".reverb.diffusion", settings->get_double("diffusion"));

  root.put(section + ".reverb.amount", settings->get_double("amount"));

  root.put(section + ".reverb.dry", settings->get_double("dry"));

  root.put(section + ".reverb.predelay", settings->get_double("predelay"));

  root.put(section + ".reverb.bass-cut", settings->get_double("bass-cut"));

  root.put(section + ".reverb.treble-cut", settings->get_double("treble-cut"));
}

void ReverbPreset::load(boost::property_tree::ptree& root,
                        const std::string& section,
                        const Glib::RefPtr<Gio::Settings>& settings) {
  settings->set_boolean("state",
                        root.get<bool>(section + ".reverb.state",
                                       get_default<bool>(settings, "state")));

  settings->set_double(
      "input-gain",
      root.get<double>(section + ".reverb.input-gain",
                       get_default<double>(settings, "input-gain")));

  settings->set_double(
      "output-gain",
      root.get<double>(section + ".reverb.output-gain",
                       get_default<double>(settings, "output-gain")));

  settings->set_string(
      "room-size",
      root.get<std::string>(section + ".reverb.room-size",
                            get_default<std::string>(settings, "room-size")));

  settings->set_double(
      "decay-time",
      root.get<double>(section + ".reverb.decay-time",
                       get_default<double>(settings, "decay-time")));

  settings->set_double(
      "hf-damp", root.get<double>(section + ".reverb.hf-damp",
                                  get_default<double>(settings, "hf-damp")));

  settings->set_double(
      "diffusion",
      root.get<double>(section + ".reverb.diffusion",
                       get_default<double>(settings, "diffusion")));

  settings->set_double(
      "amount", root.get<double>(section + ".reverb.amount",
                                 get_default<double>(settings, "amount")));

  settings->set_double("dry",
                       root.get<double>(section + ".reverb.dry",
                                        get_default<double>(settings, "dry")));

  settings->set_double(
      "predelay", root.get<double>(section + ".reverb.predelay",
                                   get_default<double>(settings, "predelay")));

  settings->set_double(
      "bass-cut", root.get<double>(section + ".reverb.bass-cut",
                                   get_default<double>(settings, "bass-cut")));

  settings->set_double(
      "treble-cut",
      root.get<double>(section + ".reverb.treble-cut",
                       get_default<double>(settings, "treble-cut")));
}

void ReverbPreset::write(boost::property_tree::ptree& root) {
  save(root, "input", input_settings);
  save(root, "output", output_settings);
}

void ReverbPreset::read(boost::property_tree::ptree& root) {
  load(root, "input", input_settings);
  load(root, "output", output_settings);
}
