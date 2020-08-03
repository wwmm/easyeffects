#include "pitch_ui.hpp"

PitchUi::PitchUi(BaseObjectType* cobject,
                 const Glib::RefPtr<Gtk::Builder>& builder,
                 const std::string& schema,
                 const std::string& schema_path)
    : Gtk::Grid(cobject), PluginUiBase(builder, schema, schema_path) {
  name = "pitch";

  // loading glade widgets

  builder->get_widget("faster", faster);
  builder->get_widget("formant_preserving", formant_preserving);
  builder->get_widget("plugin_reset", reset_button);

  get_object(builder, "cents", cents);
  get_object(builder, "crispness", crispness);
  get_object(builder, "semitones", semitones);
  get_object(builder, "octaves", octaves);
  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("cents", cents.get(), "value", flag);
  settings->bind("crispness", crispness.get(), "value", flag);
  settings->bind("semitones", semitones.get(), "value", flag);
  settings->bind("octaves", octaves.get(), "value", flag);
  settings->bind("faster", faster, "active", flag);
  settings->bind("formant-preserving", formant_preserving, "active", flag);
  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

PitchUi::~PitchUi() {
  util::debug(name + " ui destroyed");
}

void PitchUi::reset() {
  try {
    std::string section = (preset_type == PresetType::output) ? "output" : "input";

    update_default_key<double>(settings, "input-gain", section + ".pitch.input-gain");

    update_default_key<double>(settings, "output-gain", section + ".pitch.output-gain");

    update_default_key<double>(settings, "cents", section + ".pitch.cents");

    update_default_key<int>(settings, "semitones", section + ".pitch.semitones");

    update_default_key<int>(settings, "octaves", section + ".pitch.octaves");

    update_default_key<int>(settings, "crispness", section + ".pitch.crispness");

    update_default_key<bool>(settings, "formant-preserving", section + ".pitch.formant-preserving");

    update_default_key<bool>(settings, "faster", section + ".pitch.faster");

    util::debug(name + " plugin: successfully reset");
  } catch (std::exception& e) {
    util::debug(name + " plugin: an error occurred during reset process");
  }
}
