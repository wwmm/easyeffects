#ifndef GATE_HPP
#define GATE_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>

class Gate {
   public:
    Gate(std::string tag, std::string schema);
    ~Gate();

    std::string log_tag, name = "gate";
    GstElement *plugin, *bin, *gate;

    sigc::connection gating_connection;

    sigc::signal<void, double> gating;

   private:
    bool is_installed;

    GSettings* settings;

    void bind_to_gsettings();
};

#endif
