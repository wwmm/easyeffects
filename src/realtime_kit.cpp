#include "realtime_kit.hpp"
#include <limits.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "util.hpp"

RealtimeKit::RealtimeKit(const std::string& tag) {
  log_tag = tag + "rtkit: ";

  DBusError error;

  dbus_error_init(&error);

  if (!(bus = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error))) {
    util::warning(log_tag +
                  "Failed to connect to system bus: " + error.message);
  }

  dbus_connection_set_exit_on_disconnect(bus, false);

  dbus_error_free(&error);
}

RealtimeKit::~RealtimeKit() {
  dbus_connection_close(bus);
  dbus_connection_unref(bus);
}

/*
  This method code was adapted from the one Pulseaudio sources. File rtkit.c
*/

long long RealtimeKit::get_int_property(const char* propname) {
  DBusMessage *m = NULL, *r = NULL;
  DBusMessageIter iter, subiter;
  DBusError error;
  int current_type;
  long long propval = 0;
  const char* interfacestr = "org.freedesktop.RealtimeKit1";

  dbus_error_init(&error);

  m = dbus_message_new_method_call(RTKIT_SERVICE_NAME, RTKIT_OBJECT_PATH,
                                   "org.freedesktop.DBus.Properties", "Get");

  dbus_message_append_args(m, DBUS_TYPE_STRING, &interfacestr, DBUS_TYPE_STRING,
                           &propname, DBUS_TYPE_INVALID);

  if (!(r = dbus_connection_send_with_reply_and_block(bus, m, -1, &error))) {
    util::warning(log_tag + error.name + " : " + error.message);
  }

  dbus_message_iter_init(r, &iter);

  while ((current_type = dbus_message_iter_get_arg_type(&iter)) !=
         DBUS_TYPE_INVALID) {
    if (current_type == DBUS_TYPE_VARIANT) {
      dbus_message_iter_recurse(&iter, &subiter);

      while ((current_type = dbus_message_iter_get_arg_type(&subiter)) !=
             DBUS_TYPE_INVALID) {
        if (current_type == DBUS_TYPE_INT32) {
          dbus_int32_t i32;

          dbus_message_iter_get_basic(&subiter, &i32);

          propval = i32;
        }

        if (current_type == DBUS_TYPE_INT64) {
          dbus_int32_t i64;

          dbus_message_iter_get_basic(&subiter, &i64);

          propval = i64;
        }

        dbus_message_iter_next(&subiter);
      }
    }

    dbus_message_iter_next(&iter);
  }

  return propval;
}

void RealtimeKit::make_realtime(const std::string& source_name,
                                const int& priority) {
  DBusMessage *m = nullptr, *r = nullptr;
  dbus_uint64_t u64;
  dbus_uint32_t u32;
  DBusError error;

  pid_t thread = (pid_t)syscall(SYS_gettid);

  u64 = (dbus_uint64_t)thread;
  u32 = (dbus_uint32_t)priority;

  m = dbus_message_new_method_call(RTKIT_SERVICE_NAME, RTKIT_OBJECT_PATH,
                                   "org.freedesktop.RealtimeKit1",
                                   "MakeThreadRealtime");

  dbus_message_append_args(m, DBUS_TYPE_UINT64, &u64, DBUS_TYPE_UINT32, &u32,
                           DBUS_TYPE_INVALID);

  dbus_error_init(&error);

  if (!(r = dbus_connection_send_with_reply_and_block(bus, m, -1, &error))) {
    util::warning(log_tag + error.name + " : " + error.message);
  } else {
    util::debug(log_tag + "changed " + source_name +
                " thread real-time priority value to " +
                std::to_string(priority));
  }

  if (m) {
    dbus_message_unref(m);
  }

  if (r) {
    dbus_message_unref(r);
  }

  dbus_error_free(&error);
}

void RealtimeKit::make_high_priority(const std::string& source_name,
                                     const int& nice_value) {
  DBusMessage *m = nullptr, *r = nullptr;
  dbus_uint64_t u64;
  dbus_int32_t u32;
  DBusError error;

  pid_t thread = (pid_t)syscall(SYS_gettid);

  u64 = (dbus_uint64_t)thread;
  u32 = (dbus_int32_t)nice_value;

  m = dbus_message_new_method_call(RTKIT_SERVICE_NAME, RTKIT_OBJECT_PATH,
                                   "org.freedesktop.RealtimeKit1",
                                   "MakeThreadHighPriority");

  dbus_message_append_args(m, DBUS_TYPE_UINT64, &u64, DBUS_TYPE_INT32, &u32,
                           DBUS_TYPE_INVALID);

  dbus_error_init(&error);

  if (!(r = dbus_connection_send_with_reply_and_block(bus, m, -1, &error))) {
    util::warning(log_tag + error.name + " : " + error.message);
  } else {
    util::debug(log_tag + "changed " + source_name + " thread nice value to " +
                std::to_string(nice_value));
  }

  if (m) {
    dbus_message_unref(m);
  }

  if (r) {
    dbus_message_unref(r);
  }

  dbus_error_free(&error);
}

void RealtimeKit::set_priority(const std::string& source_name,
                               const int& priority) {
#ifdef SCHED_RESET_ON_FORK

  struct sched_param sp;

  if (pthread_setschedparam(pthread_self(), SCHED_RR | SCHED_RESET_ON_FORK,
                            &sp) == 0) {
    util::debug("SCHED_RR|SCHED_RESET_ON_FORK worked.");

    return;
  }

#endif

#ifdef RLIMIT_RTTIME

  struct rlimit rl;
  long long rttime;

  rttime = get_int_property("RTTimeUSecMax");

  if (getrlimit(RLIMIT_RTTIME, &rl) >= 0) {
    rl.rlim_cur = rl.rlim_max = rttime;

    if (setrlimit(RLIMIT_RTTIME, &rl) < 0) {
      util::warning(log_tag + "failed to set rlimit value for the " +
                    source_name + " thread");
    }
  } else {
    util::warning(log_tag + "failed to get rlimit value for the " +
                  source_name + " thread");
  }

#endif

#if defined(__linux__)

  make_realtime(source_name, priority);

#endif
}

void RealtimeKit::set_nice(const std::string& source_name,
                           const int& nice_value) {
#ifdef HAVE_SYS_RESOURCE_H

  if (setpriority(PRIO_PROCESS, 0, nice_value) >= 0) {
    util::debug("setpriority() worked.");

    return;
  }

#endif

#if defined(__linux__)

  make_high_priority(source_name, nice_value);

#endif
}
