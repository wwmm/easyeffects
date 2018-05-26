#ifndef PITCH_HPP
#define PITCH_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>

class Pitch {
   public:
    Pitch(std::string tag, std::string schema);
    ~Pitch();

    std::string log_tag, name = "pitch";
    GstElement *plugin, *bin, *pitch;

   private:
    bool is_installed;

    GSettings* settings;

    void bind_to_gsettings();
};

#endif
