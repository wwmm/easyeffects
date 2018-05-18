#ifndef FILTER_HPP
#define FILTER_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <gst/insertbin/gstinsertbin.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>

class Filter {
   public:
    Filter(std::string tag, std::string schema);
    ~Filter();

    std::string log_tag, name = "filter";
    GstElement *plugin, *bin, *filter;

    sigc::connection input_level_connection;
    sigc::connection output_level_connection;

    void on_new_autovolume_level(const std::array<double, 2>& peak);

    sigc::signal<void, std::array<double, 2>> input_level;
    sigc::signal<void, std::array<double, 2>> output_level;

   private:
    bool is_installed;

    GstElement* autovolume;

    GSettings* settings;

    void bind_to_gsettings();
};

#endif
