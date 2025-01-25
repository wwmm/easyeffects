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

#include <gio/gio.h>
#include <glib-object.h>
#include <glib-unix.h>
#include <glib.h>
#include <libintl.h>
#include <cerrno>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <ostream>
#include <string>
#include "application.hpp"
#include "config.h"
#include "util.hpp"

auto sigterm(void* data) -> int {
  auto* app = G_APPLICATION(data);

  app::hide_all_windows(app);

  g_application_quit(app);

  return G_SOURCE_REMOVE;
}

auto main(int argc, char* argv[]) -> int {
  util::debug("easyeffects version: " + std::string(VERSION));

  try {
    // Init internationalization support before anything else

    auto* bindtext_output = bindtextdomain(GETTEXT_PACKAGE, LOCALE_DIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    if (bindtext_output != nullptr) {
      util::debug("locale directory: " + std::string(bindtext_output));
    } else if (errno == ENOMEM) {
      util::warning("bindtextdomain: Not enough memory available!");

      return errno;
    }

    auto* app = app::application_new();

    g_unix_signal_add(2, G_SOURCE_FUNC(sigterm), app);

    auto status = g_application_run(app, argc, argv);

    g_object_unref(app);

    util::debug("Exitting the main function with status: " + util::to_string(status, ""));

    return status;
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';

    return EXIT_FAILURE;
  }
}
