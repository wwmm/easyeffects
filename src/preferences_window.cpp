#include "preferences_window.hpp"

namespace ui::preferences_window {

using namespace std::string_literals;

auto constexpr log_tag = "preferences_window: ";

struct _PreferencesWindow {
  AdwPreferencesWindow parent_instance{};

  GSettings* settings = nullptr;
};

G_DEFINE_TYPE(PreferencesWindow, preferences_window, ADW_TYPE_PREFERENCES_WINDOW)

void preferences_window_class_init(PreferencesWindowClass* klass) {
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/preferences_window.ui");

  //   gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, stack);

  //   gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, output_scrolled_window);
  //   gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, output_listview);
  //   gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, output_name);
  //   gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, output_search);
  //   gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, last_used_output);

  //   gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, input_scrolled_window);
  //   gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, input_listview);
  //   gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, input_name);
  //   gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, input_search);
  //   gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, last_used_input);

  //   gtk_widget_class_bind_template_callback(widget_class, create_preset);
  //   gtk_widget_class_bind_template_callback(widget_class, import_output_preset);
  //   gtk_widget_class_bind_template_callback(widget_class, import_input_preset);
}

void preferences_window_init(PreferencesWindow* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->settings = g_settings_new("com.github.wwmm.easyeffects");

  //   gtk_label_set_text(self->last_used_output, g_settings_get_string(self->settings, "last-used-output-preset"));
  //   gtk_label_set_text(self->last_used_input, g_settings_get_string(self->settings, "last-used-input-preset"));

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