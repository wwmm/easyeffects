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
#include <glibmm/i18n.h>
#include <memory>
#include <sstream>
// #include "blocklist_settings_ui.hpp"
// #include "calibration_ui.hpp"
// #include "general_settings_ui.hpp"
#include "gtkmm/label.h"
#include "gtkmm/menubutton.h"
#include "gtkmm/popovermenu.h"
#include "gtkmm/stack.h"
#include "pipe_manager.hpp"
// #include "pipe_settings_ui.hpp"
// #include "spectrum_settings_ui.hpp"

ApplicationUi::ApplicationUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& builder,
                             Application* application)
    : Gtk::ApplicationWindow(cobject), app(application), settings(app->settings) {
  apply_css_style("custom.css");

  // Gtk::IconTheme::get_default()->add_resource_path("/com/github/wwmm/pulseeffects/icons");

  // set locale (workaround for #849)

  try {
    global_locale = std::locale("");
  } catch (const std::exception& e) {
    global_locale = std::locale();
  }

  // loading builder widgets

  stack = builder->get_widget<Gtk::Stack>("stack");
  stack_menu_settings = builder->get_widget<Gtk::Stack>("stack_menu_settings");
  presets_menu_button = builder->get_widget<Gtk::MenuButton>("presets_menu_button");
  presets_menu = builder->get_widget<Gtk::Popover>("presets_menu");
  // presets_menu_label = builder->get_widget<Gtk::Label>("presets_menu_label");
  calibration_button = builder->get_widget<Gtk::Button>("calibration_button");
  subtitle_grid = builder->get_widget<Gtk::Grid>("subtitle_grid");
  headerbar = builder->get_widget<Gtk::HeaderBar>("headerbar");
  help_button = builder->get_widget<Gtk::Button>("help_button");
  bypass_button = builder->get_widget<Gtk::ToggleButton>("bypass_button");
  headerbar_icon1 = builder->get_widget<Gtk::Image>("headerbar_icon1");
  headerbar_icon2 = builder->get_widget<Gtk::Image>("headerbar_icon2");
  headerbar_info = builder->get_widget<Gtk::Label>("headerbar_info");

  // presets_menu_ui = PresetsMenuUi::add_to_popover(presets_menu, app);
  // soe_ui = StreamOutputEffectsUi::add_to_stack(stack, app->soe.get());
  // sie_ui = StreamInputEffectsUi::add_to_stack(stack, app->sie.get());
  // GeneralSettingsUi::add_to_stack(stack_menu_settings, app);
  // SpectrumSettingsUi::add_to_stack(stack_menu_settings, app);
  // PipeSettingsUi::add_to_stack(stack_menu_settings, app);
  // BlocklistSettingsUi::add_to_stack(stack_menu_settings);
  // pipe_info_ui = PipeInfoUi::add_to_stack(stack, app->pm.get());

  stack->connect_property_changed("visible-child",
                                  sigc::mem_fun(*this, &ApplicationUi::on_stack_visible_child_changed));

  // calibration

  calibration_button->signal_clicked().connect(sigc::mem_fun(*this, &ApplicationUi::on_calibration_button_clicked));

  // signals

  connections.emplace_back(app->pm->new_default_sink.connect([&](auto name) {
    if (stack->get_visible_child_name() == "stream_output") {
      update_headerbar_subtitle(0);
    }
  }));

  connections.emplace_back(app->pm->new_default_source.connect([&](auto name) {
    if (stack->get_visible_child_name() == "stream_input") {
      update_headerbar_subtitle(1);
    }
  }));

  connections.emplace_back(app->pm->sink_changed.connect([&](auto nd_info) {
    if (stack->get_visible_child_name() == "stream_output") {
      if (nd_info.id != app->soe->get_output_node_id()) {
        return;
      }

      update_headerbar_subtitle(0);
    }
  }));

  connections.emplace_back(app->pm->source_changed.connect([&](auto nd_info) {
    if (stack->get_visible_child_name() == "stream_input") {
      if (nd_info.id != app->sie->get_input_node_id()) {
        return;
      }

      update_headerbar_subtitle(1);
    }
  }));

  help_button->signal_clicked().connect([=]() { app->activate_action("help"); });

  // presets_menu_button->signal_clicked().connect(
  //     sigc::mem_fun(*presets_menu_ui, &PresetsMenuUi::on_presets_menu_button_clicked));

  presets_menu_button->get_label() = settings->get_string("last-used-output-preset");

  connections.emplace_back(settings->signal_changed("last-used-input-preset").connect([=](auto key) {
    presets_menu_button->get_label() = settings->get_string("last-used-input-preset");
  }));

  connections.emplace_back(settings->signal_changed("last-used-output-preset").connect([=](auto key) {
    presets_menu_button->get_label() = settings->get_string("last-used-output-preset");
  }));

  // headerbar info

  connections.emplace_back(app->soe->new_latency.connect([=](int latency) {
    soe_latency = latency;

    if (stack->get_visible_child_name() == "stream_output") {
      update_headerbar_subtitle(0);
    }
  }));

  if (app->soe->playing) {
    app->soe->get_latency();
  }

  connections.emplace_back(app->sie->new_latency.connect([=](int latency) {
    sie_latency = latency;

    if (stack->get_visible_child_name() == "stream_input") {
      update_headerbar_subtitle(1);
    }
  }));

  if (app->sie->playing) {
    app->sie->get_latency();
  }

  // updating headerbar info

  update_headerbar_subtitle(0);

  // binding glade widgets to gsettings keys

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

void ApplicationUi::update_headerbar_subtitle(const int& index) {
  std::ostringstream null_sink_rate;
  std::ostringstream pipeline_rate;
  std::ostringstream current_dev_rate;

  const float khz_factor = 0.001F;

  null_sink_rate.imbue(global_locale);
  null_sink_rate.precision(1);

  pipeline_rate.imbue(global_locale);
  pipeline_rate.precision(1);

  current_dev_rate.imbue(global_locale);
  current_dev_rate.precision(1);

  switch (index) {
    case 0: {  // soe

      subtitle_grid->show();

      headerbar_icon1->set_from_icon_name("emblem-music-symbolic");

      headerbar_icon2->set_from_icon_name("audio-speakers-symbolic");

      null_sink_rate << std::fixed << app->pm->pe_sink_node.rate * khz_factor << "kHz";

      pipeline_rate << std::fixed << app->soe->sampling_rate * khz_factor << "kHz";

      std::string current_device_format;

      for (const auto& node : app->pm->list_nodes) {
        if (node.id == app->soe->get_output_node_id()) {
          current_dev_rate << std::fixed << node.rate * khz_factor << "kHz";

          current_device_format = node.format;

          break;
        }
      }

      headerbar_info->set_text(" ⟶ " + app->pm->pe_sink_node.format + " " + null_sink_rate.str() + " ⟶ F32LE " +
                               pipeline_rate.str() + " ⟶ " + current_device_format + " " + current_dev_rate.str() +
                               " ⟶ " + std::to_string(soe_latency) + "ms ⟶ ");

      break;
    }
    case 1: {  // sie

      subtitle_grid->show();

      headerbar_icon1->set_from_icon_name("audio-input-microphone-symbolic");

      headerbar_icon2->set_from_icon_name("emblem-music-symbolic");

      null_sink_rate << std::fixed << app->pm->pe_source_node.rate * khz_factor << "kHz";

      pipeline_rate << std::fixed << app->sie->sampling_rate * khz_factor << "kHz";

      std::string current_device_format;

      for (const auto& node : app->pm->list_nodes) {
        if (node.id == app->sie->get_input_node_id()) {
          current_dev_rate << std::fixed << node.rate * khz_factor << "kHz";

          current_device_format = node.format;

          break;
        }
      }

      headerbar_info->set_text(" ⟶ " + current_device_format + " " + current_dev_rate.str() + " ⟶ F32LE " +
                               pipeline_rate.str() + " ⟶ " + app->pm->pe_source_node.format + " " +
                               null_sink_rate.str() + " ⟶ " + std::to_string(sie_latency) + "ms ⟶ ");

      break;
    }
    default:  // pulse info

      subtitle_grid->hide();

      break;
  }
}

void ApplicationUi::on_stack_visible_child_changed() {
  auto name = stack->get_visible_child_name();

  if (name == "stream_output") {
    update_headerbar_subtitle(0);

    presets_menu_label->set_text(settings->get_string("last-used-output-preset"));
  } else if (name == "stream_input") {
    update_headerbar_subtitle(1);

    presets_menu_label->set_text(settings->get_string("last-used-input-preset"));
  } else if (name == "pipe_info") {
    update_headerbar_subtitle(2);
  }
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
