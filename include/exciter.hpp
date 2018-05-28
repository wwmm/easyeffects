#ifndef EXCITER_HPP
#define EXCITER_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Exciter : public PluginBase {
   public:
    Exciter(std::string tag, std::string schema);
    ~Exciter();

    GstElement* exciter;

    sigc::connection harmonics_connection;

    sigc::signal<void, double> harmonics;

   private:
    void bind_to_gsettings();
};

#endif
