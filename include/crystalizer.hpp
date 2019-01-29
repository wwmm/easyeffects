#ifndef CRYSTALIZER_HPP
#define CRYSTALIZER_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Crystalizer : public PluginBase {
 public:
  Crystalizer(const std::string& tag, const std::string& schema);
  ~Crystalizer();

  GstElement *crystalizer_low = nullptr, *crystalizer_high = nullptr;

 private:
  void bind_to_gsettings();
};

#endif
