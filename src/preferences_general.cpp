/*
 *  Copyright © 2017-2025 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "preferences_general.hpp"
#include <adwaita.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <gobject/gobject.h>
#include <gtk/gtk.h>
#include <filesystem>
#include <fstream>
#include "tags_app.hpp"
#include "tags_resources.hpp"
#include "ui_helpers.hpp"
#include "util.hpp"

namespace ui::preferences::general {

using namespace std::string_literals;

struct _PreferencesGeneral {
  AdwPreferencesPage parent_instance;

  GtkSwitch *enable_autostart, *process_all_inputs, *process_all_outputs, *theme_switch, *shutdown_on_window_close,
      *use_cubic_volumes, *inactivity_timer_enable, *autohide_popovers, *exclude_monitor_streams,
      *show_native_plugin_ui, *listen_to_mic;

  GtkSpinButton *inactivity_timeout, *meters_update_interval, *lv2ui_update_frequency;

  GSettings* settings;
};

// NOLINTNEXTLINE
G_DEFINE_TYPE(PreferencesGeneral, preferences_general, ADW_TYPE_PREFERENCES_PAGE)

#ifndef ENABLE_LIBPORTAL

auto on_enable_autostart(GtkSwitch* obj, gboolean state, gpointer user_data) -> gboolean {
  std::filesystem::path autostart_dir{g_get_user_config_dir() + "/autostart"s};

  if (!std::filesystem::is_directory(autostart_dir)) {
    std::filesystem::create_directories(autostart_dir);
  }

  std::filesystem::path autostart_file{g_get_user_config_dir() + "/autostart/easyeffects-service.desktop"s};

  if (state != 0) {
    if (!std::filesystem::exists(autostart_file)) {
      std::ofstream ofs{autostart_file};

      ofs << "[Desktop Entry]\n";
      ofs << "Name=Easy Effects\n";
      ofs << "Comment=Easy Effects Service\n";
      ofs << "Exec=easyeffects --gapplication-service\n";
      ofs << "Icon=com.github.wwmm.easyeffects\n";
      ofs << "StartupNotify=false\n";
      ofs << "Terminal=false\n";
      ofs << "Type=Application\n";

      ofs.close();

      util::debug("autostart file created");
    }
  } else {
    if (std::filesystem::exists(autostart_file)) {
      std::filesystem::remove(autostart_file);

      util::debug("autostart file removed");
    }
  }

  return 0;
}

#endif

void dispose(GObject* object) {
  auto* self = EE_PREFERENCES_GENERAL(object);

  g_object_unref(self->settings);

#ifdef ENABLE_LIBPORTAL
  g_settings_unbind(self->enable_autostart, "active");
  g_settings_unbind(self->shutdown_on_window_close, "active");
#endif

  util::debug("disposed");

  G_OBJECT_CLASS(preferences_general_parent_class)->dispose(object);
}

void preferences_general_class_init(PreferencesGeneralClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::preferences_general_ui);

  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, enable_autostart);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, process_all_inputs);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, process_all_outputs);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, theme_switch);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, autohide_popovers);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, shutdown_on_window_close);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, use_cubic_volumes);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, exclude_monitor_streams);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, listen_to_mic);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, inactivity_timer_enable);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, inactivity_timeout);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, meters_update_interval);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, lv2ui_update_frequency);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, show_native_plugin_ui);
}

void preferences_general_init(PreferencesGeneral* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->settings = g_settings_new(tags::app::id);

  prepare_spinbuttons<"s">(self->inactivity_timeout);
  prepare_spinbuttons<"ms">(self->meters_update_interval);
  prepare_spinbuttons<"Hz">(self->lv2ui_update_frequency);

  // initializing some widgets

  gsettings_bind_widgets<"process-all-inputs", "process-all-outputs", "use-dark-theme", "shutdown-on-window-close",
                        "use-cubic-volumes", "autohide-popovers", "exclude-monitor-streams", "listen-to-mic",
                        "inactivity-timer-enable", "inactivity-timeout", "meters-update-interval", "lv2ui-update-frequency",
                        "show-native-plugin-ui">(
      self->settings, self->process_all_inputs, self->process_all_outputs, self->theme_switch, self->shutdown_on_window_close,
      self->use_cubic_volumes, self->autohide_popovers, self->exclude_monitor_streams, self->listen_to_mic,
      self->inactivity_timer_enable, self->inactivity_timeout, self->meters_update_interval,
      self->lv2ui_update_frequency, self->show_native_plugin_ui);

#ifdef ENABLE_LIBPORTAL
  libportal::init(self->enable_autostart, self->shutdown_on_window_close);
#else

  gtk_switch_set_active(self->enable_autostart,
                        static_cast<gboolean>(std::filesystem::is_regular_file(
                            g_get_user_config_dir() + "/autostart/easyeffects-service.desktop"s)));

  g_signal_connect(self->enable_autostart, "state-set", G_CALLBACK(on_enable_autostart), nullptr);
#endif
}

auto create() -> PreferencesGeneral* {
  return static_cast<PreferencesGeneral*>(g_object_new(EE_TYPE_PREFERENCES_GENERAL, nullptr));
}

}  // namespace ui::preferences::general
