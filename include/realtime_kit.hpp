#ifndef REALTIMEKIT_HPP
#define REALTIMEKIT_HPP

#include <dbus-1.0/dbus/dbus.h>

class RealtimeKit {
 public:
  RealtimeKit();

  ~RealtimeKit();

 private:
  DBusError error;
  DBusConnection* bus;
};

#endif
