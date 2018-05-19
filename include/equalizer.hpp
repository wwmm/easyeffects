#ifndef EQUALIZER_HPP
#define EQUALIZER_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <gst/insertbin/gstinsertbin.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>

class Equalizer {
   public:
    Equalizer(std::string tag, std::string schema);
    ~Equalizer();

    std::string log_tag, name = "equalizer";
    GstElement *plugin, *bin, *equalizer;

   private:
    bool is_installed;

    GSettings* settings;

    void bind_to_gsettings();
};

#endif
