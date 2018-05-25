#ifndef CROSSFEED_HPP
#define CROSSFEED_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>

class Crossfeed {
   public:
    Crossfeed(std::string tag, std::string schema);
    ~Crossfeed();

    std::string log_tag, name = "crossfeed";
    GstElement *plugin, *bin, *crossfeed;

   private:
    bool is_installed;

    GSettings* settings;

    void bind_to_gsettings();
};

#endif
