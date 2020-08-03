#ifndef PITCH_HPP
#define PITCH_HPP

#include "plugin_base.hpp"

class Pitch : public PluginBase {
 public:
  Pitch(const std::string& tag, const std::string& schema, const std::string& schema_path);
  Pitch(const Pitch&) = delete;
  auto operator=(const Pitch&) -> Pitch& = delete;
  Pitch(const Pitch&&) = delete;
  auto operator=(const Pitch &&) -> Pitch& = delete;
  ~Pitch() override;

  GstElement* pitch = nullptr;

 private:
  void bind_to_gsettings();
};

#endif
