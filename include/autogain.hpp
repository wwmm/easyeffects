#ifndef AUTOGAIN_HPP
#define AUTOGAIN_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class AutoGain : public PluginBase {
 public:
  AutoGain(const std::string& tag, const std::string& schema);
  ~AutoGain();

  GstElement* autogain;

  sigc::signal<void, float> momentary;
  sigc::signal<void, float> shortterm;
  sigc::signal<void, float> integrated;
  sigc::signal<void, float> relative;
  sigc::signal<void, float> loudness;
  sigc::signal<void, float> range;
  sigc::signal<void, float> gain;

 private:
  void bind_to_gsettings();
};

#endif
