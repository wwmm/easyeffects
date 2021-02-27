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

    default:
      return g_variant_new_string("None");
  }
}

}  // namespace

GeneralSettingsUi::GeneralSettingsUi(BaseObjectType* cobject,
                                     const Glib::RefPtr<Gtk::Builder>& builder,
                                     Application* application)
    : Gtk::Box(cobject), settings(Gio::Settings::create("com.github.wwmm.pulseeffects")), app(application) {
  // loading builder widgets

  theme_switch = builder->get_widget<Gtk::Switch>("theme_switch");
  process_all_inputs = builder->get_widget<Gtk::Switch>("process_all_inputs");
  process_all_outputs = builder->get_widget<Gtk::Switch>("process_all_outputs");
  enable_autostart = builder->get_widget<Gtk::Switch>("enable_autostart");
  reset_settings = builder->get_widget<Gtk::Button>("reset_settings");
  about_button = builder->get_widget<Gtk::Button>("about_button");
  spin_button_priority = builder->get_widget<Gtk::SpinButton>("spin_button_priority");
  spin_button_niceness = builder->get_widget<Gtk::SpinButton>("spin_button_niceness");
  priority_type = builder->get_widget<Gtk::ComboBoxText>("priority_type");

  realtime_priority = builder->get_object<Gtk::Adjustment>("realtime_priority");
  niceness = builder->get_object<Gtk::Adjustment>("niceness");
  audio_activity_timeout = builder->get_object<Gtk::Adjustment>("audio_activity_timeout");

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

  settings->bind("use-dark-theme", theme_switch, "active");
  settings->bind("process-all-inputs", process_all_inputs, "active");
  settings->bind("process-all-outputs", process_all_outputs, "active");
  settings->bind("realtime-priority", realtime_priority.get(), "value");
  settings->bind("niceness", niceness.get(), "value");
  settings->bind("audio-activity-timeout", audio_activity_timeout.get(), "value");

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
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/general_settings.ui");

  auto* ui = Gtk::Builder::get_widget_derived<GeneralSettingsUi>(builder, "top_box", app);

  stack->add(*ui, "general_spectrum", _("General"));
}

void GeneralSettingsUi::init_autostart_switch() {
  auto path = Glib::get_user_config_dir() + "/autostart/pulseeffects-service.desktop";

  if (std::filesystem::is_regular_file(path)) {
    enable_autostart->set_active(true);
  } else {
    enable_autostart->set_active(false);
  }
}

auto GeneralSettingsUi::on_enable_autostart(bool state) -> bool {
  std::filesystem::path autostart_dir{Glib::get_user_config_dir() + "/autostart"};

  if (!std::filesystem::is_directory(autostart_dir)) {
    std::filesystem::create_directories(autostart_dir);
  }

  std::filesystem::path autostart_file{Glib::get_user_config_dir() + "/autostart/pulseeffects-service.desktop"};

  if (state) {
    if (!std::filesystem::exists(autostart_file)) {
      std::ofstream ofs{autostart_file};

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
    if (std::filesystem::exists(autostart_file)) {
      std::filesystem::remove(autostart_file);

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
      spin_button_niceness->set_sensitive(true);
      spin_button_priority->set_sensitive(false);
      break;
    }
    case 1: {
      spin_button_niceness->set_sensitive(false);
      spin_button_priority->set_sensitive(true);
      break;
    }
    case 2: {
      spin_button_niceness->set_sensitive(false);
      spin_button_priority->set_sensitive(false);
      break;
    }
    default:
      break;
  }
}
