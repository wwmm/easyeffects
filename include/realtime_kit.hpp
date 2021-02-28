/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef REALTIMEKIT_HPP
#define REALTIMEKIT_HPP

#include <giomm.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <climits>
#include "util.hpp"

#define RTKIT_SERVICE_NAME "org.freedesktop.RealtimeKit1"
#define RTKIT_OBJECT_PATH "/org/freedesktop/RealtimeKit1"

class RealtimeKit {
 public:
  RealtimeKit(const std::string& tag);

  void set_priority(const std::string& source_name, const int& priority);
  void set_nice(const std::string& source_name, const int& nice_value);

 private:
  std::string log_tag;

  Glib::RefPtr<Gio::DBus::Proxy> proxy;
  Glib::RefPtr<Gio::DBus::Proxy> properties_proxy;

  auto get_int_property(const char* propname) -> long long;

  void make_realtime(const std::string& source_name, const int& priority);

  void make_high_priority(const std::string& source_name, const int& nice_value);
};

#endif
