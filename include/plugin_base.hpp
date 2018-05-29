#ifndef PLUGIN_BASE_HPP
#define PLUGIN_BASE_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>

class PluginBase {
   public:
    PluginBase(std::string tag, std::string plugin_name, std::string schema);
    ~PluginBase();

    std::string log_tag, name;
    GstElement *plugin, *bin, *identity_in, *identity_out;

    bool in_pad_cb = false;

   protected:
    GSettings* settings;

    bool is_installed(GstElement* e);
};

#endif
