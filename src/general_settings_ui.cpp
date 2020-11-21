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

#include "general_settings_ui.hpp"
#include <giomm/file.h>
#include <glibmm.h>
#include <boost/filesystem.hpp>
#include "util.hpp"

namespace {

auto priority_type_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "Niceness") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Real Time") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "None") == 0) {
    g_value_set_int(value, 2);
  }

  return 1;
}

auto int_to_priority_type_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("Niceness");

    case 1:
      return g_variant_new_string("Real Time");

    case 2:
      return g_variant_new_string("None");

    default:
      return g_variant_new_string("None");
  }
}

}  // namespace

GeneralSettingsUi::GeneralSettingsUi(BaseObjectType* cobject,
                                     const Glib::RefPtr<Gtk::Builder>& builder,
                                     Application* application)
    : Gtk::Grid(cobject), settings(Gio::Settings::create("com.github.wwmm.pulseeffects")), app(application) {
  // loading glade widgets

  builder->get_widget("theme_switch", theme_switch);
  builder->get_widget("enable_autostart", enable_autostart);
  builder->get_widget("enable_all_sinkinputs", enable_all_sinkinputs);
  builder->get_widget("enable_all_sourceoutputs", enable_all_sourceoutputs);
  builder->get_widget("reset_settings", reset_settings);
  builder->get_widget("about_button", about_button);
  builder->get_widget("realtime_priority", realtime_priority_control);
  builder->get_widget("niceness", niceness_control);
  builder->get_widget("priority_type", priority_type);

  get_object(builder, "adjustment_priority", adjustment_priority);
  get_object(builder, "adjustment_niceness", adjustment_niceness);
  get_object(builder, "adjustment_audio_activity_timeout", adjustment_audio_activity_timeout);

  // signals connection

  enable_autostart->signal_state_set().connect(sigc::mem_fun(*this, &GeneralSettingsUi::on_enable_autostart), false);

  reset_settings->signal_clicked().connect(sigc::mem_fun(*this, &GeneralSettingsUi::on_reset_settings));

  about_button->signal_clicked().connect([=]() { app->activate_action("about"); });

  connections.emplace_back(settings->signal_changed("priority-type").connect([&](auto key) {
    set_priority_controls_visibility();

    app->sie->set_null_pipeline();
    app->soe->set_null_pipeline();

    app->sie->update_pipeline_state();
    app->soe->update_pipeline_state();
  }));

  connections.emplace_back(settings->signal_changed("realtime-priority").connect([&](auto key) {
    app->sie->set_null_pipeline();
    app->soe->set_null_pipeline();

    app->sie->update_pipeline_state();
    app->soe->update_pipeline_state();
  }));

  connections.emplace_back(settings->signal_changed("niceness").connect([&](auto key) {
    app->sie->set_null_pipeline();
    app->soe->set_null_pipeline();

    app->sie->update_pipeline_state();
    app->soe->update_pipeline_state();
  }));

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("use-dark-theme", theme_switch, "active", flag);
  settings->bind("enable-all-sinkinputs", enable_all_sinkinputs, "active", flag);
  settings->bind("enable-all-sourceoutputs", enable_all_sourceoutputs, "active", flag);
  settings->bind("realtime-priority", adjustment_priority.get(), "value", flag);
  settings->bind("niceness", adjustment_niceness.get(), "value", flag);
  settings->bind("audio-activity-timeout", adjustment_audio_activity_timeout.get(), "value", flag);

  g_settings_bind_with_mapping(settings->gobj(), "priority-type", priority_type->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, priority_type_enum_to_int, int_to_priority_type_enum, nullptr,
                               nullptr);

  init_autostart_switch();
  set_priority_controls_visibility();
}

GeneralSettingsUi::~GeneralSettingsUi() {
  for (auto& c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

void GeneralSettingsUi::add_to_stack(Gtk::Stack* stack, Application* app) {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/general_settings.glade");

  GeneralSettingsUi* ui;

  builder->get_widget_derived("widgets_grid", ui, app);

  stack->add(*ui, "general_spectrum", _("General"));
}

void GeneralSettingsUi::init_autostart_switch() {
  auto path = Glib::get_user_config_dir() + "/autostart/pulseeffects-service.desktop";

  try {
    auto file = Gio::File::create_for_path(path);

    if (file->query_exists()) {
      enable_autostart->set_active(true);
    } else {
      enable_autostart->set_active(false);
    }
  } catch (const Glib::Exception& ex) {
    util::warning(log_tag + ex.what());
  }
}

auto GeneralSettingsUi::on_enable_autostart(bool state) -> bool {
  boost::filesystem::path autostart_dir{Glib::get_user_config_dir() + "/autostart"};

  if (!boost::filesystem::is_directory(autostart_dir)) {
    boost::filesystem::create_directories(autostart_dir);
  }

  boost::filesystem::path autostart_file{Glib::get_user_config_dir() + "/autostart/pulseeffects-service.desktop"};

  if (state) {
    if (!boost::filesystem::exists(autostart_file)) {
      boost::filesystem::ofstream ofs{autostart_file};

      ofs << "[Desktop Entry]\n";
      ofs << "Name=PulseEffects\n";
      ofs << "Comment=PulseEffects Service\n";
      ofs << "Exec=pulseeffects --gapplication-service\n";
      ofs << "Icon=pulseeffects\n";
      ofs << "StartupNotify=false\n";
      ofs << "Terminal=false\n";
      ofs << "Type=Application\n";

      ofs.close();

      util::debug(log_tag + "autostart file created");
    }
  } else {
    if (boost::filesystem::exists(autostart_file)) {
      boost::filesystem::remove(autostart_file);

      util::debug(log_tag + "autostart file removed");
    }
  }

  return false;
}

void GeneralSettingsUi::on_reset_settings() {
  settings->reset("");
}

void GeneralSettingsUi::set_priority_controls_visibility() {
  auto priority_type = settings->get_enum("priority-type");

  switch (priority_type) {
    case 0: {
      niceness_control->set_sensitive(true);
      realtime_priority_control->set_sensitive(false);
      break;
    }
    case 1: {
      niceness_control->set_sensitive(false);
      realtime_priority_control->set_sensitive(true);
      break;
    }
    case 2: {
      niceness_control->set_sensitive(false);
      realtime_priority_control->set_sensitive(false);
      break;
    }
    default:
      break;
  }
}
