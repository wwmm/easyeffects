#ifndef PLUGIN_BASE_HPP
#define PLUGIN_BASE_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <array>
#include <atomic>
#include <iostream>

class PluginBase {
 public:
  PluginBase(const std::string& tag,
             const std::string& plugin_name,
             const std::string& schema);
  virtual ~PluginBase();

  std::string log_tag, name;
  GstElement *plugin, *bin, *identity_in, *identity_out;

  std::atomic<bool> changing_pipeline;

  bool plugin_is_installed = false;

  void enable();
  void disable();

 protected:
  GSettings* settings;

  bool is_installed(GstElement* e);
};

#endif
