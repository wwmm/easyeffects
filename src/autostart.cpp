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

#include "autostart.hpp"
#include <libportal-qt6/portal-qt6.h>
#include <libportal/background.h>
#include <libportal/parent.h>
#include <qobject.h>
#include <QWindow>
#include <string>
#include "gio/gio.h"
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
#include "tags_app.hpp"
#include "util.hpp"

namespace {

void on_request_background_called([[maybe_unused]] GObject* source,
                                  GAsyncResult* result,
                                  [[maybe_unused]] gpointer widgets_ptr) {
  g_autoptr(GError) error = nullptr;

  // libportal check if portal request worked

  if (xdp_portal_request_background_finish(XdpQt::globalPortalObject(), result, &error) == 0) {
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    std::string reason;
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    std::string explanation;
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    std::string error_message;

    if (error != nullptr) {
      error_message = error->message;

      if (error->code == 19) {
        // 19 seemingly corresponds to the "cancelled" error which actually means the permission is in a revoked state.

        reason = "Background access has been denied";

        explanation =
            std::string("Please allow Easy Effects to ask again with Flatpak permission-reset ") + tags::app::id;
      } else {
        reason = "Generic error";
        explanation = "Please verify your system has a XDG Background Portal implementation running and working.";
      }
    } else {
      error_message = "unknown reason";
      reason = "Unknown error";
      explanation = "No explanation could be provided, error was null";
    }

    util::debug("A background request failed: " + error_message);
    util::warning(reason);
    util::warning(explanation);

    // TODO find a bettery way of getting the preferences window
    // it shouldn't be possible to open the preferences window without the top level window open,
    // so the index 1 should correspond with the preferences window
    // ui::show_simple_message_dialog(GTK_WIDGET(g_list_model_get_item(gtk_window_get_toplevels(), 1)),
    //                                "Unable to get background access: " + reason, explanation);

    // if autostart is wrongly enabled (we got an error when talking to the portal), we must reset it
    // if (static_cast<bool>(gtk_switch_get_active(enable_autostart)) ||
    //     static_cast<bool>(gtk_switch_get_state(enable_autostart))) {
    //   resetting_autostart = true;

    //   util::warning("due to error, setting autostart state and switch to false");

    //   gtk_switch_set_state(enable_autostart, 0);
    //   gtk_switch_set_active(enable_autostart, 0);
    // }

    // if running in the background (which happens if we don't shutdown on window close) is wrongly enabled (we got an
    // error when talking to the portal), we must reset it

    // if (!static_cast<bool>(gtk_switch_get_active(shutdown_on_window_close)) ||
    //     !static_cast<bool>(gtk_switch_get_state(shutdown_on_window_close))) {
    //   resetting_shutdown = true;

    //   util::warning("due to error, setting shutdown on window close state and switch to true");

    //   gtk_switch_set_state(shutdown_on_window_close, 1);
    //   gtk_switch_set_active(shutdown_on_window_close, 1);
    // }

    // resetting_autostart = false;
    // resetting_shutdown = false;

    return;
  }
}

}  // namespace

Autostart::Autostart(QObject* parent) : QObject(parent) {}
void Autostart::set_window(QWindow* window) {
  this->window = window;
}

void Autostart::enable() {
  auto xdp_parent = xdp_parent_new_qt(window);

  xdp_portal_request_background(XdpQt::globalPortalObject(), xdp_parent, const_cast<char*>("EasyEffects-Autostart"),
                                nullptr, XDP_BACKGROUND_FLAG_AUTOSTART, nullptr, on_request_background_called, nullptr);

  xdp_parent_free(xdp_parent);
}