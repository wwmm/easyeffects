#include "libportal.hpp"
#include "preferences_general.hpp"

namespace {

bool resetting_shutdown = false, resetting_autostart = false;

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
    std::string reason;
    std::string explanation;

    if (error != nullptr) {
      // 19 seemingly corresponds to the "cancelled" error which actually means the permission is in a revoked state.
      if (error->code == 19) {
        reason = "Background access has been denied";
        explanation = "Please allow EasyEffects to ask again with flatpak permission-reset com.github.wwmm.easyeffects";
      } else {
        reason = "Unknown error";
        explanation = "Please verify your system has a XDG Background Portal implementation running and working.";
      }
    } else {
      reason = "Unknown error";
      explanation = "No explanation could be provided, error was null";
    }

    util::debug(std::string("a background request failed: ") +
                ((error) != nullptr ? error->message : "unknown reason"));
    util::warning(reason);
    util::warning(explanation);

    // TODO find a bettery way of getting the preferences window
    // it shouldn't be possible to open the preferences window without the top level window open,
    // so the index 1 should correspond with the preferences window
    auto* window_levels = gtk_window_get_toplevels();
    GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(g_list_model_get_item(window_levels, 1)), GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE,
                                               "Unable to get background access: %s", reason.c_str());

    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", explanation.c_str());

    gtk_widget_show(dialog);
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), nullptr);

    // if autostart is wrongly enabled (we got an error when talking to the portal), we must reset it
    if (static_cast<bool>(gtk_switch_get_active(enable_autostart)) ||
        static_cast<bool>(gtk_switch_get_state(enable_autostart))) {
      resetting_autostart = true;

      util::warning(std::string("due to error, setting autostart state and switch to false"));

      gtk_switch_set_state(enable_autostart, 0);
      gtk_switch_set_active(enable_autostart, 0);
    }
    // if running in the background (which happens if we don't shutdown on window close) is wrongly enabled (we got an
    // error when talking to the portal), we must reset it
    if (!static_cast<bool>(gtk_switch_get_active(shutdown_on_window_close)) ||
        !static_cast<bool>(gtk_switch_get_state(shutdown_on_window_close))) {
      resetting_shutdown = true;

      util::warning(std::string("due to error, setting shutdown on window close state and switch to true"));

      gtk_switch_set_state(shutdown_on_window_close, 1);
      gtk_switch_set_active(shutdown_on_window_close, 1);
    }

    resetting_autostart = false;
    resetting_shutdown = false;

    return;
  }

  gtk_switch_set_state(enable_autostart, gtk_switch_get_active(enable_autostart));

  gtk_switch_set_state(shutdown_on_window_close, gtk_switch_get_active(shutdown_on_window_close));

  resetting_autostart = false;
  resetting_shutdown = false;

  util::debug("a background request successfully completed");
}

// generic portal update function
void update_background_portal(const bool& use_autostart) {
  XdpBackgroundFlags background_flags = XDP_BACKGROUND_FLAG_NONE;

  g_autoptr(GPtrArray) command_line = nullptr;

  if (use_autostart) {
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

void on_enable_autostart(GtkSwitch* obj, gboolean state, gpointer user_data) {
  // this callback could be triggered when the settings are reset by other code due to an error calling the portal, in
  // that case we must not call the portal again.
  if (!resetting_autostart) {
    state == 1 ? util::debug("requesting autostart file since autostart is enabled")
               : util::debug("not requesting autostart file since autostart is disabled");

    update_background_portal(state != 0);
  }
}

void on_shutdown_on_window_close(GtkSwitch* btn, gboolean state, gpointer user_data) {
  // this callback could be triggered when the settings are reset by other code due to an error calling the portal, in
  // that case we must not call the portal again.
  if (!resetting_shutdown) {
    if (gtk_switch_get_active(enable_autostart) != 0) {
      const auto* msg = (state == 0) ? "requesting both background access and autostart file since autostart is enabled"
                                     : "requesting autostart access since autostart enabled";

      util::debug(msg);

      update_background_portal(true);
    } else {
      if (state == 0) {
        util::debug("requesting only background access, not creating autostart file");

        update_background_portal(false);
      } else {
        util::debug("not requesting any access since enabling shutdown on window close");

        gtk_switch_set_state(shutdown_on_window_close, gtk_switch_get_active(shutdown_on_window_close));
      }
    }
  }
}

void init(GtkSwitch* g_enable_autostart, GtkSwitch* g_shutdown_on_window_close) {
  enable_autostart = g_enable_autostart;
  shutdown_on_window_close = g_shutdown_on_window_close;

  if (enable_autostart == nullptr || shutdown_on_window_close == nullptr) {
    util::error("Programming error. When using libportal its pointers to our settings widgets must be initialized.");
  }

  if (portal == nullptr) {
    portal = xdp_portal_new();
  }

  settings = g_settings_new("com.github.wwmm.easyeffects.libportal");

  ui::gsettings_bind_widget(settings, "enable-autostart", g_enable_autostart);

  g_signal_connect(enable_autostart, "state-set", G_CALLBACK(on_enable_autostart), nullptr);
  g_signal_connect(shutdown_on_window_close, "state-set", G_CALLBACK(on_shutdown_on_window_close), nullptr);

  // sanity checks in case switch(es) was somehow already set previously.
  // give extra info for debugging purposes
  // the only the case where we must not ask the portal for access is if autostart is disabled and shutdown on window
  // close is disabled

  auto enable_autostart_state = gtk_switch_get_active(enable_autostart);
  auto shutdown_on_window_close_state = gtk_switch_get_active(shutdown_on_window_close);

  if ((enable_autostart_state == 0) && (shutdown_on_window_close_state == 0)) {
    util::debug(std::string("doing portal sanity check, autostart and shutdown switches are disabled"));

    update_background_portal(false);
  } else if ((enable_autostart_state != 0) && (shutdown_on_window_close_state != 0)) {
    util::debug(std::string("doing portal sanity check, autostart and shutdown switches are enabled"));

    update_background_portal(true);
  } else if ((enable_autostart_state != 0) && (shutdown_on_window_close_state == 0)) {
    util::debug(std::string("doing portal sanity check, autostart switch is enabled and shutdown switch is disabled"));

    update_background_portal(true);
  } else {
    util::debug(
        std::string("not doing portal sanity check, autostart switch should be disabled and shutdown switch should be "
                    "enabled so no background portal access is needed"));
  }
}

}  // namespace libportal
