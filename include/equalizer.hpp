#ifndef EQUALIZER_HPP
#define EQUALIZER_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Equalizer : public PluginBase {
 public:
  Equalizer(const std::string& tag,
            const std::string& schema,
            const std::string& schema_left,
            const std::string& schema_right);
  ~Equalizer();

  GstElement *equalizer_L = nullptr, *equalizer_R = nullptr, *queue_L = nullptr,
             *queue_R = nullptr, *interleave = nullptr,
             *audioconvert_out = nullptr;

  void update_equalizer();

 private:
  GSettings *settings_left = nullptr, *settings_right = nullptr;

  void bind_band(GstElement* equalizer, GSettings* cfg, const int index);
  void unbind_band(GstElement* equalizer, const int index);
};

#endif
