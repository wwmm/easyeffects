#include "application_window.hpp"

ApplicationWindow::ApplicationWindow(Application* application)
    : app(application),
      builder(Gtk::Builder::create_from_resource(
          "/com/github/wwmm/pulseeffects/application.glade")) {
    Gtk::IconTheme::get_default()->add_resource_path(
        "/com/github/wwmm/pulseeffects/");

    builder->get_widget("ApplicationWindow", this->window);
    builder->get_widget("theme_switch", this->theme_switch);
    builder->get_widget("enable_all_apps", this->enable_all_apps);
    builder->get_widget("use_default_sink", this->use_default_sink);
    builder->get_widget("use_default_source", this->use_default_source);
    builder->get_widget("input_device", this->input_device);
    builder->get_widget("output_device", this->output_device);

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;
    auto flag_invert_boolean =
        Gio::SettingsBindFlags::SETTINGS_BIND_INVERT_BOOLEAN;

    this->app->settings->bind("use-dark-theme", this->theme_switch, "active",
                              flag);

    this->app->settings->bind("use-dark-theme",
                              Gtk::Settings::get_default().get(),
                              "gtk_application_prefer_dark_theme", flag);

    this->app->settings->bind("enable-all-apps", this->enable_all_apps,
                              "active", flag);

    this->app->settings->bind("use-default-sink", this->use_default_sink,
                              "active", flag);

    this->app->settings->bind("use-default-sink", this->input_device, "active",
                              flag | flag_invert_boolean);

    this->app->settings->bind("use-default-source", this->use_default_source,
                              "active", flag);

    this->app->settings->bind("use-default-source", this->output_device,
                              "active", flag | flag_invert_boolean);

    this->app->add_window(*this->window);

    this->window->show();
}
