#include <glib-unix.h>
#include <glibmm/i18n.h>
#include "application_ui.hpp"
#include "config.h"

bool sigterm(void* data) {
  auto app = static_cast<Application*>(data);

  for (auto w : app->get_windows()) {
    w->hide();
  }

  app->quit();

  return G_SOURCE_REMOVE;
}

int main(int argc, char* argv[]) {
  // Init internationalization support before anything else

  bindtextdomain(GETTEXT_PACKAGE, LOCALE_DIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);

  auto app = Application::create();

  g_unix_signal_add(2, (GSourceFunc)sigterm, app.get());

  auto status = app->run(argc, argv);

  return status;
}
