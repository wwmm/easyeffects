#ifndef CROSSFEED_HPP
#define CROSSFEED_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Crossfeed : public PluginBase {
   public:
    Crossfeed(std::string tag, std::string schema);
    ~Crossfeed();

    GstElement* crossfeed;

   private:
    void bind_to_gsettings();
};

#endif
