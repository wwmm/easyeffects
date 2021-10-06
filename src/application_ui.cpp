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

#include "application_ui.hpp"

ApplicationUi::ApplicationUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& builder,
                             Application* application)
    : Gtk::ApplicationWindow(cobject), app(application), settings(app->settings) {
  apply_css_style("custom.css");

  // loading builder widgets

  stack = builder->get_widget<Gtk::Stack>("stack");
  stack_menu_settings = builder->get_widget<Gtk::Stack>("stack_menu_settings");
  presets_menu_button = builder->get_widget<Gtk::MenuButton>("presets_menu_button");
  bypass_button = builder->get_widget<Gtk::ToggleButton>("bypass_button");
  toggle_output = builder->get_widget<Gtk::ToggleButton>("toggle_output");
  toggle_input = builder->get_widget<Gtk::ToggleButton>("toggle_input");
  toggle_pipe_info = builder->get_widget<Gtk::ToggleButton>("toggle_pipe_info");

  presets_menu_ui = PresetsMenuUi::create(app);
  GeneralSettingsUi::add_to_stack(stack_menu_settings, app);
  SpectrumSettingsUi::add_to_stack(stack_menu_settings, app);

  auto icon_theme = setup_icon_theme();

  soe_ui = StreamOutputEffectsUi::add_to_stack(stack, app->soe.get(), icon_theme);
  sie_ui = StreamInputEffectsUi::add_to_stack(stack, app->sie.get(), icon_theme);
  pipe_info_ui = PipeInfoUi::add_to_stack(stack, app->pm.get(), app->presets_manager.get());

  presets_menu_button->set_popover(*presets_menu_ui);

  soe_ui->set_transient_window(this);
  sie_ui->set_transient_window(this);

  // signals

  toggle_output->signal_toggled().connect([&, this]() {
    if (toggle_output->get_active()) {
      stack->get_pages()->select_item(0, true);
    }
  });

  toggle_input->signal_toggled().connect([&, this]() {
    if (toggle_input->get_active()) {
      stack->get_pages()->select_item(1, true);
    }
  });

  toggle_pipe_info->signal_toggled().connect([&, this]() {
    if (toggle_pipe_info->get_active()) {
      stack->get_pages()->select_item(2, true);
    }
  });

  // binding properties to gsettings keys

  settings->bind("use-dark-theme", Gtk::Settings::get_default().get(), "gtk_application_prefer_dark_theme");
  settings->bind("bypass", bypass_button, "active");

  // restore window size

  if (const auto& window_width = settings->get_int("window-width"), window_height = settings->get_int("window-height");
      window_width > 0 && window_height > 0) {
    set_default_size(window_width, window_height);
  }
}

ApplicationUi::~ApplicationUi() {
  for (auto& c : connections) {
    c.disconnect();
  }

  presets_menu_ui->unreference();

  util::debug(log_tag + "destroyed");
}

auto ApplicationUi::create(Application* app_this) -> ApplicationUi* {
  const auto& builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/application_window.ui");

  return Gtk::Builder::get_widget_derived<ApplicationUi>(builder, "ApplicationUi", app_this);
}

void ApplicationUi::apply_css_style(const std::string& css_file_name) {
  auto provider = Gtk::CssProvider::create();

  provider->load_from_resource("/com/github/wwmm/easyeffects/ui/" + css_file_name);

  const auto& display = Gdk::Display::get_default();
  const auto& priority = GTK_STYLE_PROVIDER_PRIORITY_APPLICATION;

  Gtk::StyleContext::add_provider_for_display(display, provider, priority);
}

auto ApplicationUi::setup_icon_theme() -> Glib::RefPtr<Gtk::IconTheme> {
  try {
    Glib::RefPtr<Gtk::IconTheme> ic_theme = Gtk::IconTheme::get_for_display(Gdk::Display::get_default());

    const auto& icon_theme_name = ic_theme->get_theme_name();

    if (icon_theme_name.empty()) {
      util::debug("application_ui: Icon Theme detected, but the name is empty");
    } else {
      util::debug("application_ui: Icon Theme " + icon_theme_name.raw() + " detected");
    }

    return ic_theme;
  } catch (...) {
    util::warning("application_ui: Can't retrieve the icon theme in use on the system. App icons won't be shown.");

    return nullptr;
  }
}
