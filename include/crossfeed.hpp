#ifndef CROSSFEED_HPP
#define CROSSFEED_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Crossfeed : public PluginBase {
 public:
  Crossfeed(const std::string& tag, const std::string& schema);
  ~Crossfeed();

  GstElement* crossfeed = nullptr;

 private:
  void bind_to_gsettings();
};

#endif
