#include "application_window.hpp"

ApplicationWindow::ApplicationWindow(Application* application)
    : app(application),
      builder(Gtk::Builder::create_from_resource(
          "/com/github/wwmm/pulseeffects/application.glade")),
      settings(app->settings) {
    Gtk::IconTheme::get_default()->add_resource_path(
        "/com/github/wwmm/pulseeffects/");

    builder->get_widget("ApplicationWindow", window);
    builder->get_widget("theme_switch", theme_switch);
    builder->get_widget("enable_all_apps", enable_all_apps);
    builder->get_widget("use_default_sink", use_default_sink);
    builder->get_widget("use_default_source", use_default_source);
    builder->get_widget("input_device", input_device);
    builder->get_widget("output_device", output_device);
    builder->get_widget("show_spectrum", show_spectrum);

    buffer_in = (Gtk::Adjustment*)builder->get_object("buffer_in").get();
    buffer_out = (Gtk::Adjustment*)builder->get_object("buffer_out").get();
    latency_in = (Gtk::Adjustment*)builder->get_object("latency_in").get();
    latency_out = (Gtk::Adjustment*)builder->get_object("latency_out").get();
    spectrum_n_points =
        (Gtk::Adjustment*)builder->get_object("spectrum_n_points").get();

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;
    auto flag_invert_boolean =
        Gio::SettingsBindFlags::SETTINGS_BIND_INVERT_BOOLEAN;

    settings->bind("use-dark-theme", theme_switch, "active", flag);

    settings->bind("use-dark-theme", Gtk::Settings::get_default().get(),
                   "gtk_application_prefer_dark_theme", flag);

    settings->bind("enable-all-apps", enable_all_apps, "active", flag);

    settings->bind("use-default-sink", use_default_sink, "active", flag);

    settings->bind("use-default-sink", input_device, "sensitive",
                   flag | flag_invert_boolean);

    settings->bind("use-default-source", use_default_source, "active", flag);

    settings->bind("use-default-source", output_device, "sensitive",
                   flag | flag_invert_boolean);

    settings->bind("buffer-out", buffer_out, "value", flag);
    settings->bind("latency-out", latency_out, "value", flag);

    settings->bind("buffer-in", buffer_in, "value", flag);
    settings->bind("latency-in", latency_in, "value", flag);

    settings->bind("show-spectrum", show_spectrum, "active", flag);
    settings->bind("spectrum-n-points", spectrum_n_points, "value", flag);

    app->add_window(*window);

    window->show();
}
