#ifndef CRYSTALIZER_HPP
#define CRYSTALIZER_HPP

#include "plugin_base.hpp"

class Crystalizer : public PluginBase {
 public:
  Crystalizer(const std::string& tag, const std::string& schema, const std::string& schema_path);
  Crystalizer(const Crystalizer&) = delete;
  auto operator=(const Crystalizer&) -> Crystalizer& = delete;
  Crystalizer(const Crystalizer&&) = delete;
  auto operator=(const Crystalizer &&) -> Crystalizer& = delete;
  ~Crystalizer() override;

  GstElement* crystalizer = nullptr;

  sigc::connection range_before_connection, range_after_connection;

  sigc::signal<void, double> range_before, range_after;

 private:
  void bind_to_gsettings();
};

#endif
