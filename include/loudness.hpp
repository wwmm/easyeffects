#ifndef LOUDNESS_HPP
#define LOUDNESS_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Loudness : public PluginBase {
 public:
  Loudness(const std::string& tag, const std::string& schema);
  ~Loudness();

  GstElement* loudness = nullptr;

 private:
  void bind_to_gsettings();
};

#endif
