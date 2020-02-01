#ifndef PLUGIN_BASE_HPP
#define PLUGIN_BASE_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>
#include <mutex>

class PluginBase {
 public:
  PluginBase(const std::string& tag, const std::string& plugin_name, const std::string& schema);
  PluginBase(const PluginBase&) = delete;
  auto operator=(const PluginBase&) -> PluginBase& = delete;
  PluginBase(const PluginBase&&) = delete;
  auto operator=(const PluginBase &&) -> PluginBase& = delete;
  virtual ~PluginBase();

  std::string log_tag, name;
  GstElement *plugin = nullptr, *bin = nullptr, *identity_in = nullptr, *identity_out = nullptr;

  bool plugin_is_installed = false;

  void enable();
  void disable();

 protected:
  GSettings* settings = nullptr;

  auto is_installed(GstElement* e) -> bool;
};

#endif
