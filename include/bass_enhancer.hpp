#ifndef BASS_ENHANCER_HPP
#define BASS_ENHANCER_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>

class BassEnhancer {
   public:
    BassEnhancer(std::string tag, std::string schema);
    ~BassEnhancer();

    std::string log_tag, name = "bass_enhancer";
    GstElement *plugin, *bin, *bass_enhancer;

    sigc::connection harmonics_connection;

    sigc::signal<void, double> harmonics;

   private:
    bool is_installed;

    GSettings* settings;

    void bind_to_gsettings();
};

#endif
