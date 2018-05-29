#ifndef STEREO_ENHANCER_HPP
#define STEREO_ENHANCER_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class StereoEnhancer : public PluginBase {
   public:
    StereoEnhancer(const std::string& tag, const std::string& schema);
    ~StereoEnhancer();

    GstElement* stereo_enhancer;

    sigc::connection input_level_connection;
    sigc::connection output_level_connection;
    sigc::connection side_level_connection;

    sigc::signal<void, std::array<double, 2>> input_level;
    sigc::signal<void, std::array<double, 2>> output_level;
    sigc::signal<void, std::array<double, 2>> side_level;

   private:
    void bind_to_gsettings();
};

#endif
