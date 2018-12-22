#ifndef REALTIMEKIT_HPP
#define REALTIMEKIT_HPP

#include <dbus-1.0/dbus/dbus.h>
#include <iostream>

class RealtimeKit {
 public:
  RealtimeKit();

  ~RealtimeKit();

 private:
  std::string log_tag = "rtkit: ";

  DBusError error;
  DBusConnection* bus;
};

#endif
