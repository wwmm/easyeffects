#ifndef DELAY_HPP
#define DELAY_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Delay : public PluginBase {
   public:
    Delay(std::string tag, std::string schema);
    ~Delay();

    GstElement* delay;

    sigc::connection tempo_connection;

    sigc::signal<void, std::array<double, 2>> tempo;

   private:
    void bind_to_gsettings();
};

#endif
