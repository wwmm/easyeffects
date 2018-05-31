#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtkmm/dialog.h>
#include "application.hpp"
#include "application_ui.hpp"
#include "pulse_manager.hpp"
#include "util.hpp"

Application::Application()
    : Gtk::Application("com.github.wwmm.pulseeffects",
                       Gio::APPLICATION_HANDLES_COMMAND_LINE) {
    Glib::set_application_name("PulseEffects");
    Glib::setenv("PULSE_PROP_application.id", "com.github.wwmm.pulseeffects");
    Glib::setenv("PULSE_PROP_application.icon_name", "pulseeffects");

    add_main_option_entry(
        Gio::Application::OPTION_TYPE_BOOL, "quit", 'q',
        _("Quit PulseEffects. Useful when running in service mode."));

    add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL, "presets", 'p',
                          _("Show available presets."));

    add_main_option_entry(Gio::Application::OPTION_TYPE_STRING, "load-preset",
                          'l',
                          _("Load a preset. Example: pulseeffects -l music"));
}

Application::~Application() {}

Glib::RefPtr<Application> Application::create() {
    return Glib::RefPtr<Application>(new Application());
}

int Application::on_command_line(
    const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) {
    auto options = command_line->get_options_dict();

    if (options->contains("quit")) {
        quit();
    } else if (options->contains("presets")) {
        std::string list;

        for (auto name : presets_manager->get_names()) {
            list += name + ",";
        }

        util::info(log_tag + _("Presets: ") + list);
    } else if (options->contains("load-preset")) {
        Glib::ustring name;

        if (!options->lookup_value("load-preset", name)) {
            util::debug(log_tag + "failed to load preset: " + name);
        } else {
            presets_manager->load(name);
        }
    } else {
        activate();
    }

    return Gtk::Application::on_command_line(command_line);
}

void Application::on_startup() {
    Gtk::Application::on_startup();

    running_as_service = false;

    create_appmenu();

    settings = Gio::Settings::create("com.github.wwmm.pulseeffects");

    pm = std::shared_ptr<PulseManager>(new PulseManager());

    sie = std::make_shared<SinkInputEffects>(pm);
    soe = std::make_shared<SourceOutputEffects>(pm);
    presets_manager = std::make_shared<PresetsManager>();

    pm->new_default_sink.connect([&](auto name) {
        util::debug("new default sink: " + name);
        sie->set_output_sink_name(name);
        soe->webrtc->set_probe_src_device(name + ".monitor");
    });

    pm->new_default_source.connect([&](auto name) {
        util::debug("new default source: " + name);
        soe->set_source_monitor_name(name);
    });

    if (get_flags() & Gio::ApplicationFlags::APPLICATION_IS_SERVICE) {
        running_as_service = true;

        pm->find_sink_inputs();
        pm->find_source_outputs();
        pm->find_sinks();
        pm->find_sources();

        util::debug(log_tag + "Running in Background");

        hold();
    }
}

void Application::on_activate() {
    if (get_active_window() == nullptr) {
        auto window = ApplicationUi::create(this);

        add_window(*window);

        window->signal_hide().connect([window]() { delete window; });

        window->show_all();
        window->present();

        pm->find_sink_inputs();
        pm->find_source_outputs();
        pm->find_sinks();
        pm->find_sources();
    }
}

void Application::create_appmenu() {
    add_action("about", [&]() {
        auto builder = Gtk::Builder::create_from_resource(
            "/com/github/wwmm/pulseeffects/about.glade");

        auto dialog = (Gtk::Dialog*)builder->get_object("about_dialog").get();

        dialog->set_transient_for(*get_active_window());

        dialog->show();

        // Bring it to the front, in case it was already shown:
        dialog->present();
    });

    add_action("quit", [&] {
        auto windows = get_windows();

        for (auto w : windows) {
            w->hide();
        }

        quit();
    });

    auto menu = Gio::Menu::create();

    menu->append("About", "app.about");
    menu->append("Quit", "app.quit");

    set_app_menu(menu);
}
