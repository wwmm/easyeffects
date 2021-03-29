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

#include "deesser_ui.hpp"
#include <cstring>

namespace {

auto detection_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "RMS") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Peak") == 0) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_detection_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("RMS");

    case 1:
      return g_variant_new_string("Peak");

    default:
      return g_variant_new_string("RMS");
  }
}

auto mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "Wide") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Split") == 0) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("Wide");

    case 1:
      return g_variant_new_string("Split");

    default:
      return g_variant_new_string("Wide");
  }
}

}  // namespace

DeesserUi::DeesserUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     const std::string& schema,
                     const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = plugin_name::deesser;

  // loading builder widgets

  input_gain = builder->get_widget<Gtk::Scale>("input_gain");
  output_gain = builder->get_widget<Gtk::Scale>("output_gain");

  makeup = builder->get_widget<Gtk::SpinButton>("makeup");
  ratio = builder->get_widget<Gtk::SpinButton>("ratio");
  threshold = builder->get_widget<Gtk::SpinButton>("threshold");
  ratio = builder->get_widget<Gtk::SpinButton>("ratio");
  f1_freq = builder->get_widget<Gtk::SpinButton>("f1_freq");
  f2_freq = builder->get_widget<Gtk::SpinButton>("f2_freq");
  f1_level = builder->get_widget<Gtk::SpinButton>("f1_level");
  f2_level = builder->get_widget<Gtk::SpinButton>("f2_level");
  f2_q = builder->get_widget<Gtk::SpinButton>("f2_q");
  laxity = builder->get_widget<Gtk::SpinButton>("laxity");

  mode = builder->get_widget<Gtk::ComboBoxText>("mode");
  detection = builder->get_widget<Gtk::ComboBoxText>("detection");

  compression = builder->get_widget<Gtk::LevelBar>("compression");
  detected = builder->get_widget<Gtk::LevelBar>("detected");

  compression_label = builder->get_widget<Gtk::Label>("compression_label");
  detected_label = builder->get_widget<Gtk::Label>("detected_label");

  sc_listen = builder->get_widget<Gtk::ToggleButton>("sc_listen");

  // gsettings bindings

  settings->bind("sc-listen", sc_listen, "active");
  settings->bind("makeup", makeup->get_adjustment().get(), "value");
  settings->bind("ratio", ratio->get_adjustment().get(), "value");
  settings->bind("threshold", threshold->get_adjustment().get(), "value");
  settings->bind("f1-freq", f1_freq->get_adjustment().get(), "value");
  settings->bind("f2-freq", f2_freq->get_adjustment().get(), "value");
  settings->bind("f1-level", f1_level->get_adjustment().get(), "value");
  settings->bind("f2-level", f2_level->get_adjustment().get(), "value");
  settings->bind("f2-q", f2_q->get_adjustment().get(), "value");
  settings->bind("laxity", laxity->get_adjustment().get(), "value");

  g_settings_bind_with_mapping(settings->gobj(), "detection", detection->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               detection_enum_to_int, int_to_detection_enum, nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "mode", mode->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               mode_enum_to_int, int_to_mode_enum, nullptr, nullptr);

  prepare_spinbutton(makeup, "dB");
  prepare_spinbutton(threshold, "dB");
  prepare_spinbutton(f1_level, "dB");
  prepare_spinbutton(f2_level, "dB");

  prepare_spinbutton(f1_freq, "Hz");
  prepare_spinbutton(f2_freq, "Hz");
}

DeesserUi::~DeesserUi() {
  util::debug(name + " ui destroyed");
}

auto DeesserUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> DeesserUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/deesser.ui");

  auto* ui = Gtk::Builder::get_widget_derived<DeesserUi>(builder, "top_box", "com.github.wwmm.pulseeffects.deesser",
                                                         schema_path + "deesser/");

  auto stack_page = stack->add(*ui, plugin_name::deesser);

  return ui;
}

void DeesserUi::reset() {
  settings->reset("detection");

  settings->reset("mode");

  settings->reset("threshold");

  settings->reset("ratio");

  settings->reset("laxity");

  settings->reset("makeup");

  settings->reset("f1-freq");

  settings->reset("f2-freq");

  settings->reset("f1-level");

  settings->reset("f2-level");

  settings->reset("f2-q");

  settings->reset("sc-listen");
}

void DeesserUi::on_new_compression(double value) {
  compression->set_value(1.0 - value);

  compression_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}

void DeesserUi::on_new_detected(double value) {
  detected->set_value(value);

  detected_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}
