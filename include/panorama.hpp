#ifndef PANORAMA_HPP
#define PANORAMA_HPP

#include "plugin_base.hpp"

class Panorama : public PluginBase {
   public:
    Panorama(const std::string& tag, const std::string& schema);
    ~Panorama();

    GstElement* panorama;

   private:
    void bind_to_gsettings();
};

#endif
