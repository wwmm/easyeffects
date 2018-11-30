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

  sigc::connection compression_connection;

  sigc::signal<void, double> compression;

 private:
  void bind_to_gsettings();
};

#endif
