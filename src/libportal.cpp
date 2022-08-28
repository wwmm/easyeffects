#include "libportal.hpp"
#include "preferences_general.hpp"

namespace {

bool reset_shutdown = false, reset_autostart = false;

}

namespace libportal {

XdpPortal* portal = nullptr;

void on_request_background_called(GObject* source, GAsyncResult* result, gpointer widgets_ptr) {
  auto* widgets = static_cast<Widgets*>(widgets_ptr);

  if (widgets->enable_autostart == nullptr || widgets->shutdown_on_window_close == nullptr) {
    util::error("Programming error. When using libportal its pointers to our settings widgets must be initialized.");
  }

  g_autoptr(GError) error = nullptr;

  // libportal check if portal request worked

  if (xdp_portal_request_background_finish(portal, result, &error) == 0) {
    util::warning(std::string("portal: a background request failed: ") + ((error) ? error->message : "unknown reason"));
    util::warning(std::string("portal: background portal access has likely been denied"));
    util::warning(
        "portal: to let EasyEffects ask for the portal again, run flatpak permission-reset "
        "com.github.wwmm.easyeffects");

    // this seems wrong but also works?
    auto* window_levels = gtk_window_get_toplevels();

    GtkWidget* dialog = gtk_message_dialog_new(
        (GtkWindow*)g_list_model_get_item(window_levels, 0), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE,
        "Unable to get background access: %s", ((error) != nullptr ? error->message : "Unknown reason"));

    gtk_message_dialog_format_secondary_text(
        GTK_MESSAGE_DIALOG(dialog),
        "To let EasyEffects ask again, run: \nflatpak permission-reset com.github.wwmm.easyeffects");

    gtk_widget_show(dialog);
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), nullptr);

    if (static_cast<bool>(gtk_switch_get_active(widgets->enable_autostart)) ||
        static_cast<bool>(gtk_switch_get_state(widgets->enable_autostart))) {
      reset_autostart = true;

      util::warning(std::string("portal: setting autostart state and switch to false"));

      gtk_switch_set_state(widgets->enable_autostart, 0);
      gtk_switch_set_active(widgets->enable_autostart, 0);
    }

    if (!static_cast<bool>(gtk_switch_get_active(widgets->shutdown_on_window_close)) ||
        !static_cast<bool>(gtk_switch_get_state(widgets->shutdown_on_window_close))) {
      reset_shutdown = true;

      util::warning(std::string("portal: setting shutdown on window close state and switch to true"));

      gtk_switch_set_state(widgets->shutdown_on_window_close, 1);
      gtk_switch_set_active(widgets->shutdown_on_window_close, 1);
    }

    reset_autostart = false;
    reset_shutdown = false;

    return;
  }

  gtk_switch_set_state(widgets->enable_autostart, gtk_switch_get_active(widgets->enable_autostart));

  gtk_switch_set_state(widgets->shutdown_on_window_close, gtk_switch_get_active(widgets->shutdown_on_window_close));

  reset_autostart = false;
  reset_shutdown = false;

  util::debug("portal: a background request successfully completed");
}

// generic portal update function
void update_background_portal(const bool& state, Widgets* self) {
  XdpBackgroundFlags background_flags = XDP_BACKGROUND_FLAG_NONE;

  g_autoptr(GPtrArray) command_line = nullptr;

  if (state) {
    command_line = g_ptr_array_new_with_free_func(g_free);

    g_ptr_array_add(command_line, g_strdup("easyeffects"));
    g_ptr_array_add(command_line, g_strdup("--gapplication-service"));

    background_flags = XDP_BACKGROUND_FLAG_AUTOSTART;
  }

  auto* reason = g_strdup("EasyEffects Background Access");

  // libportal portal request
  xdp_portal_request_background(portal, nullptr, reason, command_line, background_flags, nullptr,
                                on_request_background_called, self);

  g_free(reason);
}

}  // namespace libportal