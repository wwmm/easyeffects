#ifndef CRYSTALIZER_HPP
#define CRYSTALIZER_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Crystalizer : public PluginBase {
 public:
  Crystalizer(const std::string& tag, const std::string& schema);
  ~Crystalizer();

  GstElement* crystalizer = nullptr;

  sigc::connection range_before_connection, range_after_connection;

  sigc::signal<void, double> range_before, range_after;

 private:
  void bind_to_gsettings();
};

#endif
