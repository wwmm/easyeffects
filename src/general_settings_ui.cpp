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

#include "general_settings_ui.hpp"

GeneralSettingsUi::GeneralSettingsUi(BaseObjectType* cobject,
                                     const Glib::RefPtr<Gtk::Builder>& builder,
                                     Application* application)
    : Gtk::Box(cobject), settings(Gio::Settings::create("com.github.wwmm.easyeffects")), app(application) {
  // loading builder widgets

  theme_switch = builder->get_widget<Gtk::Switch>("theme_switch");
  process_all_inputs = builder->get_widget<Gtk::Switch>("process_all_inputs");
  process_all_outputs = builder->get_widget<Gtk::Switch>("process_all_outputs");
  enable_autostart = builder->get_widget<Gtk::Switch>("enable_autostart");
  shutdown_on_window_close = builder->get_widget<Gtk::Switch>("shutdown_on_window_close");

  reset_settings = builder->get_widget<Gtk::Button>("reset_settings");
  about_button = builder->get_widget<Gtk::Button>("about_button");

  // signals connection

  enable_autostart->signal_state_set().connect(sigc::mem_fun(*this, &GeneralSettingsUi::on_enable_autostart), false);

  reset_settings->signal_clicked().connect(sigc::mem_fun(*this, &GeneralSettingsUi::on_reset_settings));

  about_button->signal_clicked().connect([=, this]() { app->activate_action("about"); });

  settings->bind("use-dark-theme", theme_switch, "active");
  settings->bind("process-all-inputs", process_all_inputs, "active");
  settings->bind("process-all-outputs", process_all_outputs, "active");
  settings->bind("shutdown-on-window-close", shutdown_on_window_close, "active");

  init_autostart_switch();
}

GeneralSettingsUi::~GeneralSettingsUi() {
  for (auto& c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

void GeneralSettingsUi::add_to_stack(Gtk::Stack* stack, Application* app) {
  const auto& builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/general_settings.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<GeneralSettingsUi>(builder, "top_box", app);

  stack->add(*ui, "general_spectrum", _("General"));
}

void GeneralSettingsUi::init_autostart_switch() {
  const auto& path = Glib::get_user_config_dir() + "/autostart/easyeffects-service.desktop";

  enable_autostart->set_active(std::filesystem::is_regular_file(path) ? true : false);
}

auto GeneralSettingsUi::on_enable_autostart(bool state) -> bool {
  std::filesystem::path autostart_dir{Glib::get_user_config_dir() + "/autostart"};

  if (!std::filesystem::is_directory(autostart_dir)) {
    std::filesystem::create_directories(autostart_dir);
  }

  std::filesystem::path autostart_file{Glib::get_user_config_dir() + "/autostart/easyeffects-service.desktop"};

  if (state) {
    if (!std::filesystem::exists(autostart_file)) {
      std::ofstream ofs{autostart_file};

      ofs << "[Desktop Entry]\n";
      ofs << "Name=EasyEffects\n";
      ofs << "Comment=EasyEffects Service\n";
      ofs << "Exec=easyeffects --gapplication-service\n";
      ofs << "Icon=easyeffects\n";
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
