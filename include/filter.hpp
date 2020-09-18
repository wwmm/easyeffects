#ifndef FILTER_HPP
#define FILTER_HPP

#include "plugin_base.hpp"

class Filter : public PluginBase {
 public:
  Filter(const std::string& tag, const std::string& schema, const std::string& schema_path);
  Filter(const Filter&) = delete;
  auto operator=(const Filter&) -> Filter& = delete;
  Filter(const Filter&&) = delete;
  auto operator=(const Filter &&) -> Filter& = delete;
  ~Filter();

  GstElement* filter = nullptr;

  sigc::connection input_level_connection, output_level_connection;

  sigc::signal<void, std::array<double, 2>> input_level, output_level;

 private:
  void bind_to_gsettings();
};

#endif
