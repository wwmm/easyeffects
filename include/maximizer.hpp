#ifndef MAXIMIZER_HPP
#define MAXIMIZER_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>

class Maximizer {
   public:
    Maximizer(std::string tag, std::string schema);
    ~Maximizer();

    std::string log_tag, name = "maximizer";
    GstElement *plugin, *bin, *maximizer;

    sigc::connection reduction_connection;

    sigc::signal<void, double> reduction;

   private:
    bool is_installed;

    GSettings* settings;

    void bind_to_gsettings();
};

#endif
