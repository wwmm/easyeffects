#include <glib-unix.h>
#include <glibmm/i18n.h>
#include "application_ui.hpp"
#include "config.h"

auto sigterm(void* data) -> bool {
  auto app = static_cast<Application*>(data);

  for (auto w : app->get_windows()) {
    w->hide();
  }

  app->quit();

  return G_SOURCE_REMOVE;
}

auto main(int argc, char* argv[]) -> int {
  try {
    // Init internationalization support before anything else

    auto bindtext_output = bindtextdomain(GETTEXT_PACKAGE, LOCALE_DIR);
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
