#ifndef DRAW_SPECTRUM_HPP
#define DRAW_SPECTRUM_HPP

#include <gtkmm.h>
#include "application_window.hpp"

class DrawSpectrum {
   public:
    DrawSpectrum(ApplicationWindow* app);

   private:
    ApplicationWindow* app;

    bool on_draw(const Cairo::RefPtr<Cairo::Context>& ctx);
};

#endif
