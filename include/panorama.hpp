#ifndef PANORAMA_HPP
#define PANORAMA_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>

class Panorama {
   public:
    Panorama(std::string tag, std::string schema);
    ~Panorama();

    std::string log_tag, name = "panorama";
    GstElement *plugin, *bin, *panorama, *identity_in, *identity_out;

   private:
    bool is_installed;

    GSettings* settings;

    void bind_to_gsettings();
};

#endif
