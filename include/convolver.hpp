#ifndef CONVOLVER_HPP
#define CONVOLVER_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Convolver : public PluginBase {
 public:
  Convolver(const std::string& tag, const std::string& schema);
  ~Convolver();

  GstElement *convolver = nullptr, *adapter = nullptr;

 private:
  void bind_to_gsettings();
};

#endif
