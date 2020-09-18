#ifndef GATE_HPP
#define GATE_HPP

#include "plugin_base.hpp"

class Gate : public PluginBase {
 public:
  Gate(const std::string& tag, const std::string& schema, const std::string& schema_path);
  Gate(const Gate&) = delete;
  auto operator=(const Gate&) -> Gate& = delete;
  Gate(const Gate&&) = delete;
  auto operator=(const Gate &&) -> Gate& = delete;
  ~Gate() override;

  GstElement* gate = nullptr;

  sigc::connection gating_connection;

  sigc::signal<void, double> gating;

 private:
  void bind_to_gsettings();
};

#endif
