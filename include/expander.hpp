#ifndef EXPANDER_HPP
#define EXPANDER_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Expander : public PluginBase {
   public:
    Expander(const std::string& tag, const std::string& schema);
    ~Expander();

    GstElement* expander;

    sigc::connection input_level_connection;
    sigc::connection output_level_connection;

    sigc::signal<void, std::array<double, 2>> input_level;
    sigc::signal<void, std::array<double, 2>> output_level;

   private:
    void bind_to_gsettings();
};

#endif
