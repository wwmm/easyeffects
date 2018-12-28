#ifndef EQUALIZER_HPP
#define EQUALIZER_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Equalizer : public PluginBase {
 public:
  Equalizer(const std::string& tag, const std::string& schema);
  ~Equalizer();

  GstElement *equalizer_L = nullptr, *equalizer_R = nullptr, *queue_L = nullptr,
             *queue_R = nullptr;

  void update_equalizer();

 private:
  void bind_band(GstElement* equalizer, const int index);
  void unbind_band(GstElement* equalizer, const int index);
};

#endif
