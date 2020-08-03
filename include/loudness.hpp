#ifndef LOUDNESS_HPP
#define LOUDNESS_HPP

#include "plugin_base.hpp"

class Loudness : public PluginBase {
 public:
  Loudness(const std::string& tag, const std::string& schema, const std::string& schema_path);
  Loudness(const Loudness&) = delete;
  auto operator=(const Loudness&) -> Loudness& = delete;
  Loudness(const Loudness&&) = delete;
  auto operator=(const Loudness &&) -> Loudness& = delete;
  ~Loudness() override;

  GstElement* loudness = nullptr;

 private:
  void bind_to_gsettings();
};

#endif
