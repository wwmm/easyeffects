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

#include "reverb_ui.hpp"

namespace {

auto room_size_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (g_strcmp0(v, "Small") == 0) {
    g_value_set_int(value, 0);
  } else if (g_strcmp0(v, "Medium") == 0) {
    g_value_set_int(value, 1);
  } else if (g_strcmp0(v, "Large") == 0) {
    g_value_set_int(value, 2);
  } else if (g_strcmp0(v, "Tunnel-like") == 0) {
    g_value_set_int(value, 3);
  } else if (g_strcmp0(v, "Large/smooth") == 0) {
    g_value_set_int(value, 4);
  } else if (g_strcmp0(v, "Experimental") == 0) {
    g_value_set_int(value, 5);
  }

  return 1;
}

auto int_to_room_size_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  switch (g_value_get_int(value)) {
    case 0:
      return g_variant_new_string("Small");

    case 1:
      return g_variant_new_string("Medium");

    case 2:
      return g_variant_new_string("Large");

    case 3:
      return g_variant_new_string("Tunnel-like");

    case 4:
      return g_variant_new_string("Large/smooth");

    case 5:
      return g_variant_new_string("Experimental");

    default:
      return g_variant_new_string("Large");
  }
}

}  // namespace

ReverbUi::ReverbUi(BaseObjectType* cobject,
                   const Glib::RefPtr<Gtk::Builder>& builder,
                   const std::string& schema,
                   const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = plugin_name::reverb;

  // loading builder widgets

  input_gain = builder->get_widget<Gtk::Scale>("input_gain");
  output_gain = builder->get_widget<Gtk::Scale>("output_gain");
  predelay = builder->get_widget<Gtk::SpinButton>("predelay");
  decay_time = builder->get_widget<Gtk::SpinButton>("decay_time");
  diffusion = builder->get_widget<Gtk::SpinButton>("diffusion");
  amount = builder->get_widget<Gtk::SpinButton>("amount");
  dry = builder->get_widget<Gtk::SpinButton>("dry");
  hf_damp = builder->get_widget<Gtk::SpinButton>("hf_damp");
  bass_cut = builder->get_widget<Gtk::SpinButton>("bass_cut");
  treble_cut = builder->get_widget<Gtk::SpinButton>("treble_cut");
  room_size = builder->get_widget<Gtk::ComboBoxText>("room_size");
  preset_room = builder->get_widget<Gtk::Button>("preset_room");
  preset_empty_walls = builder->get_widget<Gtk::Button>("preset_empty_walls");
  preset_ambience = builder->get_widget<Gtk::Button>("preset_ambience");
  preset_large_empty_hall = builder->get_widget<Gtk::Button>("preset_large_empty_hall");
  preset_disco = builder->get_widget<Gtk::Button>("preset_disco");
  preset_large_occupied_hall = builder->get_widget<Gtk::Button>("preset_large_occupied_hall");

  // gsettings bindings

  settings->bind("input-gain", input_gain->get_adjustment().get(), "value");
  settings->bind("output-gain", output_gain->get_adjustment().get(), "value");
  settings->bind("predelay", predelay->get_adjustment().get(), "value");
  settings->bind("decay-time", decay_time->get_adjustment().get(), "value");
  settings->bind("diffusion", diffusion->get_adjustment().get(), "value");
  settings->bind("amount", amount->get_adjustment().get(), "value");
  settings->bind("dry", dry->get_adjustment().get(), "value");
  settings->bind("hf-damp", hf_damp->get_adjustment().get(), "value");
  settings->bind("bass-cut", bass_cut->get_adjustment().get(), "value");
  settings->bind("treble-cut", treble_cut->get_adjustment().get(), "value");

  g_settings_bind_with_mapping(settings->gobj(), "room-size", room_size->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               room_size_enum_to_int, int_to_room_size_enum, nullptr, nullptr);

  prepare_scale(input_gain, "");
  prepare_scale(output_gain, "");

  prepare_spinbutton(decay_time, "s");

  prepare_spinbutton(amount, "dB");
  prepare_spinbutton(dry, "dB");

  prepare_spinbutton(predelay, "ms");

  prepare_spinbutton(hf_damp, "Hz");
  prepare_spinbutton(bass_cut, "Hz");
  prepare_spinbutton(treble_cut, "Hz");

  prepare_spinbutton(diffusion, "");

  init_presets_buttons();
}

