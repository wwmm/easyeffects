#include "spectrum_ui.hpp"
#include "util.hpp"

SpectrumUi::SpectrumUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       const Glib::RefPtr<Gio::Settings>& refSettings,
                       Application* application)
    : Gtk::Grid(cobject), settings(refSettings), app(application) {
  // loading glade widgets

  // builder->get_widget("enable", enable);
}

SpectrumUi::~SpectrumUi() {
  util::debug(log_tag + "destroyed");
}
