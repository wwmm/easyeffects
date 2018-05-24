#ifndef EQUALIZER_HPP
#define EQUALIZER_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>

class Equalizer {
   public:
    Equalizer(std::string tag, std::string schema);
    ~Equalizer();

    bool is_enabled = false;

    std::string log_tag, name = "equalizer";

    GstElement *plugin, *bin, *equalizer, *in_level, *out_level;

    void init_equalizer();

   private:
    bool is_installed;

    GSettings* settings;

    void bind_band(const int index);
    void unbind_band(const int index);
};

#endif