ReverbUi::~ReverbUi() {
  util::debug(name + " ui destroyed");
}

auto ReverbUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> ReverbUi* {
  const auto& builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/reverb.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<ReverbUi>(builder, "top_box", "com.github.wwmm.easyeffects.reverb",
                                                        schema_path + "reverb/");

  stack->add(*ui, plugin_name::reverb);

  return ui;
}

void ReverbUi::reset() {
  bypass->set_active(false);

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("room-size");

  settings->reset("decay-time");

  settings->reset("hf-damp");

  settings->reset("diffusion");

  settings->reset("amount");

  settings->reset("dry");

  settings->reset("predelay");

  settings->reset("bass-cut");

  settings->reset("treble-cut");
}

void ReverbUi::init_presets_buttons() {
  preset_room->signal_clicked().connect([=, this]() {
    decay_time->set_value(0.445945);
    hf_damp->set_value(5508.46);
    room_size->set_active(4);
    diffusion->set_value(0.54);
    amount->set_value(util::linear_to_db(0.469761));
    dry->set_value(util::linear_to_db(1.0));
    predelay->set_value(25.0);
    bass_cut->set_value(257.65);
    treble_cut->set_value(20000.0);
  });

  preset_empty_walls->signal_clicked().connect([=, this]() {
    decay_time->set_value(0.505687);
    hf_damp->set_value(3971.64);
    room_size->set_active(4);
    diffusion->set_value(0.17);
    amount->set_value(util::linear_to_db(0.198884));
    dry->set_value(util::linear_to_db(1.0));
    predelay->set_value(13.0);
    bass_cut->set_value(240.453);
    treble_cut->set_value(3303.47);
  });

  preset_ambience->signal_clicked().connect([=, this]() {
    decay_time->set_value(1.10354);
    hf_damp->set_value(2182.58);
    room_size->set_active(4);
    diffusion->set_value(0.69);
    amount->set_value(util::linear_to_db(0.291183));
    dry->set_value(util::linear_to_db(1.0));
    predelay->set_value(6.5);
    bass_cut->set_value(514.079);
    treble_cut->set_value(4064.15);
  });

  preset_large_empty_hall->signal_clicked().connect([=, this]() {
    decay_time->set_value(2.00689);
    hf_damp->set_value(20000.0);
    amount->set_value(util::linear_to_db(0.366022));
    settings->reset("room-size");
    settings->reset("diffusion");
    settings->reset("dry");
    settings->reset("predelay");
    settings->reset("bass-cut");
    settings->reset("treble-cut");
  });

  preset_disco->signal_clicked().connect([=, this]() {
    decay_time->set_value(1.0);
    hf_damp->set_value(3396.49);
    amount->set_value(util::linear_to_db(0.269807));
    settings->reset("room-size");
    settings->reset("diffusion");
    settings->reset("dry");
    settings->reset("predelay");
    settings->reset("bass-cut");
    settings->reset("treble-cut");
  });

  preset_large_occupied_hall->signal_clicked().connect([=, this]() {
    decay_time->set_value(1.45397);
    hf_damp->set_value(9795.58);
    amount->set_value(util::linear_to_db(0.184284));
    settings->reset("room-size");
    settings->reset("diffusion");
    settings->reset("dry");
    settings->reset("predelay");
    settings->reset("bass-cut");
    settings->reset("treble-cut");
  });
}
