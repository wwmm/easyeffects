/*
 *  Copyright © 2017-2020 Wellington Wallace
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

#include "pitch_ui.hpp"

PitchUi::PitchUi(BaseObjectType* cobject,
                 const Glib::RefPtr<Gtk::Builder>& builder,
                 const std::string& schema,
                 const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = plugin_name::pitch;

  // loading builder widgets

  input_gain = builder->get_widget<Gtk::Scale>("input_gain");
  output_gain = builder->get_widget<Gtk::Scale>("output_gain");

  crispness = builder->get_widget<Gtk::SpinButton>("crispness");
  cents = builder->get_widget<Gtk::SpinButton>("cents");
  semitones = builder->get_widget<Gtk::SpinButton>("semitones");
  octaves = builder->get_widget<Gtk::SpinButton>("octaves");

  faster = builder->get_widget<Gtk::ToggleButton>("faster");
  formant_preserving = builder->get_widget<Gtk::ToggleButton>("formant_preserving");

  // gsettings bindings

  settings->bind("input-gain", input_gain->get_adjustment().get(), "value");
  settings->bind("output-gain", output_gain->get_adjustment().get(), "value");
  settings->bind("cents", cents->get_adjustment().get(), "value");
  settings->bind("crispness", crispness->get_adjustment().get(), "value");
  settings->bind("semitones", semitones->get_adjustment().get(), "value");
  settings->bind("octaves", octaves->get_adjustment().get(), "value");
  settings->bind("faster", faster, "active");
  settings->bind("formant-preserving", formant_preserving, "active");

  prepare_scale(input_gain, "");
  prepare_scale(output_gain, "");
}

PitchUi::~PitchUi() {
  util::debug(name + " ui destroyed");
}

auto PitchUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> PitchUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/pitch.ui");

  auto* ui = Gtk::Builder::get_widget_derived<PitchUi>(builder, "top_box", "com.github.wwmm.easyeffects.pitch",
                                                       schema_path + "pitch/");

  auto stack_page = stack->add(*ui, plugin_name::pitch);

  return ui;
}

void PitchUi::reset() {
  bypass->set_active(false);

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("cents");

  settings->reset("semitones");

  settings->reset("octaves");

  settings->reset("crispness");

  settings->reset("formant-preserving");

  settings->reset("faster");
}
