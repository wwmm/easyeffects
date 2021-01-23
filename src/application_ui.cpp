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
    : Gtk::ApplicationWindow(cobject), app(application),
      settings(app->settings), global_locale(util::get_global_locale()) {
  apply_css_style("custom.css");

  Gtk::IconTheme::get_default()->add_resource_path("/com/github/wwmm/pulseeffects/icons");

  // loading glade widgets

  builder->get_widget("stack", stack);
  builder->get_widget("stack_menu_settings", stack_menu_settings);
  builder->get_widget("presets_menu_button", presets_menu_button);
  builder->get_widget("presets_menu", presets_menu);
  builder->get_widget("presets_menu_label", presets_menu_label);

  builder->get_widget("calibration_button", calibration_button);

  builder->get_widget("subtitle_grid", subtitle_grid);
  builder->get_widget("headerbar", headerbar);
  builder->get_widget("help_button", help_button);
  builder->get_widget("bypass_button", bypass_button);
  builder->get_widget("headerbar_icon1", headerbar_icon1);
  builder->get_widget("headerbar_icon2", headerbar_icon2);
  builder->get_widget("headerbar_info", headerbar_info);

  presets_menu_ui = PresetsMenuUi::add_to_popover(presets_menu, app);
  sie_ui = SinkInputEffectsUi::add_to_stack(stack, app->sie.get());
  soe_ui = SourceOutputEffectsUi::add_to_stack(stack, app->soe.get());
  GeneralSettingsUi::add_to_stack(stack_menu_settings, app);
  SpectrumSettingsUi::add_to_stack(stack_menu_settings, app);
  PulseSettingsUi::add_to_stack(stack_menu_settings, app);
  BlocklistSettingsUi::add_to_stack(stack_menu_settings);
  pulse_info_ui = PulseInfoUi::add_to_stack(stack, app->pm.get());

  stack->connect_property_changed("visible-child",
                                  sigc::mem_fun(*this, &ApplicationUi::on_stack_visible_child_changed));

  // calibration

  calibration_button->signal_clicked().connect(sigc::mem_fun(*this, &ApplicationUi::on_calibration_button_clicked));

  // signals

  connections.emplace_back(app->pm->new_default_sink.connect([&](auto name) {
    if (stack->get_visible_child_name() == "sink_inputs") {
      update_headerbar_subtitle(0);
    }
  }));

  connections.emplace_back(app->pm->new_default_source.connect([&](auto name) {
    if (stack->get_visible_child_name() == "source_outputs") {
      update_headerbar_subtitle(1);
    }
  }));

  help_button->signal_clicked().connect([=]() { app->activate_action("help"); });

  presets_menu_button->signal_clicked().connect(
      sigc::mem_fun(*presets_menu_ui, &PresetsMenuUi::on_presets_menu_button_clicked));

  presets_menu_label->set_text(settings->get_string("last-used-output-preset"));

  connections.emplace_back(settings->signal_changed("last-used-input-preset").connect([=](auto key) {
    presets_menu_label->set_text(settings->get_string("last-used-input-preset"));
  }));

  connections.emplace_back(settings->signal_changed("last-used-output-preset").connect([=](auto key) {
    presets_menu_label->set_text(settings->get_string("last-used-output-preset"));
  }));

  // headerbar info

  connections.emplace_back(app->sie->new_latency.connect([=](int latency) {
    sie_latency = latency;

    if (stack->get_visible_child_name() == "sink_inputs") {
      update_headerbar_subtitle(0);
    }
  }));

  if (app->sie->playing) {
    app->sie->get_latency();
  }

  connections.emplace_back(app->soe->new_latency.connect([=](int latency) {
    soe_latency = latency;

    if (stack->get_visible_child_name() == "source_outputs") {
      update_headerbar_subtitle(1);
    }
  }));

  if (app->soe->playing) {
    app->soe->get_latency();
  }

  // updating headerbar info

  update_headerbar_subtitle(0);

  // binding glade widgets to gsettings keys

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("use-dark-theme", Gtk::Settings::get_default().get(), "gtk_application_prefer_dark_theme", flag);
  settings->bind("bypass", bypass_button, "active", flag);

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

  util::debug(log_tag + "destroyed");
}

