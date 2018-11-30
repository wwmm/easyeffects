#ifndef EQUALIZER_HPP
#define EQUALIZER_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Equalizer : public PluginBase {
 public:
  Equalizer(const std::string& tag, const std::string& schema);
  ~Equalizer();

  GstElement* equalizer = nullptr;

  void update_equalizer();

 private:
  void bind_band(const int index);
  void unbind_band(const int index);
};

#endif
