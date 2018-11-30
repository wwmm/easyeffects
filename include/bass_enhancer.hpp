#ifndef BASS_ENHANCER_HPP
#define BASS_ENHANCER_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class BassEnhancer : public PluginBase {
 public:
  BassEnhancer(const std::string& tag, const std::string& schema);
  ~BassEnhancer();

  GstElement* bass_enhancer = nullptr;

  sigc::connection harmonics_connection;

  sigc::signal<void, double> harmonics;

 private:
  void bind_to_gsettings();
};

#endif
