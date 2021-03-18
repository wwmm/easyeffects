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

#include "crossfeed_ui.hpp"

CrossfeedUi::CrossfeedUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& schema,
                         const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = "crossfeed";

  // loading glade widgets

  builder->get_widget("preset_cmoy", preset_cmoy);
  builder->get_widget("preset_default", preset_default);
  builder->get_widget("preset_jmeier", preset_jmeier);
  builder->get_widget("plugin_reset", reset_button);

  get_object(builder, "fcut", fcut);
  get_object(builder, "feed", feed);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("fcut", fcut.get(), "value", flag);
  settings->bind("feed", feed.get(), "value", flag);

  // reset plugin
  reset_button->signal_clicked().connect([=, this]() { reset(); });

  init_presets_buttons();
}

CrossfeedUi::~CrossfeedUi() {
  util::debug(name + " ui destroyed");
}

void CrossfeedUi::reset() {
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
