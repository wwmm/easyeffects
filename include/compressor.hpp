#ifndef COMPRESSOR_HPP
#define COMPRESSOR_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Compressor : public PluginBase {
 public:
  Compressor(const std::string& tag, const std::string& schema);
  ~Compressor();

  GstElement* compressor = nullptr;

  sigc::connection input_level_connection, output_level_connection,
      reduction_connection, sidechain_connection, curve_connection;

  sigc::signal<void, std::array<double, 2>> input_level, output_level;
  sigc::signal<void, double> reduction, sidechain, curve;

 private:
  void bind_to_gsettings();
};

#endif
