#ifndef MULTIBAND_GATE_HPP
#define MULTIBAND_GATE_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class MultibandGate : public PluginBase {
 public:
  MultibandGate(const std::string& tag, const std::string& schema);
  ~MultibandGate();

  GstElement* multiband_gate = nullptr;

  sigc::connection input_level_connection, output_level_connection;

  sigc::connection output0_connection, output1_connection, output2_connection, output3_connection, gating0_connection,
      gating1_connection, gating2_connection, gating3_connection;

  sigc::signal<void, std::array<double, 2>> input_level, output_level;

  sigc::signal<void, double> output0, output1, output2, output3, gating0, gating1, gating2, gating3;

 private:
  void bind_to_gsettings();
};

#endif
