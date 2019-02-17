#ifndef STEREO_TOOLS_HPP
#define STEREO_TOOLS_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class StereoTools : public PluginBase {
 public:
  StereoTools(const std::string& tag, const std::string& schema);
  ~StereoTools();

  GstElement* stereo_tools = nullptr;

  sigc::connection input_level_connection, output_level_connection;

  sigc::signal<void, std::array<double, 2>> input_level, output_level;

 private:
  void bind_to_gsettings();
};

#endif
