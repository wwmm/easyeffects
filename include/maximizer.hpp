#ifndef MAXIMIZER_HPP
#define MAXIMIZER_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Maximizer : public PluginBase {
   public:
    Maximizer(std::string tag, std::string schema);
    ~Maximizer();

    GstElement* maximizer;

    sigc::connection reduction_connection;

    sigc::signal<void, double> reduction;

   private:
    void bind_to_gsettings();
};

#endif
