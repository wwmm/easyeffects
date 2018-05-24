#ifndef EXCITER_HPP
#define EXCITER_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>

class Exciter {
   public:
    Exciter(std::string tag, std::string schema);
    ~Exciter();

    std::string log_tag, name = "exciter";
    GstElement *plugin, *bin, *exciter;

    sigc::connection harmonics_connection;

    sigc::signal<void, double> harmonics;

   private:
    bool is_installed;

    GSettings* settings;

    void bind_to_gsettings();
};

#endif
