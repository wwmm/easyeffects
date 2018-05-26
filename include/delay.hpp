#ifndef DELAY_HPP
#define DELAY_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>

class Delay {
   public:
    Delay(std::string tag, std::string schema);
    ~Delay();

    std::string log_tag, name = "delay";
    GstElement *plugin, *bin, *delay;

    sigc::connection tempo_connection;

    sigc::signal<void, std::array<double, 2>> tempo;

   private:
    bool is_installed;

    GSettings* settings;

    void bind_to_gsettings();
};

#endif
