#ifndef AUTOGAIN_HPP
#define AUTOGAIN_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class AutoGain : public PluginBase {
 public:
  AutoGain(const std::string& tag, const std::string& schema);
  ~AutoGain();

  GstElement* autogain = nullptr;

  sigc::signal<void, float> momentary, shortterm, integrated, relative, loudness, range, gain;

 private:
  void bind_to_gsettings();
};

#endif