auto ApplicationUi::create(Application* app_this) -> ApplicationUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/application.glade");

  ApplicationUi* window = nullptr;

  builder->get_widget_derived("ApplicationUi", window, app_this);

  return window;
}

void ApplicationUi::apply_css_style(const std::string& css_file_name) {
  auto provider = Gtk::CssProvider::create();

  provider->load_from_resource("/com/github/wwmm/pulseeffects/ui/" + css_file_name);

  auto screen = Gdk::Screen::get_default();
  auto priority = GTK_STYLE_PROVIDER_PRIORITY_APPLICATION;

  Gtk::StyleContext::add_provider_for_screen(screen, provider, priority);
}

void ApplicationUi::update_headerbar_subtitle(const int& index) {
  std::ostringstream null_sink_rate;
  std::ostringstream current_dev_rate;
  const float khz_factor = 0.001F;

  null_sink_rate.imbue(global_locale);
  null_sink_rate.precision(1);

  current_dev_rate.imbue(global_locale);
  current_dev_rate.precision(1);

  switch (index) {
    case 0: {  // sie

      subtitle_grid->show();

      headerbar_icon1->set_from_icon_name("emblem-music-symbolic", Gtk::ICON_SIZE_MENU);

      headerbar_icon2->set_from_icon_name("audio-speakers-symbolic", Gtk::ICON_SIZE_MENU);

      null_sink_rate << std::fixed << app->pm->apps_sink_info->rate * khz_factor << "kHz";

      auto sink = app->pm->get_sink_info(app->pm->server_info.default_sink_name);

      current_dev_rate << std::fixed << sink->rate * khz_factor << "kHz";

      headerbar_info->set_text(" ⟶ " + app->pm->apps_sink_info->format + " " + null_sink_rate.str() +
                               " ⟶ float32le " + null_sink_rate.str() + " ⟶ " + sink->format + " " +
                               current_dev_rate.str() + " ⟶ " + std::to_string(sie_latency) + "ms ⟶ ");

      break;
    }
    case 1: {  // soe

      subtitle_grid->show();

      headerbar_icon1->set_from_icon_name("audio-input-microphone-symbolic", Gtk::ICON_SIZE_MENU);

      headerbar_icon2->set_from_icon_name("emblem-music-symbolic", Gtk::ICON_SIZE_MENU);

      null_sink_rate << std::fixed << app->pm->mic_sink_info->rate * khz_factor << "kHz";

      auto source = app->pm->get_source_info(app->pm->server_info.default_source_name);

      current_dev_rate << std::fixed << source->rate * khz_factor << "kHz";

      headerbar_info->set_text(" ⟶ " + source->format + " " + current_dev_rate.str() + " ⟶ float32le " +
                               null_sink_rate.str() + " ⟶ " + app->pm->mic_sink_info->format + " " +
                               null_sink_rate.str() + " ⟶ " + std::to_string(soe_latency) + "ms ⟶ ");

      break;
    }
    default:  // pulse info

      subtitle_grid->hide();

      break;
  }
}

void ApplicationUi::on_stack_visible_child_changed() {
  auto name = stack->get_visible_child_name();

  if (name == std::string("sink_inputs")) {
    update_headerbar_subtitle(0);

    presets_menu_label->set_text(settings->get_string("last-used-output-preset"));
  } else if (name == std::string("source_outputs")) {
    update_headerbar_subtitle(1);

    presets_menu_label->set_text(settings->get_string("last-used-input-preset"));
  } else if (name == std::string("pulse_info")) {
    update_headerbar_subtitle(2);
  }
}

void ApplicationUi::on_calibration_button_clicked() {
  std::shared_ptr<CalibrationUi> calibration_ui(CalibrationUi::create());

  auto c = app->pm->new_default_source.connect([=](auto name) { calibration_ui->set_source_monitor_name(name); });

  calibration_ui->signal_hide().connect([=]() { c->disconnect(); });

  calibration_ui->show_all();
}
