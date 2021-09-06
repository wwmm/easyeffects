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

#include "bass_loudness_ui.hpp"

BassLoudnessUi::BassLoudnessUi(BaseObjectType* cobject,
                               const Glib::RefPtr<Gtk::Builder>& builder,
                               const std::string& schema,
                               const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = plugin_name::bass_loudness;

  // loading builder widgets

  loudness = builder->get_widget<Gtk::SpinButton>("loudness");
  output = builder->get_widget<Gtk::SpinButton>("output");
  link = builder->get_widget<Gtk::SpinButton>("link");

  // gsettings bindings

  settings->bind("loudness", loudness->get_adjustment().get(), "value");
  settings->bind("output", output->get_adjustment().get(), "value");
  settings->bind("link", link->get_adjustment().get(), "value");

  prepare_spinbutton(loudness, "dB");
  prepare_spinbutton(output, "dB");
  prepare_spinbutton(link, "dB");

  setup_input_output_gain(builder);
}

BassLoudnessUi::~BassLoudnessUi() {
  util::debug(name + " ui destroyed");
}

auto BassLoudnessUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> BassLoudnessUi* {
  const auto& builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/bass_loudness.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<BassLoudnessUi>(
      builder, "top_box", "com.github.wwmm.easyeffects.bassloudness", schema_path + "bassloudness/");

  stack->add(*ui, plugin_name::bass_loudness);

  return ui;
}

void BassLoudnessUi::reset() {
  bypass->set_active(false);

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("loudness");

  settings->reset("output");

  settings->reset("link");

  settings->reset("listen");
}
