#include "realtime_kit.hpp"
#include <limits.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "util.hpp"

RealtimeKit::RealtimeKit(const std::string& tag) {
  log_tag = tag + "rtkit: ";

  try {
    proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BusType::BUS_TYPE_SYSTEM, RTKIT_SERVICE_NAME,
                                                  RTKIT_OBJECT_PATH, "org.freedesktop.RealtimeKit1");

    properties_proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BusType::BUS_TYPE_SYSTEM, RTKIT_SERVICE_NAME,
                                                             RTKIT_OBJECT_PATH, "org.freedesktop.DBus.Properties");
  } catch (const Glib::Error& err) {
    util::warning(log_tag + "Failed to connect to system bus: " + err.what().c_str());
  }
}

RealtimeKit::~RealtimeKit() {}

/*
  This method code was adapted from the one in Pulseaudio sources. File rtkit.c
*/

long long RealtimeKit::get_int_property(const char* propname) {
  Glib::VariantBase reply_body;
  long long propval = 0;
  const char* interfacestr = "org.freedesktop.RealtimeKit1";

  Glib::VariantContainerBase args = Glib::VariantContainerBase::create_tuple(std::vector<Glib::VariantBase>(
      {Glib::Variant<Glib::ustring>::create(interfacestr), Glib::Variant<Glib::ustring>::create(propname)}));

  try {
    reply_body = properties_proxy->call_sync("Get", args);

    // The rtkit reply is encoded as a tuple containing `@v <@x 123456>` instead
    // of just plain @x or @i
    if (reply_body.get_type_string() == "(v)") {
      Glib::VariantBase child =
          Glib::VariantBase::cast_dynamic<Glib::Variant<std::tuple<Glib::VariantBase>>>(reply_body)
              .get_child<Glib::VariantBase>(0);

      if (child.get_type_string() == "i") {
        propval = *(const gint32*)Glib::VariantBase::cast_dynamic<Glib::Variant<gint32>>(child).get_data();
      } else if (child.get_type_string() == "x") {
        propval = *(const gint64*)Glib::VariantBase::cast_dynamic<Glib::Variant<gint64>>(child).get_data();
      } else {
        util::warning(log_tag + " Expected value of type i or x but received " + child.get_type_string());
      }
    } else {
      util::warning(log_tag + " Expected value of type (v) but received " + reply_body.get_type_string());
    }
  } catch (const Glib::Error& err) {
    util::warning(log_tag + err.what().c_str());
  }

  return propval;
}

void RealtimeKit::make_realtime(const std::string& source_name, const int& priority) {
#if defined(__linux__)

  pid_t thread = (pid_t)syscall(SYS_gettid);
  guint64 u64 = (guint64)thread;
  guint32 u32 = (guint32)priority;

  Glib::VariantContainerBase args = Glib::VariantContainerBase::create_tuple(
      std::vector<Glib::VariantBase>({Glib::Variant<guint64>::create(u64), Glib::Variant<guint32>::create(u32)}));

  try {
    proxy->call_sync("MakeThreadRealtime", args);

    util::debug(log_tag + "changed " + source_name + " thread real-time priority value to " + std::to_string(priority));
  } catch (const Glib::Error& err) {
    util::warning(log_tag + "MakeThreadRealtime: " + err.what().c_str());
  }

#endif
}

void RealtimeKit::make_high_priority(const std::string& source_name, const int& nice_value) {
#if defined(__linux__)

  pid_t thread = (pid_t)syscall(SYS_gettid);
  guint64 u64 = (guint64)thread;
  gint32 i32 = (gint32)nice_value;

  Glib::VariantContainerBase args = Glib::VariantContainerBase::create_tuple(
      std::vector<Glib::VariantBase>({Glib::Variant<guint64>::create(u64), Glib::Variant<gint32>::create(i32)}));

  try {
    proxy->call_sync("MakeThreadHighPriority", args);

    util::debug(log_tag + "changed " + source_name + " thread nice value to " + std::to_string(nice_value));
  } catch (const Glib::Error& err) {
    util::warning(log_tag + "MakeThreadHighPriority: " + err.what().c_str());
  }

#endif
}

void RealtimeKit::set_priority(const std::string& source_name, const int& priority) {
#ifdef SCHED_RESET_ON_FORK

  struct sched_param sp;

  if (pthread_setschedparam(pthread_self(), SCHED_RR | SCHED_RESET_ON_FORK, &sp) == 0) {
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
      util::warning(log_tag + "failed to set rlimit value for the " + source_name + " thread");
    }
  } else {
    util::warning(log_tag + "failed to get rlimit value for the " + source_name + " thread");
  }

#endif

#if defined(__linux__)

  make_realtime(source_name, priority);

#endif
}

void RealtimeKit::set_nice(const std::string& source_name, const int& nice_value) {
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
