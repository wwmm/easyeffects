#ifndef REVERB_HPP
#define REVERB_HPP

#include "plugin_base.hpp"

class Reverb : public PluginBase {
 public:
  Reverb(const std::string& tag, const std::string& schema, const std::string& schema_path);
  Reverb(const Reverb&) = delete;
  auto operator=(const Reverb&) -> Reverb& = delete;
  Reverb(const Reverb&&) = delete;
  auto operator=(const Reverb &&) -> Reverb& = delete;
  ~Reverb() override;

  GstElement* reverb = nullptr;

  sigc::connection input_level_connection, output_level_connection;

  sigc::signal<void, std::array<double, 2>> input_level, output_level;

 private:
  void bind_to_gsettings();
};

#endif
