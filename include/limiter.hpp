#ifndef LIMITER_HPP
#define LIMITER_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <gst/insertbin/gstinsertbin.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>

class Limiter {
   public:
    Limiter(std::string tag, std::string schema);
    ~Limiter();

    std::string log_tag;
    GstElement *plugin, *bin;

    double get_attenuation();
    void on_new_autovolume_level(const std::array<double, 2>& peak);

    sigc::signal<void, std::array<double, 2>> input_level;
    sigc::signal<void, std::array<double, 2>> output_level;
    sigc::signal<void, double> attenuation;

   private:
    bool is_installed;

    GstElement *limiter, *autovolume;

    GSettings* settings;

    sigc::connection input_level_connection;
    sigc::connection output_level_connection;
    sigc::connection attenuation_connection;

    void bind_to_gsettings();
};

#endif
