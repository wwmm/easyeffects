#ifndef PITCH_HPP
#define PITCH_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Pitch : public PluginBase {
   public:
    Pitch(std::string tag, std::string schema);
    ~Pitch();

    GstElement* pitch;

   private:
    void bind_to_gsettings();
};

#endif
