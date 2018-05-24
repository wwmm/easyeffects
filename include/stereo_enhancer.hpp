#ifndef STEREO_ENHANCER_HPP
#define STEREO_ENHANCER_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>

class StereoEnhancer {
   public:
    StereoEnhancer(std::string tag, std::string schema);
    ~StereoEnhancer();

    std::string log_tag, name = "stereo_enhancer";
    GstElement *plugin, *stereo_enhancer;

    sigc::connection input_level_connection;
    sigc::connection output_level_connection;

    sigc::signal<void, std::array<double, 2>> input_level;
    sigc::signal<void, std::array<double, 2>> output_level;

   private:
    bool is_installed;

    GSettings* settings;

    void bind_to_gsettings();
};

#endif
