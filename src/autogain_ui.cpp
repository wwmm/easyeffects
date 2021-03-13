/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "autogain_ui.hpp"

AutoGainUi::AutoGainUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       const std::string& schema,
                       const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = "autogain";

  // loading builder widgets

  m_level = builder->get_widget<Gtk::LevelBar>("m_level");
  s_level = builder->get_widget<Gtk::LevelBar>("s_level");
  i_level = builder->get_widget<Gtk::LevelBar>("i_level");
  r_level = builder->get_widget<Gtk::LevelBar>("r_level");
  g_level = builder->get_widget<Gtk::LevelBar>("g_level");
  l_level = builder->get_widget<Gtk::LevelBar>("l_level");
  lra_level = builder->get_widget<Gtk::LevelBar>("lra_level");

  m_label = builder->get_widget<Gtk::Label>("m_label");
  s_label = builder->get_widget<Gtk::Label>("s_label");
  i_label = builder->get_widget<Gtk::Label>("i_label");
  r_label = builder->get_widget<Gtk::Label>("r_label");
  g_label = builder->get_widget<Gtk::Label>("g_label");
  l_label = builder->get_widget<Gtk::Label>("l_label");
  lra_label = builder->get_widget<Gtk::Label>("lra_label");

  reset_history = builder->get_widget<Gtk::Button>("reset");
  // builder->get_widget("detect_silence", detect_silence);
  // builder->get_widget("use_geometric_mean", use_geometric_mean);
  // builder->get_widget("weight_m_grid", weight_m_grid);
  // builder->get_widget("weight_s_grid", weight_s_grid);
  // builder->get_widget("weight_i_grid", weight_i_grid);

  reset_button = builder->get_widget<Gtk::Button>("reset_button");

  input_gain = builder->get_object<Gtk::Adjustment>("input_gain");
  output_gain = builder->get_object<Gtk::Adjustment>("output_gain");
  target = builder->get_object<Gtk::Adjustment>("target");

  // get_object(builder, "weight_m", weight_m);
  // get_object(builder, "weight_s", weight_s);
  // get_object(builder, "weight_i", weight_i);

  // gsettings bindings

  settings->bind("installed", this, "sensitive");
  settings->bind("input-gain", input_gain.get(), "value");
  settings->bind("output-gain", output_gain.get(), "value");
  settings->bind("target", target.get(), "value");
  // settings->bind("weight-m", weight_m.get(), "value", flag);
  // settings->bind("weight-s", weight_s.get(), "value", flag);
  // settings->bind("weight-i", weight_i.get(), "value", flag);
  // settings->bind("detect-silence", detect_silence, "active", flag);
  // settings->bind("use-geometric-mean", use_geometric_mean, "active", flag);
  // settings->bind("use-geometric-mean", weight_m_grid, "sensitive",
  //                Gio::SettingsBindFlags::SETTINGS_BIND_GET | Gio::SettingsBindFlags::SETTINGS_BIND_INVERT_BOOLEAN);
  // settings->bind("use-geometric-mean", weight_s_grid, "sensitive",
  //                Gio::SettingsBindFlags::SETTINGS_BIND_GET | Gio::SettingsBindFlags::SETTINGS_BIND_INVERT_BOOLEAN);
  // settings->bind("use-geometric-mean", weight_i_grid, "sensitive",
  //                Gio::SettingsBindFlags::SETTINGS_BIND_GET | Gio::SettingsBindFlags::SETTINGS_BIND_INVERT_BOOLEAN);

  reset_history->signal_clicked().connect([=, this]() { settings->set_boolean("reset", true); });

  // reset plugin
  reset_button->signal_clicked().connect([=, this]() { reset(); });
}

AutoGainUi::~AutoGainUi() {
  util::debug(name + " ui destroyed");
}

auto AutoGainUi::add_to_stack(Gtk::Stack* stack) -> AutoGainUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/autogain.ui");

  auto* ui = Gtk::Builder::get_widget_derived<AutoGainUi>(builder, "top_box", "com.github.wwmm.pulseeffects.autogain",
                                                          "/com/github/wwmm/pulseeffects/sinkinputs/autogain/");

  auto stack_page = stack->add(*ui, "autogain");

  return ui;
}

void AutoGainUi::reset() {
  settings->reset("detect-silence");

  settings->reset("use-geometric-mean");

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("target");

  settings->reset("weight-m");

  settings->reset("weight-s");

  settings->reset("weight-i");
}

void AutoGainUi::on_new_momentary(const float& value) {
  m_level->set_value(util::db_to_linear(value));
  m_label->set_text(level_to_localized_string(value, 0));
}

void AutoGainUi::on_new_shortterm(const float& value) {
  s_level->set_value(util::db_to_linear(value));
  s_label->set_text(level_to_localized_string(value, 0));
}

void AutoGainUi::on_new_integrated(const float& value) {
  i_level->set_value(util::db_to_linear(value));
  i_label->set_text(level_to_localized_string(value, 0));
}

void AutoGainUi::on_new_relative(const float& value) {
  r_level->set_value(util::db_to_linear(value));
  r_label->set_text(level_to_localized_string(value, 0));
}

void AutoGainUi::on_new_loudness(const float& value) {
  l_level->set_value(util::db_to_linear(value));
  l_label->set_text(level_to_localized_string(value, 0));
}

void AutoGainUi::on_new_range(const float& value) {
  lra_level->set_value(util::db_to_linear(value));
  lra_label->set_text(level_to_localized_string(value, 0));
}

void AutoGainUi::on_new_gain(const float& value) {
  g_level->set_value(value);
  g_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}
