/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "limiter_ui.hpp"

namespace {

auto mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "Herm Thin") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Herm Wide") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "Herm Tail") == 0) {
    g_value_set_int(value, 2);
  } else if (std::strcmp(v, "Herm Duck") == 0) {
    g_value_set_int(value, 3);
  } else if (std::strcmp(v, "Exp Thin") == 0) {
    g_value_set_int(value, 4);
  } else if (std::strcmp(v, "Exp Wide") == 0) {
    g_value_set_int(value, 5);
  } else if (std::strcmp(v, "Exp Tail") == 0) {
    g_value_set_int(value, 6);
  } else if (std::strcmp(v, "Exp Duck") == 0) {
    g_value_set_int(value, 7);
  } else if (std::strcmp(v, "Line Thin") == 0) {
    g_value_set_int(value, 8);
  } else if (std::strcmp(v, "Line Wide") == 0) {
    g_value_set_int(value, 9);
  } else if (std::strcmp(v, "Line Tail") == 0) {
    g_value_set_int(value, 10);
  } else if (std::strcmp(v, "Line Duck") == 0) {
    g_value_set_int(value, 11);
  }

  return 1;
}

auto int_to_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("Herm Thin");

    case 1:
      return g_variant_new_string("Herm Wide");

    case 2:
      return g_variant_new_string("Herm Tail");

    case 3:
      return g_variant_new_string("Herm Duck");

    case 4:
      return g_variant_new_string("Exp Thin");

    case 5:
      return g_variant_new_string("Exp Wide");

    case 6:
      return g_variant_new_string("Exp Tail");

    case 7:
      return g_variant_new_string("Exp Duck");

    case 8:
      return g_variant_new_string("Line Thin");

    case 9:
      return g_variant_new_string("Line Wide");

    case 10:
      return g_variant_new_string("Line Tail");

    case 11:
      return g_variant_new_string("Line Duck");

    default:
      return g_variant_new_string("Herm Thin");
  }
}

auto ovs_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "None") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Half x2(2L)") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "Half x2(3L)") == 0) {
    g_value_set_int(value, 2);
  } else if (std::strcmp(v, "Half x3(2L)") == 0) {
    g_value_set_int(value, 3);
  } else if (std::strcmp(v, "Half x3(3L)") == 0) {
    g_value_set_int(value, 4);
  } else if (std::strcmp(v, "Half x4(2L)") == 0) {
    g_value_set_int(value, 5);
  } else if (std::strcmp(v, "Half x4(3L)") == 0) {
    g_value_set_int(value, 6);
  } else if (std::strcmp(v, "Half x6(2L)") == 0) {
    g_value_set_int(value, 7);
  } else if (std::strcmp(v, "Half x6(3L)") == 0) {
    g_value_set_int(value, 8);
  } else if (std::strcmp(v, "Half x8(2L)") == 0) {
    g_value_set_int(value, 9);
  } else if (std::strcmp(v, "Half x8(3L)") == 0) {
    g_value_set_int(value, 10);
  } else if (std::strcmp(v, "Full x2(2L)") == 0) {
    g_value_set_int(value, 11);
  } else if (std::strcmp(v, "Full x2(3L)") == 0) {
    g_value_set_int(value, 12);
  } else if (std::strcmp(v, "Full x3(2L)") == 0) {
    g_value_set_int(value, 13);
  } else if (std::strcmp(v, "Full x3(3L)") == 0) {
    g_value_set_int(value, 14);
  } else if (std::strcmp(v, "Full x4(2L)") == 0) {
    g_value_set_int(value, 15);
  } else if (std::strcmp(v, "Full x4(3L)") == 0) {
    g_value_set_int(value, 16);
  } else if (std::strcmp(v, "Full x6(2L)") == 0) {
    g_value_set_int(value, 17);
  } else if (std::strcmp(v, "Full x6(3L)") == 0) {
    g_value_set_int(value, 18);
  } else if (std::strcmp(v, "Full x8(2L)") == 0) {
    g_value_set_int(value, 19);
  } else if (std::strcmp(v, "Full x8(3L)") == 0) {
    g_value_set_int(value, 20);
  }

  return 1;
}

