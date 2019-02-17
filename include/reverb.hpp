#ifndef REVERB_HPP
#define REVERB_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Reverb : public PluginBase {
 public:
  Reverb(const std::string& tag, const std::string& schema);
  ~Reverb();

  GstElement* reverb = nullptr;

  sigc::connection input_level_connection, output_level_connection;

  sigc::signal<void, std::array<double, 2>> input_level, output_level;

 private:
  void bind_to_gsettings();
};

#endif
