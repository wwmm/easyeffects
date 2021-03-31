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

#include "application_ui.hpp"

ApplicationUi::ApplicationUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& builder,
                             Application* application)
    : Gtk::ApplicationWindow(cobject), app(application), settings(app->settings) {
  apply_css_style("custom.css");

  Gtk::IconTheme::get_for_display(Gdk::Display::get_default())
      ->add_resource_path("/com/github/wwmm/pulseeffects/icons");

  // loading builder widgets

  stack = builder->get_widget<Gtk::Stack>("stack");
  stack_menu_settings = builder->get_widget<Gtk::Stack>("stack_menu_settings");
  presets_menu_button = builder->get_widget<Gtk::MenuButton>("presets_menu_button");
  calibration_button = builder->get_widget<Gtk::Button>("calibration_button");
  help_button = builder->get_widget<Gtk::Button>("help_button");
  bypass_button = builder->get_widget<Gtk::ToggleButton>("bypass_button");
  toggle_output = builder->get_widget<Gtk::ToggleButton>("toggle_output");
  toggle_input = builder->get_widget<Gtk::ToggleButton>("toggle_input");
  toggle_pipe_info = builder->get_widget<Gtk::ToggleButton>("toggle_pipe_info");

  presets_menu_ui = PresetsMenuUi::create(app);
  GeneralSettingsUi::add_to_stack(stack_menu_settings, app);
  SpectrumSettingsUi::add_to_stack(stack_menu_settings, app);

  soe_ui = StreamOutputEffectsUi::add_to_stack(stack, app->soe.get());
  sie_ui = StreamInputEffectsUi::add_to_stack(stack, app->sie.get());
  pipe_info_ui = PipeInfoUi::add_to_stack(stack, app->pm.get());
  // PipeSettingsUi::add_to_stack(stack_menu_settings, app);

  presets_menu_button->set_popover(*presets_menu_ui);

  soe_ui->set_transient_window(this);
  sie_ui->set_transient_window(this);

  // signals

  stack_model = stack->get_pages();

  stack->get_pages()->signal_selection_changed().connect([&, this](guint position, guint n_items) {
    toggle_output->set_active(stack_model->is_selected(0));
    toggle_input->set_active(stack_model->is_selected(1));
    toggle_pipe_info->set_active(stack_model->is_selected(2));
  });

  toggle_output->signal_toggled().connect([&, this]() {
    if (toggle_output->get_active()) {
      stack->get_pages()->select_item(0, true);
    } else {
      toggle_output->set_active(stack_model->is_selected(0));
    }
  });

  toggle_input->signal_toggled().connect([&, this]() {
    if (toggle_input->get_active()) {
      stack->get_pages()->select_item(1, true);
    } else {
      toggle_input->set_active(stack_model->is_selected(1));
    }
  });

  toggle_pipe_info->signal_toggled().connect([&, this]() {
    if (toggle_pipe_info->get_active()) {
      stack->get_pages()->select_item(2, true);
    } else {
      toggle_pipe_info->set_active(stack_model->is_selected(2));
    }
  });

  help_button->signal_clicked().connect([=, this]() { app->activate_action("help"); });

  calibration_button->signal_clicked().connect(sigc::mem_fun(*this, &ApplicationUi::on_calibration_button_clicked));

  // binding properties to gsettings keys

  settings->bind("use-dark-theme", Gtk::Settings::get_default().get(), "gtk_application_prefer_dark_theme");
  settings->bind("bypass", bypass_button, "active");

  // restore window size

  auto window_width = settings->get_int("window-width");
  auto window_height = settings->get_int("window-height");

  if (window_width > 0 && window_height > 0) {
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
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/application_window.ui");

  return Gtk::Builder::get_widget_derived<ApplicationUi>(builder, "ApplicationUi", app_this);
}

void ApplicationUi::apply_css_style(const std::string& css_file_name) {
  auto provider = Gtk::CssProvider::create();

  provider->load_from_resource("/com/github/wwmm/pulseeffects/ui/" + css_file_name);

  auto display = Gdk::Display::get_default();
  auto priority = GTK_STYLE_PROVIDER_PRIORITY_APPLICATION;

  Gtk::StyleContext::add_provider_for_display(display, provider, priority);
}

void ApplicationUi::on_calibration_button_clicked() {
  // calibration_ui = CalibrationUi::create();

  // auto c = app->pm->new_default_source.connect(
  //     [=](const NodeInfo& node_info) { calibration_ui->set_input_node_id(node_info.id); });

  // calibration_ui->signal_hide().connect([=]() {
  //   c->disconnect();

  //   delete calibration_ui;
  // });

  // calibration_ui->set_transient_for(*this);

  // calibration_ui->show_all();
}
