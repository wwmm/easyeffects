#ifndef LIMITER_HPP
#define LIMITER_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Limiter : public PluginBase {
 public:
  Limiter(const std::string& tag, const std::string& schema);
  ~Limiter();

  GstElement* limiter = nullptr;

  sigc::connection input_level_connection, output_level_connection, attenuation_connection;

  sigc::signal<void, std::array<double, 2>> input_level, output_level;
  sigc::signal<void, double> attenuation;

 private:
  void bind_to_gsettings();
};

#endif
