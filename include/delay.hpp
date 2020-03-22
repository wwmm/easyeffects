#ifndef DELAY_HPP
#define DELAY_HPP

#include "plugin_base.hpp"

class Delay : public PluginBase {
 public:
  Delay(const std::string& tag, const std::string& schema);
  Delay(const Delay&) = delete;
  auto operator=(const Delay&) -> Delay& = delete;
  Delay(const Delay&&) = delete;
  auto operator=(const Delay &&) -> Delay& = delete;
  ~Delay() override;

  GstElement* delay = nullptr;

 private:
  void bind_to_gsettings();
};

#endif
