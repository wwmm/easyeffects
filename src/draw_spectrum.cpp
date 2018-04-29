#include "draw_spectrum.hpp"

DrawSpectrum::DrawSpectrum(ApplicationWindow* application) : app(application) {
    app->spectrum->signal_draw().connect(
        sigc::mem_fun(*this, &DrawSpectrum::on_draw), false);
}

bool DrawSpectrum::on_draw(const Cairo::RefPtr<Cairo::Context>& ctx) {
    ctx->paint();

    g_debug("draw");

    return false;
}
