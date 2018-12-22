#ifndef REALTIMEKIT_HPP
#define REALTIMEKIT_HPP

#include <dbus-1.0/dbus/dbus.h>
#include <iostream>

#define RTKIT_SERVICE_NAME "org.freedesktop.RealtimeKit1"
#define RTKIT_OBJECT_PATH "/org/freedesktop/RealtimeKit1"

class RealtimeKit {
 public:
  RealtimeKit();

  ~RealtimeKit();

  void make_realtime();

 private:
  std::string log_tag = "rtkit: ";

  bool failed = false;

  DBusConnection* bus;

  long long get_int_property(const char* propname);
};

#endif
