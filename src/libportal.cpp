#include "libportal.hpp"
#include "preferences_general.hpp"

namespace {

bool reset_shutdown = false, reset_autostart = false;

GSettings* settings = nullptr;

GtkSwitch *enable_autostart = nullptr, *shutdown_on_window_close = nullptr;

}  // namespace

namespace libportal {

XdpPortal* portal = nullptr;

void on_request_background_called(GObject* source, GAsyncResult* result, gpointer widgets_ptr) {
  if (enable_autostart == nullptr || shutdown_on_window_close == nullptr) {
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

    if (static_cast<bool>(gtk_switch_get_active(enable_autostart)) ||
        static_cast<bool>(gtk_switch_get_state(enable_autostart))) {
      reset_autostart = true;

      util::warning(std::string("portal: setting autostart state and switch to false"));

      gtk_switch_set_state(enable_autostart, 0);
      gtk_switch_set_active(enable_autostart, 0);
    }

    if (!static_cast<bool>(gtk_switch_get_active(shutdown_on_window_close)) ||
        !static_cast<bool>(gtk_switch_get_state(shutdown_on_window_close))) {
      reset_shutdown = true;

      util::warning(std::string("portal: setting shutdown on window close state and switch to true"));

      gtk_switch_set_state(shutdown_on_window_close, 1);
      gtk_switch_set_active(shutdown_on_window_close, 1);
    }

    reset_autostart = false;
    reset_shutdown = false;

    return;
  }

  gtk_switch_set_state(enable_autostart, gtk_switch_get_active(enable_autostart));

  gtk_switch_set_state(shutdown_on_window_close, gtk_switch_get_active(shutdown_on_window_close));

  reset_autostart = false;
  reset_shutdown = false;

  util::debug("portal: a background request successfully completed");
}

// generic portal update function
void update_background_portal(const bool& state) {
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
                                on_request_background_called, nullptr);

  g_free(reason);
}

auto on_enable_autostart(GtkSwitch* obj, gboolean state, gpointer user_data) -> gboolean {
  if (!reset_autostart) {
    util::debug("portal: requesting autostart file since autostart is enabled");

    update_background_portal(state != 0);
  }

  return 1;
}

auto on_shutdown_on_window_close_called(GtkSwitch* btn, gboolean state, gpointer user_data) -> gboolean {
  if (!reset_shutdown) {
    bool enable_autostart = g_settings_get_boolean(settings, "enable-autostart") != 0;

    if (enable_autostart) {
      const auto* msg = (state == 0)
                            ? "portal: requesting both background access and autostart file since autostart is enabled"
                            : "portal: requesting autostart access since autostart enabled";

      util::debug(msg);

      update_background_portal(true);
    } else {
      if (state == 0) {
        util::debug("portal: requesting only background access, not creating autostart file");

        update_background_portal(false);
      } else {
        util::debug("portal: not requesting any access since enabling shutdown on window close");

        gtk_switch_set_state(shutdown_on_window_close, gtk_switch_get_active(shutdown_on_window_close));
      }
    }
  }

  return 1;
}

void init(GtkSwitch* g_enable_autostart, GtkSwitch* g_shutdown_on_window_close) {
  enable_autostart = g_enable_autostart;
  shutdown_on_window_close = g_shutdown_on_window_close;

  if (enable_autostart == nullptr || shutdown_on_window_close == nullptr) {
    util::error("Programming error. When using libportal its pointers to our settings widgets must be initialized.");
  }

  settings = g_settings_new("com.github.wwmm.easyeffects.libportal");

  g_signal_connect(enable_autostart, "state-set", G_CALLBACK(on_enable_autostart), nullptr);
  g_signal_connect(shutdown_on_window_close, "state-set", G_CALLBACK(on_shutdown_on_window_close_called), nullptr);

  // sanity checks in case switch(es) was somehow already set previously.

  if ((gtk_switch_get_active(shutdown_on_window_close) == 0) && (gtk_switch_get_active(enable_autostart) == 0)) {
    util::debug(std::string("portal: Running portal sanity check, autostart and shutdown switches are disabled"));

    update_background_portal(false);
  } else if ((gtk_switch_get_active(shutdown_on_window_close) != 0) && (gtk_switch_get_active(enable_autostart) != 0)) {
    util::debug(std::string("portal: Running portal sanity check, autostart and shutdown switches are enabled"));

    update_background_portal(true);
  } else if ((gtk_switch_get_active(shutdown_on_window_close) == 0) && (gtk_switch_get_active(enable_autostart) != 0)) {
    util::debug(std::string(
        "portal: Running portal sanity check, autostart switch is enabled and shutdown switch is disabled"));

    update_background_portal(true);
  }
}

}  // namespace libportal