auto int_to_ovs_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("None");

    case 1:
      return g_variant_new_string("Half x2(2L)");

    case 2:
      return g_variant_new_string("Half x2(3L)");

    case 3:
      return g_variant_new_string("Half x3(2L)");

    case 4:
      return g_variant_new_string("Half x3(3L)");

    case 5:
      return g_variant_new_string("Half x4(2L)");

    case 6:
      return g_variant_new_string("Half x4(3L)");

    case 7:
      return g_variant_new_string("Half x6(2L)");

    case 8:
      return g_variant_new_string("Half x6(3L)");

    case 9:
      return g_variant_new_string("Half x8(2L)");

    case 10:
      return g_variant_new_string("Half x8(3L)");

    case 11:
      return g_variant_new_string("Full x2(2L)");

    case 12:
      return g_variant_new_string("Full x2(3L)");

    case 13:
      return g_variant_new_string("Full x3(2L)");

    case 14:
      return g_variant_new_string("Full x3(3L)");

    case 15:
      return g_variant_new_string("Full x4(2L)");

    case 16:
      return g_variant_new_string("Full x4(3L)");

    case 17:
      return g_variant_new_string("Full x6(2L)");

    case 18:
      return g_variant_new_string("Full x6(3L)");

    case 19:
      return g_variant_new_string("Full x8(2L)");

    case 20:
      return g_variant_new_string("Full x8(3L)");

    default:
      return g_variant_new_string("None");
  }
}

auto dither_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "None") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "7bit") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "8bit") == 0) {
    g_value_set_int(value, 2);
  } else if (std::strcmp(v, "11bit") == 0) {
    g_value_set_int(value, 3);
  } else if (std::strcmp(v, "12bit") == 0) {
    g_value_set_int(value, 4);
  } else if (std::strcmp(v, "15bit") == 0) {
    g_value_set_int(value, 5);
  } else if (std::strcmp(v, "16bit") == 0) {
    g_value_set_int(value, 6);
  } else if (std::strcmp(v, "23bit") == 0) {
    g_value_set_int(value, 7);
  } else if (std::strcmp(v, "24bit") == 0) {
    g_value_set_int(value, 8);
  }

  return 1;
}

auto int_to_dither_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("None");

    case 1:
      return g_variant_new_string("7bit");

    case 2:
      return g_variant_new_string("8bit");

    case 3:
      return g_variant_new_string("11bit");

    case 4:
      return g_variant_new_string("12bit");

    case 5:
      return g_variant_new_string("15bit");

    case 6:
      return g_variant_new_string("16bit");

    case 7:
      return g_variant_new_string("23bit");

    case 8:
      return g_variant_new_string("24bit");

    default:
      return g_variant_new_string("None");
  }
}

}  // namespace

