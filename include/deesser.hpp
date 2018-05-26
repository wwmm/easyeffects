#ifndef DEESSER_HPP
#define DEESSER_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>

class Deesser {
   public:
    Deesser(std::string tag, std::string schema);
    ~Deesser();

    std::string log_tag, name = "deesser";
    GstElement *plugin, *bin, *deesser;

    sigc::connection compression_connection, detected_connection;

    sigc::signal<void, double> compression, detected;

   private:
    bool is_installed;

    GSettings* settings;

    void bind_to_gsettings();
};

#endif
