#ifndef PITCH_HPP
#define PITCH_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Pitch : public PluginBase {
 public:
  Pitch(const std::string& tag, const std::string& schema);
  ~Pitch();

  GstElement* pitch = nullptr;

 private:
  void bind_to_gsettings();
};

#endif
