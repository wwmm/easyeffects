#include "realtime_kit.hpp"
#include <sys/syscall.h>
#include <unistd.h>
#include "util.hpp"

RealtimeKit::RealtimeKit() {
  DBusError error;

  dbus_error_init(&error);

  if (!(bus = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error))) {
    util::warning(log_tag +
                  "Failed to connect to system bus: " + error.message);

    failed = true;
  }

  if (!failed) {
    dbus_connection_set_exit_on_disconnect(bus, false);
  }

  dbus_error_free(&error);
}

RealtimeKit::~RealtimeKit() {
  if (!failed) {
    dbus_connection_close(bus);
    dbus_connection_unref(bus);
  }
}

void RealtimeKit::make_realtime() {
  if (!failed) {
    DBusMessage *m = nullptr, *r = nullptr;
    dbus_uint64_t u64;
    dbus_uint32_t u32;
    DBusError error;
    int nice_level = 12;

    pid_t thread = (pid_t)syscall(SYS_gettid);

    u64 = (dbus_uint64_t)thread;
    u32 = (dbus_uint32_t)nice_level;

    if (!(m = dbus_message_new_method_call(
              RTKIT_SERVICE_NAME, RTKIT_OBJECT_PATH,
              "org.freedesktop.RealtimeKit1", "MakeThreadRealtime"))) {
    }

    if (!dbus_message_append_args(m, DBUS_TYPE_UINT64, &u64, DBUS_TYPE_UINT32,
                                  &u32, DBUS_TYPE_INVALID)) {
    }

    if (!dbus_message_append_args(m, DBUS_TYPE_UINT64, &u64, DBUS_TYPE_UINT32,
                                  &u32, DBUS_TYPE_INVALID)) {
    }

    dbus_error_init(&error);

    if (!(r = dbus_connection_send_with_reply_and_block(bus, m, -1, &error))) {
      util::warning(log_tag + error.name + " : " + +error.message);
    }

    // if (dbus_set_error_from_message(&error, r)) {
    //   util::warning(log_tag + error.name);
    // }

    if (m) {
      dbus_message_unref(m);
    }

    if (r) {
      dbus_message_unref(r);
    }

    dbus_error_free(&error);
  }
}
