#ifndef CROSSFEED_HPP
#define CROSSFEED_HPP

#include "plugin_base.hpp"

class Crossfeed : public PluginBase {
 public:
  Crossfeed(const std::string& tag, const std::string& schema);
  Crossfeed(const Crossfeed&) = delete;
  auto operator=(const Crossfeed&) -> Crossfeed& = delete;
  Crossfeed(const Crossfeed&&) = delete;
  auto operator=(const Crossfeed &&) -> Crossfeed& = delete;
  ~Crossfeed() override;

  GstElement* crossfeed = nullptr;

 private:
  void bind_to_gsettings();
};

#endif
