#ifndef FILTER_HPP
#define FILTER_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Filter : public PluginBase {
   public:
    Filter(const std::string& tag, const std::string& schema);
    ~Filter();

    GstElement* filter;

    sigc::connection input_level_connection;
    sigc::connection output_level_connection;

    sigc::signal<void, std::array<double, 2>> input_level;
    sigc::signal<void, std::array<double, 2>> output_level;

   private:
    void bind_to_gsettings();
};

#endif
