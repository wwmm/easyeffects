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

#include "application_ui.hpp"
#include "config.h"

namespace ui::application_window {

using namespace std::string_literals;

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  int width = 0, height = 0;

  bool maximized = false, fullscreen = false;

  GApplication* gapp = nullptr;

  GtkIconTheme* icon_theme = nullptr;

  std::vector<sigc::connection> connections;
};

struct _ApplicationWindow {
  AdwWindow parent_instance;

  AdwViewStack* stack;

  GtkMenuButton* presets_menu_button;

  GtkToggleButton* bypass_button;

  ui::presets_menu::PresetsMenu* presetsMenu;
  ui::effects_box::EffectsBox* soe_ui;
  ui::effects_box::EffectsBox* sie_ui;
  ui::pipe_manager_box::PipeManagerBox* pm_box;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(ApplicationWindow, application_window, ADW_TYPE_APPLICATION_WINDOW)

void init_theme_color(ApplicationWindow* self) {
  if (g_settings_get_boolean(self->settings, "use-dark-theme") == 0) {
    adw_style_manager_set_color_scheme(adw_style_manager_get_default(), ADW_COLOR_SCHEME_PREFER_LIGHT);
  } else {
    adw_style_manager_set_color_scheme(adw_style_manager_get_default(), ADW_COLOR_SCHEME_PREFER_DARK);
  }
}

auto setup_icon_theme() -> GtkIconTheme* {
  auto* icon_theme = gtk_icon_theme_get_for_display(gdk_display_get_default());

  if (icon_theme == nullptr) {
    util::warning("can't retrieve the icon theme in use on the system. App icons won't be shown.");

    return nullptr;
  }

  auto* name = gtk_icon_theme_get_theme_name(icon_theme);

  if (name == nullptr) {
    util::debug("Icon Theme detected, but the name is empty");
  } else {
    util::debug("Icon Theme "s + name + " detected");

    g_free(name);
  }

  gtk_icon_theme_add_resource_path(icon_theme, tags::resources::icons);

  return icon_theme;
}

void apply_css_style() {
  auto* provider = gtk_css_provider_new();

  gtk_css_provider_load_from_resource(provider, tags::resources::css);

  gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

void constructed(GObject* object) {
  auto* self = EE_APP_WINDOW(object);

  self->data->maximized = g_settings_get_boolean(self->settings, "window-maximized") != 0;
  self->data->fullscreen = g_settings_get_boolean(self->settings, "window-fullscreen") != 0;
  self->data->width = g_settings_get_int(self->settings, "window-width");
  self->data->height = g_settings_get_int(self->settings, "window-height");

  gtk_window_set_default_size(GTK_WINDOW(self), self->data->width, self->data->height);

  gtk_window_set_default_icon_name(IS_DEVEL_BUILD ? std::string(tags::app::id).append(".Devel").c_str()
                                                  : tags::app::id);

  if (self->data->maximized) {
    gtk_window_maximize(GTK_WINDOW(self));
  }

  if (self->data->fullscreen) {
    gtk_window_fullscreen(GTK_WINDOW(self));
  }

  G_OBJECT_CLASS(application_window_parent_class)->constructed(object);
}

void size_allocate(GtkWidget* widget, int width, int height, int baseline) {
  auto* self = EE_APP_WINDOW(widget);

  GTK_WIDGET_CLASS(application_window_parent_class)->size_allocate(widget, width, height, baseline);

  if (!self->data->maximized && !self->data->fullscreen) {
    gtk_window_get_default_size(GTK_WINDOW(self), &self->data->width, &self->data->height);
  }
}

void surface_state_changed(GtkWidget* widget) {
  auto* self = EE_APP_WINDOW(widget);

  GdkToplevelState new_state = GDK_TOPLEVEL_STATE_MAXIMIZED;

  new_state = gdk_toplevel_get_state(GDK_TOPLEVEL(gtk_native_get_surface(GTK_NATIVE(widget))));

  self->data->maximized = (new_state & GDK_TOPLEVEL_STATE_MAXIMIZED) != 0;
  self->data->fullscreen = (new_state & GDK_TOPLEVEL_STATE_FULLSCREEN) != 0;
}

void realize(GtkWidget* widget) {
  GTK_WIDGET_CLASS(application_window_parent_class)->realize(widget);

  g_signal_connect_swapped(gtk_native_get_surface(GTK_NATIVE(widget)), "notify::state",
                           G_CALLBACK(surface_state_changed), widget);

  auto* self = EE_APP_WINDOW(widget);

  /*
    Getting the gapp pointer here because it is not defined when "init" is called
  */

  self->data->gapp = G_APPLICATION(gtk_window_get_application(GTK_WINDOW(widget)));

  ui::presets_menu::setup(self->presetsMenu, app::EE_APP(self->data->gapp));
  ui::effects_box::setup(self->soe_ui, app::EE_APP(self->data->gapp), PipelineType::output, self->data->icon_theme);
  ui::effects_box::setup(self->sie_ui, app::EE_APP(self->data->gapp), PipelineType::input, self->data->icon_theme);
  ui::pipe_manager_box::setup(self->pm_box, app::EE_APP(self->data->gapp));

  self->data->connections.push_back(
      app::EE_APP(self->data->gapp)
          ->presets_manager->preset_load_error.connect([=](const std::string& title, const std::string& descr) {
            ui::show_simple_message_dialog(widget, title, descr);
          }));
}

void unrealize(GtkWidget* widget) {
  auto* self = EE_APP_WINDOW(widget);

  for (auto& c : self->data->connections) {
    c.disconnect();
  }

  self->data->connections.clear();

  g_signal_handlers_disconnect_by_func(gtk_native_get_surface(GTK_NATIVE((widget))),
                                       reinterpret_cast<gpointer>(surface_state_changed), widget);

  GTK_WIDGET_CLASS(application_window_parent_class)->unrealize(widget);
}

void dispose(GObject* object) {
  auto* self = EE_APP_WINDOW(object);

  g_settings_set_int(self->settings, "window-width", self->data->width);
  g_settings_set_int(self->settings, "window-height", self->data->height);
  g_settings_set_boolean(self->settings, "window-maximized", static_cast<gboolean>(self->data->maximized));
  g_settings_set_boolean(self->settings, "window-fullscreen", static_cast<gboolean>(self->data->fullscreen));

  if (g_settings_get_boolean(self->settings, "shutdown-on-window-close") != 0 &&
      (g_application_get_flags(self->data->gapp) & G_APPLICATION_IS_SERVICE) != 0) {
    g_application_release(self->data->gapp);
  }

  g_object_unref(self->settings);

  util::debug("disposed");

  G_OBJECT_CLASS(application_window_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_APP_WINDOW(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(application_window_parent_class)->finalize(object);
}

void application_window_class_init(ApplicationWindowClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->constructed = constructed;
  object_class->dispose = dispose;
  object_class->finalize = finalize;

  widget_class->size_allocate = size_allocate;
  widget_class->realize = realize;
  widget_class->unrealize = unrealize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::application_window_ui);

  gtk_widget_class_bind_template_child(widget_class, ApplicationWindow, stack);
  gtk_widget_class_bind_template_child(widget_class, ApplicationWindow, presets_menu_button);
  gtk_widget_class_bind_template_child(widget_class, ApplicationWindow, bypass_button);
}

void application_window_init(ApplicationWindow* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  self->data->width = -1;
  self->data->height = -1;
  self->data->maximized = false;
  self->data->fullscreen = false;

  self->settings = g_settings_new(tags::app::id);

  init_theme_color(self);

  apply_css_style();

  if (IS_DEVEL_BUILD) {
    GtkStyleContext* style_context = nullptr;

    style_context = gtk_widget_get_style_context(GTK_WIDGET(self));

    gtk_style_context_add_class(style_context, "devel");
  }

  self->data->icon_theme = setup_icon_theme();

  /*
    We save the user locale here because we have to wait for the changes GTK is going to make to the global locale.
    It seems that the init method of the main window widget is a good place for this.
  */

  ui::save_user_locale();

  self->presetsMenu = ui::presets_menu::create();
  self->soe_ui = ui::effects_box::create();
  self->sie_ui = ui::effects_box::create();
  self->pm_box = ui::pipe_manager_box::create();

  auto* soe_ui_page = adw_view_stack_add_titled(self->stack, GTK_WIDGET(self->soe_ui), "stream_output", _("_Output"));
  auto* sie_ui_page = adw_view_stack_add_titled(self->stack, GTK_WIDGET(self->sie_ui), "stream_input", _("_Input"));
  auto* pm_box_page = adw_view_stack_add_titled(self->stack, GTK_WIDGET(self->pm_box), "page_pipewire", _("_PipeWire"));

  adw_view_stack_page_set_use_underline(soe_ui_page, true);
  adw_view_stack_page_set_use_underline(sie_ui_page, true);
  adw_view_stack_page_set_use_underline(pm_box_page, true);

  adw_view_stack_page_set_icon_name(soe_ui_page, "audio-speakers-symbolic");
  adw_view_stack_page_set_icon_name(sie_ui_page, "audio-input-microphone-symbolic");
  adw_view_stack_page_set_icon_name(pm_box_page, "network-server-symbolic");

  gtk_menu_button_set_popover(self->presets_menu_button, GTK_WIDGET(self->presetsMenu));

  g_settings_bind(self->settings, "bypass", self->bypass_button, "active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "autohide-popovers", self->presetsMenu, "autohide", G_SETTINGS_BIND_DEFAULT);

  g_signal_connect(self->settings, "changed::use-dark-theme",
                   G_CALLBACK(+[](GSettings* settings, char* key, ApplicationWindow* self) { init_theme_color(self); }),
                   self);
}

auto create(GApplication* gapp) -> ApplicationWindow* {
  return static_cast<ApplicationWindow*>(g_object_new(EE_TYPE_APPLICATION_WINDOW, "application", gapp, nullptr));
}

}  // namespace ui::application_window
