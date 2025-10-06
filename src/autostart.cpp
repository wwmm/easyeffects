/**
 * Copyright © 2017-2025 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "autostart.hpp"
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <glibconfig.h>
#include <libportal-qt6/portal-qt6.h>
#include <libportal/background.h>
#include <libportal/parent.h>
#include <qobject.h>
#include <QWindow>
#include <string>
#include "db_manager.hpp"
#include "tags_app.hpp"
#include "util.hpp"

namespace {

void on_request_background_called([[maybe_unused]] GObject* source,
                                  GAsyncResult* result,
                                  [[maybe_unused]] gpointer widgets_ptr) {
  g_autoptr(GError) error = nullptr;

  // libportal check if portal request worked

  if (xdp_portal_request_background_finish(XdpQt::globalPortalObject(), result, &error) == 0) {
    std::string reason;
    std::string explanation;
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

    util::warning("Due to error, setting autostart state and switch to false");

    db::Main::setAutostartOnLogin(false);

    return;
  }
}

}  // namespace

Autostart::Autostart(QObject* parent) : QObject(parent) {
  connect(db::Main::self(), &db::Main::autostartOnLoginChanged, [&]() { update_background_portal(); });
}

void Autostart::set_window(QWindow* window) {
  this->window = window;
}

void Autostart::update_background_portal() {
  auto xdp_parent = xdp_parent_new_qt(window);

  g_autoptr(GPtrArray) cmd = nullptr;
  XdpBackgroundFlags flags = XDP_BACKGROUND_FLAG_NONE;

  if (db::Main::autostartOnLogin()) {
    cmd = g_ptr_array_new_with_free_func(g_free);

    g_ptr_array_add(cmd, g_strdup("easyeffects"));
    g_ptr_array_add(cmd, g_strdup("--service-mode"));
    g_ptr_array_add(cmd, g_strdup("--hide-window"));

    flags = XDP_BACKGROUND_FLAG_AUTOSTART;
  }

  xdp_portal_request_background(XdpQt::globalPortalObject(), xdp_parent, const_cast<char*>("EasyEffects-Autostart"),
                                cmd, flags, nullptr, on_request_background_called, this);

  xdp_parent_free(xdp_parent);
}
