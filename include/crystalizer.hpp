#ifndef CRYSTALIZER_HPP
#define CRYSTALIZER_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Crystalizer : public PluginBase {
   public:
    Crystalizer(const std::string& tag, const std::string& schema);
    ~Crystalizer();

    GstElement* crystalizer;

   private:
    void bind_to_gsettings();
};

#endif
