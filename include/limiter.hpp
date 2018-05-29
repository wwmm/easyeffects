#ifndef LIMITER_HPP
#define LIMITER_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Limiter : public PluginBase {
   public:
    Limiter(const std::string& tag, const std::string& schema);
    ~Limiter();

    GstElement* limiter;

    sigc::connection input_level_connection;
    sigc::connection output_level_connection;
    sigc::connection attenuation_connection;

    void on_new_autovolume_level(const std::array<double, 2>& peak);

    sigc::signal<void, std::array<double, 2>> input_level;
    sigc::signal<void, std::array<double, 2>> output_level;
    sigc::signal<void, double> attenuation;

   private:
    GstElement* autovolume;

    void bind_to_gsettings();
};

#endif
