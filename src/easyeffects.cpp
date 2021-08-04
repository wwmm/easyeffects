/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <glib-unix.h>
#include "application.hpp"
#include "config.h"
#include "gtkmm/window.h"

auto sigterm(void* data) -> bool {
  auto* app = static_cast<Application*>(data);

  for (const auto& w : app->get_windows()) {
    w->hide();
  }

  app->quit();

  return G_SOURCE_REMOVE;
}

auto main(int argc, char* argv[]) -> int {
  try {
    // Init internationalization support before anything else

    auto* bindtext_output = bindtextdomain(GETTEXT_PACKAGE, LOCALE_DIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    if (bindtext_output != nullptr) {
      util::debug("main: locale directory: " + std::string(bindtext_output));
    } else if (errno == ENOMEM) {
      util::warning("main: bindtextdomain: Not enough memory available!");

      return errno;
    }

    auto app = Application::create();

    g_unix_signal_add(2, (GSourceFunc)sigterm, app.get());

    return app->run(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;

    return EXIT_FAILURE;
  }
}
