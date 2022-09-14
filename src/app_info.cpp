/*
 *  Copyright © 2017-2023 Wellington Wallace
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

#include "app_info.hpp"

namespace ui::app_info {

using namespace std::string_literals;

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  app::Application* application;

  NodeInfo info;

  gulong handler_id_enable, handler_id_volume, handler_id_mute, handler_id_blocklist;

  std::unordered_map<uint, bool>* enabled_app_list;
};

struct _AppInfo {
  GtkBox parent_instance;

  GtkImage* app_icon;

  GtkLabel *app_name, *media_name, *format, *rate, *channels, *latency, *state;

  GtkToggleButton* mute;

  GtkSpinButton* volume;

  GtkCheckButton *blocklist, *enable;

  GtkIconTheme* icon_theme;

  GSettings *settings, *app_settings;

  Data* data;
};

G_DEFINE_TYPE(AppInfo, app_info, GTK_TYPE_BOX)

auto node_state_to_char_pointer(const pw_node_state& state) -> const char* {
  switch (state) {
    case PW_NODE_STATE_RUNNING:
      return _("Running");
    case PW_NODE_STATE_SUSPENDED:
      return _("Suspended");
    case PW_NODE_STATE_IDLE:
      return _("Idle");
    case PW_NODE_STATE_CREATING:
      return _("Creating");
    case PW_NODE_STATE_ERROR:
      return _("Error");
    default:
      return _("Unknown");
  }
}

auto app_is_blocklisted(AppInfo* self, const std::string& name) -> bool {
  const auto list = util::gchar_array_to_vector(g_settings_get_strv(self->settings, "blocklist"));

  return std::ranges::find(list, name) != list.end();
}

auto get_app_icon_name(const NodeInfo& node_info) -> std::string {
  // map to handle cases where PipeWire does not set icon name string or app name equal to icon name.

  constexpr auto icon_map = std::to_array<std::pair<const char*, const char*>>(
      {{"chromium-browser", "chromium"}, {"firefox", "firefox"}, {"obs", "com.obsproject.Studio"}});

  std::string icon_name;

  if (!node_info.app_icon_name.empty()) {
    icon_name = node_info.app_icon_name;
  } else if (!node_info.media_icon_name.empty()) {
    icon_name = node_info.media_icon_name;
  } else if (!node_info.name.empty()) {
    icon_name = node_info.name;

    // get lowercase name so if it changes in the future, we have a chance to pick the same index

    std::transform(icon_name.begin(), icon_name.end(), icon_name.begin(),
                   [](unsigned char c) { return std::tolower(c); });
  }

  for (const auto& [key, value] : icon_map) {
    if (key == icon_name) {
      return value;
    }
  }

  return icon_name;
}

auto icon_available(AppInfo* self, const std::string& icon_name) -> bool {
  if (gtk_icon_theme_has_icon(self->icon_theme, icon_name.c_str()) != 0) {
    return true;
  }

  // The icon object can't lookup icons in pixmaps directories, so we check their existence there also.

  constexpr auto pixmaps_dirs = std::to_array({"/usr/share/pixmaps", "/usr/local/share/pixmaps"});

  for (const auto& dir : pixmaps_dirs) {
    try {
      for (std::filesystem::directory_iterator it{dir}; it != std::filesystem::directory_iterator{}; ++it) {
        if (std::filesystem::is_regular_file(it->status())) {
          if (it->path().stem().string() == icon_name) {
            util::debug(icon_name + " icon name not included in the icon theme, but found in " + dir);

            return true;
          }
        }
      }
    } catch (...) {
      util::debug("cannot lookup application icon " + icon_name + " in " + dir);
    }
  }

  return false;
}

void connect_stream(AppInfo* self, const uint& id, const std::string& media_class) {
  if (media_class == tags::pipewire::media_class::output_stream) {
    self->data->application->pm->connect_stream_output(id);
  } else if (media_class == tags::pipewire::media_class::input_stream) {
    self->data->application->pm->connect_stream_input(id);
  }
}

void disconnect_stream(AppInfo* self, const uint& id, const std::string& media_class) {
  if (media_class == tags::pipewire::media_class::output_stream ||
      media_class == tags::pipewire::media_class::input_stream) {
    self->data->application->pm->disconnect_stream(id);
  }
}

void on_enable(GtkCheckButton* btn, AppInfo* self) {
  auto is_enabled = gtk_check_button_get_active(btn) != 0;

  auto is_blocklisted = app_is_blocklisted(self, self->data->info.name);

  if (!is_blocklisted) {
    (is_enabled) ? connect_stream(self, self->data->info.id, self->data->info.media_class)
                 : disconnect_stream(self, self->data->info.id, self->data->info.media_class);

    self->data->enabled_app_list->insert_or_assign(self->data->info.id, is_enabled);
  }
}

void on_volume_changed(GtkSpinButton* sbtn, AppInfo* self) {
  auto vol = static_cast<float>(gtk_spin_button_get_value(sbtn)) / 100.0F;

  if (g_settings_get_boolean(self->app_settings, "use-cubic-volumes") != 0) {
    vol = vol * vol * vol;
  }

  if (self->data->info.proxy != nullptr) {
    PipeManager::set_node_volume(self->data->info.proxy, self->data->info.n_volume_channels, vol);
  }
}

void on_mute(GtkToggleButton* btn, AppInfo* self) {
  const auto state = gtk_toggle_button_get_active(btn);

  if (state != 0) {
    gtk_button_set_icon_name(GTK_BUTTON(btn), "audio-volume-muted-symbolic");
  } else {
    gtk_button_set_icon_name(GTK_BUTTON(btn), "audio-volume-high-symbolic");
  }

  if (self->data->info.proxy != nullptr) {
    PipeManager::set_node_mute(self->data->info.proxy, state != 0);
  }
}

void on_blocklist(GtkCheckButton* btn, AppInfo* self) {
  const auto is_blocklisted = gtk_check_button_get_active(btn);

  std::string app_tag = self->data->info.application_id;

  if (app_tag.empty()) {
    app_tag = self->data->info.name;
  }

  if (is_blocklisted != 0) {
    self->data->enabled_app_list->insert_or_assign(self->data->info.id, gtk_check_button_get_active(self->enable));

    util::add_new_blocklist_entry(self->settings, app_tag);
  } else {
    util::remove_blocklist_entry(self->settings, app_tag);
  }
}

void update(AppInfo* self, const NodeInfo node_info) {
  if (node_info.state == PW_NODE_STATE_CREATING) {
    // PW_NODE_STATE_CREATING is useless and does not give any meaningful info, therefore skip it
    return;
  }

  self->data->info = node_info;

  gtk_label_set_text(self->app_name, node_info.name.c_str());
  gtk_label_set_text(self->media_name, node_info.media_name.c_str());
  gtk_label_set_text(self->format, node_info.format.c_str());
  gtk_label_set_text(
      self->rate,
      fmt::format(ui::get_user_locale(), "{0:.1Lf} kHz", static_cast<float>(node_info.rate) / 1000.0F).c_str());
  gtk_label_set_text(self->channels, fmt::format("{0:d} {1}", node_info.n_volume_channels, _("channels")).c_str());
  gtk_label_set_text(self->latency, fmt::format("{0:.0f} ms", 1000.0F * node_info.latency).c_str());
  gtk_label_set_text(self->state, node_state_to_char_pointer(node_info.state));

  // updating the enable toggle button

  g_signal_handler_block(self->enable, self->data->handler_id_enable);

  const auto is_enabled = self->data->application->pm->stream_is_connected(node_info.id, node_info.media_class);
  const auto is_blocklisted = app_is_blocklisted(self, node_info.name);

  gtk_widget_set_sensitive(GTK_WIDGET(self->enable), static_cast<gboolean>(is_enabled || !is_blocklisted));
  gtk_check_button_set_active(self->enable, static_cast<gboolean>(is_enabled));

  g_signal_handler_unblock(self->enable, self->data->handler_id_enable);

  // updating the volume

  g_signal_handler_block(self->volume, self->data->handler_id_volume);

  if (g_settings_get_boolean(self->app_settings, "use-cubic-volumes") != 0) {
    gtk_spin_button_set_value(self->volume, 100.0 * std::cbrt(static_cast<double>(node_info.volume)));
  } else {
    gtk_spin_button_set_value(self->volume, 100.0 * static_cast<double>(node_info.volume));
  }

  g_signal_handler_unblock(self->volume, self->data->handler_id_volume);

  // updating the mute button state

  g_signal_handler_block(self->mute, self->data->handler_id_mute);

  if (node_info.mute) {
    gtk_button_set_icon_name(GTK_BUTTON(self->mute), "audio-volume-muted-symbolic");
  } else {
    gtk_button_set_icon_name(GTK_BUTTON(self->mute), "audio-volume-high-symbolic");
  }

  gtk_toggle_button_set_active(self->mute, static_cast<gboolean>(node_info.mute));

  g_signal_handler_unblock(self->mute, self->data->handler_id_mute);

  // set the icon name

  if (const auto default_app_icon = "ee-applications-multimedia-symbolic"s; self->icon_theme == nullptr) {
    gtk_image_set_from_icon_name(self->app_icon, default_app_icon.c_str());
  } else if (const auto icon_name = get_app_icon_name(node_info); icon_name.empty()) {
    gtk_image_set_from_icon_name(self->app_icon, default_app_icon.c_str());
  } else if (!icon_available(self, icon_name)) {
    gtk_image_set_from_icon_name(self->app_icon, default_app_icon.c_str());
  } else {
    gtk_image_set_from_icon_name(self->app_icon, icon_name.c_str());
  }

  // updating the blocklist button state

  g_signal_handler_block(self->blocklist, self->data->handler_id_blocklist);

  gtk_check_button_set_active(self->blocklist, static_cast<gboolean>(is_blocklisted));

  g_signal_handler_unblock(self->blocklist, self->data->handler_id_blocklist);

  // save app "enabled state" only the first time when it is not present in the enabled_app_list map

  if (self->data->enabled_app_list->find(node_info.id) == self->data->enabled_app_list->end()) {
    self->data->enabled_app_list->insert({node_info.id, is_enabled});
  }
}

void setup(AppInfo* self,
           app::Application* application,
           GSettings* settings,
           GtkIconTheme* icon_theme,
           std::unordered_map<uint, bool>& enabled_app_list) {
  self->data->application = application;
  self->settings = settings;
  self->icon_theme = icon_theme;
  self->data->enabled_app_list = &enabled_app_list;
}

void dispose(GObject* object) {
  auto* self = EE_APP_INFO(object);

  g_object_unref(self->app_settings);

  util::debug(self->data->info.name + " disposed");

  G_OBJECT_CLASS(app_info_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_APP_INFO(object);

  util::debug(self->data->info.name + " finalized");

  delete self->data;

  G_OBJECT_CLASS(app_info_parent_class)->finalize(object);
}

void app_info_class_init(AppInfoClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::app_info_ui);

  gtk_widget_class_bind_template_child(widget_class, AppInfo, enable);
  gtk_widget_class_bind_template_child(widget_class, AppInfo, app_icon);
  gtk_widget_class_bind_template_child(widget_class, AppInfo, app_name);
  gtk_widget_class_bind_template_child(widget_class, AppInfo, media_name);
  gtk_widget_class_bind_template_child(widget_class, AppInfo, format);
  gtk_widget_class_bind_template_child(widget_class, AppInfo, rate);
  gtk_widget_class_bind_template_child(widget_class, AppInfo, channels);
  gtk_widget_class_bind_template_child(widget_class, AppInfo, latency);
  gtk_widget_class_bind_template_child(widget_class, AppInfo, state);
  gtk_widget_class_bind_template_child(widget_class, AppInfo, volume);
  gtk_widget_class_bind_template_child(widget_class, AppInfo, mute);
  gtk_widget_class_bind_template_child(widget_class, AppInfo, blocklist);
}

void app_info_init(AppInfo* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  self->app_settings = g_settings_new(tags::app::id);

  prepare_spinbuttons<"%">(self->volume);

  self->data->handler_id_enable = g_signal_connect(self->enable, "toggled", G_CALLBACK(on_enable), self);
  self->data->handler_id_volume = g_signal_connect(self->volume, "value-changed", G_CALLBACK(on_volume_changed), self);
  self->data->handler_id_mute = g_signal_connect(self->mute, "toggled", G_CALLBACK(on_mute), self);
  self->data->handler_id_blocklist = g_signal_connect(self->blocklist, "toggled", G_CALLBACK(on_blocklist), self);
}

auto create() -> AppInfo* {
  return static_cast<AppInfo*>(g_object_new(EE_TYPE_APP_INFO, nullptr));
}

}  // namespace ui::app_info
