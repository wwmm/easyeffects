#include "realtime_kit.hpp"
#include "util.hpp"

RealtimeKit::RealtimeKit() {
  bool failed = false;

  dbus_error_init(&error);

  if (!(bus = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error))) {
    util::warning(log_tag +
                  "Failed to connect to system bus: " + error.message);

    errno = -EIO;
    failed = true;
  }

  if (!failed) {
    dbus_connection_set_exit_on_disconnect(bus, false);
  }
}

RealtimeKit::~RealtimeKit() {
  dbus_error_free(&error);
  dbus_connection_close(bus);
  dbus_connection_unref(bus);
}
