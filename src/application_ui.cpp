#include <glibmm.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/settings.h>
#include <iostream>
#include <memory>
#include "application_ui.hpp"
#include "util.hpp"

ApplicationWindow::ApplicationWindow(Application* application)
    : app(application),
      builder(Gtk::Builder::create_from_resource(
          "/com/github/wwmm/pulseeffects/application.glade")),
      settings(app->settings) {
    apply_css_style("listbox.css");

    Gtk::IconTheme::get_default()->add_resource_path(
        "/com/github/wwmm/pulseeffects/");

    // loading glade widgets

    builder->get_widget("ApplicationWindow", window);
    builder->get_widget("theme_switch", theme_switch);
    builder->get_widget("enable_autostart", enable_autostart);
    builder->get_widget("enable_all_apps", enable_all_apps);
    builder->get_widget("use_default_sink", use_default_sink);
    builder->get_widget("use_default_source", use_default_source);
    builder->get_widget("input_device", input_device);
    builder->get_widget("output_device", output_device);
    builder->get_widget("show_spectrum", show_spectrum);
    builder->get_widget("reset_settings", reset_settings);
    builder->get_widget("spectrum", spectrum);

    get_object("buffer_in", buffer_in);
    get_object("buffer_out", buffer_out);
    get_object("latency_in", latency_in);
    get_object("latency_out", latency_out);
    get_object("spectrum_n_points", spectrum_n_points);
    get_object("sink_list", sink_list);
    get_object("source_list", source_list);

    // binding glade widgets to gsettings keys

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

    init_autostart_switch();

    // callbacks connection

    enable_autostart->signal_state_set().connect(
        sigc::bind(
            sigc::mem_fun(*this, &ApplicationWindow::on_enable_autostart),
            log_tag),
        false);

    reset_settings->signal_clicked().connect(
        sigc::mem_fun(*this, &ApplicationWindow::on_reset_settings));

    spectrum->signal_draw().connect(
        sigc::mem_fun(*this, &ApplicationWindow::on_draw));
    spectrum->signal_enter_notify_event().connect(
        sigc::mem_fun(*this, &ApplicationWindow::on_enter_notify_event));
    spectrum->signal_leave_notify_event().connect(
        sigc::mem_fun(*this, &ApplicationWindow::on_leave_notify_event));

    spectrum->signal_motion_notify_event().connect(sigc::bind(
        sigc::mem_fun(*this, &ApplicationWindow::on_motion_notify_event),
        spectrum));

    app->pm->sink_added.connect(
        sigc::mem_fun(*this, &ApplicationWindow::on_sink_added));

    // show main window

    app->add_window(*window);

    window->show();
}

void ApplicationWindow::apply_css_style(std::string css_file_name) {
    auto provider = Gtk::CssProvider::create();

    provider->load_from_resource("/com/github/wwmm/pulseeffects/" +
                                 css_file_name);

    auto screen = Gdk::Screen::get_default();
    auto priority = GTK_STYLE_PROVIDER_PRIORITY_APPLICATION;

    Gtk::StyleContext::add_provider_for_screen(screen, provider, priority);
}

void ApplicationWindow::init_autostart_switch() {
    auto path =
        Glib::get_user_config_dir() + "/autostart/pulseeffects-service.desktop";

    try {
        auto file = Gio::File::create_for_path(path);

        if (file->query_exists()) {
            enable_autostart->set_active(true);
        } else {
            enable_autostart->set_active(false);
        }
    } catch (const Glib::Exception& ex) {
        util::warning(log_tag + ex.what());
    }
}

bool ApplicationWindow::on_enable_autostart(bool state, std::string tag) {
    auto path =
        Glib::get_user_config_dir() + "/autostart/pulseeffects-service.desktop";

    auto file = Gio::File::create_for_path(path);

    if (state) {
        try {
            Glib::RefPtr<Gio::FileOutputStream> stream;

            if (file->query_exists())
                stream = file->replace();
            else
                stream = file->create_file();

            stream->write("[Desktop Entry]\n");
            stream->write("Name=PulseEffects\n");
            stream->write("Comment=PulseEffects Service\n");
            stream->write("Exec=pulseeffects --gapplication-service\n");
            stream->write("Icon=pulseeffects\n");
            stream->write("StartupNotify=false\n");
            stream->write("Terminal=false\n");
            stream->write("Type=Application\n");
            stream->close();
            stream.reset();

            util::debug(tag + "autostart file created");
        } catch (const Glib::Exception& ex) {
            util::warning(tag + ex.what());
        }
    } else {
        try {
            file->remove();

            util::debug(tag + "autostart file removed");
        } catch (const Glib::Exception& ex) {
            util::warning(tag + ex.what());
        }
    }

    return false;
}

void ApplicationWindow::on_reset_settings() {
    settings->reset("buffer-in");
    settings->reset("buffer-out");
    settings->reset("latency-in");
    settings->reset("latency-out");
    settings->reset("show-spectrum");
    settings->reset("spectrum-n-points");
    settings->reset("use-dark-theme");
    settings->reset("enable-all-apps");
    settings->reset("use-default-sink");
    settings->reset("use-default-source");
}

bool ApplicationWindow::on_draw(const Cairo::RefPtr<Cairo::Context>& ctx) {
    ctx->paint();

    g_debug("draw event");

    return false;
}

bool ApplicationWindow::on_enter_notify_event(GdkEventCrossing* event) {
    g_debug("enter event");

    return false;
}

bool ApplicationWindow::on_leave_notify_event(GdkEventCrossing* event) {
    g_debug("leave event");

    return false;
}

bool ApplicationWindow::on_motion_notify_event(GdkEventMotion* event,
                                               Gtk::DrawingArea* area) {
    auto allocation = area->get_allocation();

    // auto width = allocation.get_width();
    auto height = allocation.get_height();

    mouse_intensity = -event->y * 120 / height;

    util::debug("mouse intensity: " + std::to_string(mouse_intensity));

    return false;
}

void ApplicationWindow::on_sink_added(std::shared_ptr<mySinkInfo> info) {
    // bool add_to_list = true;

    // Gtk::TreeModel::Row row = *sink_list->append();

    // row[Gtk::TreeModelColumn<int>()] = 2;
    // row[Gtk::TreeModelColumn<Glib::ustring>()] = "wwmm";

    // auto children = sink_list->children();
    //
    // for (auto c : children) {
    //     auto idx = c.get_value(Gtk::TreeModelColumn<int>());
    //     auto name = c.get_value(Gtk::TreeModelColumn<Glib::ustring>());
    //
    //     std::cout << idx << name << std::endl;
    // }
}
