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

#include "crossfeed_ui.hpp"

CrossfeedUi::CrossfeedUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& schema,
                         const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = plugin_name::crossfeed;

  // loading builder widgets

  fcut = builder->get_widget<Gtk::SpinButton>("fcut");
  feed = builder->get_widget<Gtk::SpinButton>("feed");

  preset_cmoy = builder->get_widget<Gtk::Button>("preset_cmoy");
  preset_default = builder->get_widget<Gtk::Button>("preset_default");
  preset_jmeier = builder->get_widget<Gtk::Button>("preset_jmeier");

  // gsettings bindings

  settings->bind("fcut", fcut->get_adjustment().get(), "value");
  settings->bind("feed", feed->get_adjustment().get(), "value");

  prepare_spinbutton(fcut, "Hz");
  prepare_spinbutton(feed, "dB");

  init_presets_buttons();

  setup_input_output_gain(builder);
}

CrossfeedUi::~CrossfeedUi() {
  util::debug(name + " ui destroyed");
}

auto CrossfeedUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> CrossfeedUi* {
  const auto& builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/crossfeed.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<CrossfeedUi>(builder, "top_box", "com.github.wwmm.easyeffects.crossfeed",
                                                           schema_path + "crossfeed/");

  stack->add(*ui, plugin_name::crossfeed);

  return ui;
}

void CrossfeedUi::reset() {
  bypass->set_active(false);

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("fcut");

  settings->reset("feed");
}

void CrossfeedUi::init_presets_buttons() {
  preset_cmoy->signal_clicked().connect([=, this]() {
    fcut->set_value(700.0);
    feed->set_value(6.0);
  });

  preset_default->signal_clicked().connect([=, this]() {
    fcut->set_value(700.0);
    feed->set_value(4.5);
  });

  preset_jmeier->signal_clicked().connect([=, this]() {
    fcut->set_value(650.0);
    feed->set_value(9.0);
  });
}