LimiterUi::LimiterUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     const std::string& schema,
                     const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = plugin_name::limiter;

  // loading builder widgets

  input_gain = builder->get_widget<Gtk::Scale>("input_gain");
  output_gain = builder->get_widget<Gtk::Scale>("output_gain");

  mode = builder->get_widget<Gtk::ComboBoxText>("mode");
  oversampling = builder->get_widget<Gtk::ComboBoxText>("oversampling");
  dither = builder->get_widget<Gtk::ComboBoxText>("dither");

  sc_preamp = builder->get_widget<Gtk::SpinButton>("sc_preamp");
  lookahead = builder->get_widget<Gtk::SpinButton>("lookahead");
  attack = builder->get_widget<Gtk::SpinButton>("attack");
  release = builder->get_widget<Gtk::SpinButton>("release");
  threshold = builder->get_widget<Gtk::SpinButton>("threshold");
  stereo_link = builder->get_widget<Gtk::SpinButton>("stereo_link");
  alr_attack = builder->get_widget<Gtk::SpinButton>("alr_attack");
  alr_release = builder->get_widget<Gtk::SpinButton>("alr_release");
  alr_knee = builder->get_widget<Gtk::SpinButton>("alr_knee");

  boost = builder->get_widget<Gtk::CheckButton>("gain_boost");
  alr = builder->get_widget<Gtk::ToggleButton>("alr");

  gain_left = builder->get_widget<Gtk::Label>("gain_left_label");
  gain_right = builder->get_widget<Gtk::Label>("gain_right_label");
  sidechain_left = builder->get_widget<Gtk::Label>("sidechain_left_label");
  sidechain_right = builder->get_widget<Gtk::Label>("sidechain_right_label");

  // gsettings bindings

  settings->bind("input-gain", input_gain->get_adjustment().get(), "value");
  settings->bind("output-gain", output_gain->get_adjustment().get(), "value");
  settings->bind("sidechain-preamp", sc_preamp->get_adjustment().get(), "value");
  settings->bind("lookahead", lookahead->get_adjustment().get(), "value");
  settings->bind("attack", attack->get_adjustment().get(), "value");
  settings->bind("release", release->get_adjustment().get(), "value");
  settings->bind("threshold", threshold->get_adjustment().get(), "value");
  settings->bind("stereo-link", stereo_link->get_adjustment().get(), "value");
  settings->bind("gain-boost", boost, "active");
  settings->bind("alr", alr, "active");
  settings->bind("alr-attack", alr_attack->get_adjustment().get(), "value");
  settings->bind("alr-release", alr_release->get_adjustment().get(), "value");
  settings->bind("alr-knee", alr_knee->get_adjustment().get(), "value");

  g_settings_bind_with_mapping(settings->gobj(), "mode", mode->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               mode_enum_to_int, int_to_mode_enum, nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "oversampling", oversampling->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, ovs_enum_to_int, int_to_ovs_enum, nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "dithering", dither->gobj(), "active",
                              G_SETTINGS_BIND_DEFAULT, dither_enum_to_int, int_to_dither_enum, nullptr, nullptr);

  // prepare widgets

  prepare_scale(input_gain, "");
  prepare_scale(output_gain, "");

  prepare_spinbutton(sc_preamp, "db");
  prepare_spinbutton(lookahead, "ms");
  prepare_spinbutton(attack, "ms");
  prepare_spinbutton(release, "ms");
  prepare_spinbutton(threshold, "db");
  prepare_spinbutton(stereo_link, "%");
  prepare_spinbutton(alr_attack, "ms");
  prepare_spinbutton(alr_release, "ms");
  prepare_spinbutton(alr_knee, "db");

  // set alr spinbuttons sensitive on alr button

  auto set_alr_spinbuttons_sensitivity = [=, this]() {
    const auto& active = alr->get_active();

    alr_attack->set_sensitive(active);
    alr_release->set_sensitive(active);
    alr_knee->set_sensitive(active);
  };

  set_alr_spinbuttons_sensitivity();

  alr->signal_toggled().connect(set_alr_spinbuttons_sensitivity);
}

LimiterUi::~LimiterUi() {
  util::debug(name + " ui destroyed");
}

auto LimiterUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> LimiterUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/limiter.ui");

  auto* ui = Gtk::Builder::get_widget_derived<LimiterUi>(builder, "top_box", "com.github.wwmm.easyeffects.limiter",
                                                         schema_path + "limiter/");

  auto stack_page = stack->add(*ui, plugin_name::limiter);

  return ui;
}

void LimiterUi::reset() {
  bypass->set_active(false);

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("mode");

  settings->reset("oversampling");

  settings->reset("dither");

  settings->reset("sidechain-preamp");

  settings->reset("lookahead");

  settings->reset("attack");

  settings->reset("release");

  settings->reset("threshold");

  settings->reset("gain-boost");

  settings->reset("stereo-link");

  settings->reset("alr");

  settings->reset("alr-attack");

  settings->reset("alr-release");

  settings->reset("alr-knee");
}

void LimiterUi::on_new_attenuation(double value) {
  //attenuation->set_value(1.0 - value);

  //attenuation_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}
