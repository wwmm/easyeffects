#ifndef GATE_HPP
#define GATE_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Gate : public PluginBase {
 public:
  Gate(const std::string& tag, const std::string& schema);
  ~Gate();

  GstElement* gate = nullptr;

  sigc::connection gating_connection;

  sigc::signal<void, double> gating;

 private:
  void bind_to_gsettings();
};

#endif
