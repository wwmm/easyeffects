#ifndef DEESSER_HPP
#define DEESSER_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Deesser : public PluginBase {
 public:
  Deesser(const std::string& tag, const std::string& schema);
  ~Deesser();

  GstElement* deesser = nullptr;

  sigc::connection compression_connection, detected_connection;

  sigc::signal<void, double> compression, detected;

 private:
  void bind_to_gsettings();
};

#endif
