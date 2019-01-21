#ifndef DELAY_HPP
#define DELAY_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Delay : public PluginBase {
 public:
  Delay(const std::string& tag, const std::string& schema);
  ~Delay();

  GstElement* delay = nullptr;

 private:
  void bind_to_gsettings();
};

#endif
