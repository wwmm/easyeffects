#include "application_window.hpp"

ApplicationWindow::ApplicationWindow(Application* app)
    : builder(Gtk::Builder::create_from_resource(
          "/com/github/wwmm/pulseeffects/application.glade")) {
    builder->get_widget("ApplicationWindow", this->window);

    app->add_window(*this->window);

    this->window->show();
}
