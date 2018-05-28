#ifndef PANORAMA_HPP
#define PANORAMA_HPP

#include "plugin_base.hpp"

class Panorama : public PluginBase {
   public:
    Panorama(std::string tag, std::string schema);
    ~Panorama();

    GstElement* panorama;

   private:
    void bind_to_gsettings();
};

#endif
