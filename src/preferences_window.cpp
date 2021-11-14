#include "preferences_window.hpp"

namespace ui::preferences_window {

using namespace std::string_literals;

auto constexpr log_tag = "preferences_window: ";

struct _PreferencesWindow {
  AdwPreferencesWindow parent_instance{};

  GtkSwitch *enable_autostart = nullptr, *process_all_inputs = nullptr, *process_all_outputs = nullptr,
            *theme_switch = nullptr, *shutdown_on_window_close = nullptr, *use_cubic_volumes = nullptr;

  GSettings* settings = nullptr;
};

G_DEFINE_TYPE(PreferencesWindow, preferences_window, ADW_TYPE_PREFERENCES_WINDOW)

void preferences_window_class_init(PreferencesWindowClass* klass) {
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/preferences_window.ui");

  gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, enable_autostart);
  gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, process_all_inputs);
  gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, process_all_outputs);
  gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, theme_switch);
  gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, shutdown_on_window_close);
  gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, use_cubic_volumes);

  //   gtk_widget_class_bind_template_callback(widget_class, create_preset);
  //   gtk_widget_class_bind_template_callback(widget_class, import_output_preset);
  //   gtk_widget_class_bind_template_callback(widget_class, import_input_preset);
}

void preferences_window_init(PreferencesWindow* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->settings = g_settings_new("com.github.wwmm.easyeffects");

  g_settings_bind(self->settings, "use-dark-theme", self->theme_switch, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "process-all-inputs", self->process_all_inputs, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "process-all-outputs", self->process_all_outputs, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "shutdown-on-window-close", self->shutdown_on_window_close, "active",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "use-cubic-volumes", self->use_cubic_volumes, "active", G_SETTINGS_BIND_DEFAULT);

  //   g_signal_connect(self->settings, "changed::last-used-output-preset",
  //                    G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
  //                      auto self = static_cast<PreferencesWindow*>(user_data);

  //                      gtk_label_set_text(self->last_used_output, g_settings_get_string(settings, key));
  //                    }),
  //                    self);

  //   g_signal_connect(self->settings, "changed::last-used-input-preset",
  //                    G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
  //                      auto self = static_cast<PreferencesWindow*>(user_data);

  //                      gtk_label_set_text(self->last_used_input, g_settings_get_string(settings, key));
  //                    }),
  //                    self);
}

auto create() -> PreferencesWindow* {
  return static_cast<PreferencesWindow*>(g_object_new(EE_TYPE_PREFERENCES_WINDOW, nullptr));
}

}  // namespace ui::preferences_window