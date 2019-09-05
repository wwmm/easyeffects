#include "autogain_ui.hpp"

AutoGainUi::AutoGainUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "autogain";

  // loading glade widgets

  builder->get_widget("m_level", m_level);
  builder->get_widget("s_level", s_level);
  builder->get_widget("i_level", i_level);
  builder->get_widget("r_level", r_level);
  builder->get_widget("g_level", g_level);
  builder->get_widget("l_level", l_level);
  builder->get_widget("lra_level", lra_level);

  builder->get_widget("m_label", m_label);
  builder->get_widget("s_label", s_label);
  builder->get_widget("i_label", i_label);
  builder->get_widget("r_label", r_label);
  builder->get_widget("g_label", g_label);
  builder->get_widget("l_label", l_label);
  builder->get_widget("lra_label", lra_label);

  builder->get_widget("reset", reset);
  builder->get_widget("detect_silence", detect_silence);

  get_object(builder, "input_gain", input_gain);
  builder->get_widget("l_label", l_label);
  get_object(builder, "output_gain", output_gain);
  get_object(builder, "target", target);
  get_object(builder, "weight_m", weight_m);
  get_object(builder, "weight_s", weight_s);
  get_object(builder, "weight_i", weight_i);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);
  settings->bind("target", target.get(), "value", flag);
  settings->bind("weight-m", weight_m.get(), "value", flag);
  settings->bind("weight-s", weight_s.get(), "value", flag);
  settings->bind("weight-i", weight_i.get(), "value", flag);
  settings->bind("detect-silence", detect_silence, "active", flag);

  reset->signal_clicked().connect([=]() { settings->set_boolean("reset", true); });
}

AutoGainUi::~AutoGainUi() {
  util::debug(name + " ui destroyed");
}

void AutoGainUi::on_new_momentary(const float& value) {
  m_level->set_value(util::db_to_linear(value));
  m_label->set_text(level_to_str(value, 0));
}

void AutoGainUi::on_new_shortterm(const float& value) {
  s_level->set_value(util::db_to_linear(value));
  s_label->set_text(level_to_str(value, 0));
}

void AutoGainUi::on_new_integrated(const float& value) {
  i_level->set_value(util::db_to_linear(value));
  i_label->set_text(level_to_str(value, 0));
}

void AutoGainUi::on_new_relative(const float& value) {
  r_level->set_value(util::db_to_linear(value));
  r_label->set_text(level_to_str(value, 0));
}

void AutoGainUi::on_new_loudness(const float& value) {
  l_level->set_value(util::db_to_linear(value));
  l_label->set_text(level_to_str(value, 0));
}

void AutoGainUi::on_new_range(const float& value) {
  lra_level->set_value(util::db_to_linear(value));
  lra_label->set_text(level_to_str(value, 0));
}

void AutoGainUi::on_new_gain(const float& value) {
  g_level->set_value(value);
  g_label->set_text(level_to_str(util::linear_to_db(value), 0));
}
