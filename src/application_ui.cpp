#include <glibmm.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/settings.h>
#include "application_ui.hpp"
#include "util.hpp"

ApplicationUi::ApplicationUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& refBuilder,
                             Application* application)
    : Gtk::ApplicationWindow(cobject),
      app(application),
      builder(refBuilder),
      settings(app->settings) {
    apply_css_style("listbox.css");

    Gtk::IconTheme::get_default()->add_resource_path(
        "/com/github/wwmm/pulseeffects/");

    // loading glade widgets

    builder->get_widget("theme_switch", theme_switch);
    builder->get_widget("enable_autostart", enable_autostart);
    builder->get_widget("enable_all_apps", enable_all_apps);
    builder->get_widget("use_default_sink", use_default_sink);
    builder->get_widget("use_default_source", use_default_source);
    builder->get_widget("input_device", input_device);
    builder->get_widget("output_device", output_device);
    builder->get_widget("reset_settings", reset_settings);
    builder->get_widget("show_spectrum", show_spectrum);
    builder->get_widget("spectrum_box", spectrum_box);
    builder->get_widget("spectrum", spectrum);
    builder->get_widget("stack", stack);

    get_object("buffer_in", buffer_in);
    get_object("buffer_out", buffer_out);
    get_object("latency_in", latency_in);
    get_object("latency_out", latency_out);
    get_object("spectrum_n_points", spectrum_n_points);
    get_object("sink_list", sink_list);
    get_object("source_list", source_list);

    // signals connection

    enable_autostart->signal_state_set().connect(
        sigc::mem_fun(*this, &ApplicationUi::on_enable_autostart), false);

    reset_settings->signal_clicked().connect(
        sigc::mem_fun(*this, &ApplicationUi::on_reset_settings));

    show_spectrum->signal_state_set().connect(
        sigc::mem_fun(*this, &ApplicationUi::on_show_spectrum), false);

    spectrum->signal_draw().connect(
        sigc::mem_fun(*this, &ApplicationUi::on_spectrum_draw));
    spectrum->signal_enter_notify_event().connect(
        sigc::mem_fun(*this, &ApplicationUi::on_spectrum_enter_notify_event));
    spectrum->signal_leave_notify_event().connect(
        sigc::mem_fun(*this, &ApplicationUi::on_spectrum_leave_notify_event));
    spectrum->signal_motion_notify_event().connect(
        sigc::mem_fun(*this, &ApplicationUi::on_spectrum_motion_notify_event));

    use_default_sink->signal_toggled().connect(
        sigc::mem_fun(*this, &ApplicationUi::on_use_default_sink_toggled));
    use_default_source->signal_toggled().connect(
        sigc::mem_fun(*this, &ApplicationUi::on_use_default_source_toggled));

    input_device->signal_changed().connect(
        sigc::mem_fun(*this, &ApplicationUi::on_input_device_changed));
    output_device->signal_changed().connect(
        sigc::mem_fun(*this, &ApplicationUi::on_output_device_changed));

    stack->connect_property_changed(
        "visible-child",
        sigc::mem_fun(*this, &ApplicationUi::on_stack_visible_child_changed));

    app->pm->sink_added.connect(
        sigc::mem_fun(*this, &ApplicationUi::on_sink_added));
    app->pm->sink_removed.connect(
        sigc::mem_fun(*this, &ApplicationUi::on_sink_removed));
    app->pm->source_added.connect(
        sigc::mem_fun(*this, &ApplicationUi::on_source_added));
    app->pm->source_removed.connect(
        sigc::mem_fun(*this, &ApplicationUi::on_source_removed));

    // sink inputs interface

    sie_ui = SinkInputEffectsUi::create(app->sie);

    app->pm->sink_input_added.connect(
        sigc::mem_fun(*sie_ui, &SinkInputEffectsUi::on_app_added));
    app->pm->sink_input_changed.connect(
        sigc::mem_fun(*sie_ui, &SinkInputEffectsUi::on_app_changed));
    app->pm->sink_input_removed.connect(
        sigc::mem_fun(*sie_ui, &SinkInputEffectsUi::on_app_removed));

    stack->add(*sie_ui, "sink_inputs");
    stack->child_property_icon_name(*sie_ui).set_value(
        "audio-speakers-symbolic");

    // source outputs interface

    soe_ui = SourceOutputEffectsUi::create(app->soe);

    app->pm->source_output_added.connect(
        sigc::mem_fun(*soe_ui, &SourceOutputEffectsUi::on_app_added));
    app->pm->source_output_changed.connect(
        sigc::mem_fun(*soe_ui, &SourceOutputEffectsUi::on_app_changed));
    app->pm->source_output_removed.connect(
        sigc::mem_fun(*soe_ui, &SourceOutputEffectsUi::on_app_removed));

    stack->add(*soe_ui, "source_outputs");
    stack->child_property_icon_name(*soe_ui).set_value(
        "audio-input-microphone-symbolic");

    // temporary spectrum connection. it changes with the selected stack child

    spectrum_connection = app->sie->new_spectrum.connect(
        sigc::mem_fun(*this, &ApplicationUi::on_new_spectrum));

    // binding glade widgets to gsettings keys

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;
    auto flag_get = Gio::SettingsBindFlags::SETTINGS_BIND_GET;
    auto flag_invert_boolean =
        Gio::SettingsBindFlags::SETTINGS_BIND_INVERT_BOOLEAN;

    settings->bind("use-dark-theme", theme_switch, "active", flag);

    settings->bind("use-dark-theme", Gtk::Settings::get_default().get(),
                   "gtk_application_prefer_dark_theme", flag);

    settings->bind("enable-all-apps", enable_all_apps, "active", flag);

    settings->bind("use-default-sink", use_default_sink, "active", flag);

    settings->bind("use-default-sink", output_device, "sensitive",
                   flag | flag_invert_boolean);

    settings->bind("use-default-source", use_default_source, "active", flag);

    settings->bind("use-default-source", input_device, "sensitive",
                   flag | flag_invert_boolean);

    settings->bind("buffer-out", buffer_out, "value", flag);
    settings->bind("latency-out", latency_out, "value", flag);

    settings->bind("buffer-in", buffer_in, "value", flag);
    settings->bind("latency-in", latency_in, "value", flag);

    settings->bind("show-spectrum", show_spectrum, "active", flag);
    settings->bind("show-spectrum", spectrum_box, "visible", flag_get);
    settings->bind("spectrum-n-points", spectrum_n_points, "value", flag);

    init_autostart_switch();
}

ApplicationUi::~ApplicationUi() {
    app->sie->disable_spectrum();
    app->soe->disable_spectrum();
}

ApplicationUi* ApplicationUi::create(Application* app_this) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/application.glade");

    ApplicationUi* window = nullptr;

    builder->get_widget_derived("ApplicationUi", window, app_this);

    return window;
}

void ApplicationUi::apply_css_style(std::string css_file_name) {
    auto provider = Gtk::CssProvider::create();

    provider->load_from_resource("/com/github/wwmm/pulseeffects/" +
                                 css_file_name);

    auto screen = Gdk::Screen::get_default();
    auto priority = GTK_STYLE_PROVIDER_PRIORITY_APPLICATION;

    Gtk::StyleContext::add_provider_for_screen(screen, provider, priority);
}

void ApplicationUi::init_autostart_switch() {
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

void ApplicationUi::clear_spectrum() {
    spectrum_mag.resize(0);

    spectrum->queue_draw();
}

bool ApplicationUi::on_enable_autostart(bool state) {
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

            util::debug(log_tag + "autostart file created");
        } catch (const Glib::Exception& ex) {
            util::warning(log_tag + ex.what());
        }
    } else {
        try {
            file->remove();

            util::debug(log_tag + "autostart file removed");
        } catch (const Glib::Exception& ex) {
            util::warning(log_tag + ex.what());
        }
    }

    return false;
}

void ApplicationUi::on_reset_settings() {
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

    sie_ui->reset();
    soe_ui->reset();
}

bool ApplicationUi::on_show_spectrum(bool state) {
    if (state) {
        app->sie->enable_spectrum();
        app->soe->enable_spectrum();
    } else {
        app->sie->disable_spectrum();
        app->soe->disable_spectrum();
    }

    return false;
}

void ApplicationUi::on_new_spectrum(const std::vector<float>& magnitudes) {
    spectrum_mag = magnitudes;

    spectrum->queue_draw();
}

bool ApplicationUi::on_spectrum_draw(const Cairo::RefPtr<Cairo::Context>& ctx) {
    ctx->paint();

    auto n_bars = spectrum_mag.size();

    if (n_bars > 0) {
        auto allocation = spectrum->get_allocation();
        auto width = allocation.get_width();
        auto height = allocation.get_height();
        auto style_ctx = spectrum->get_style_context();
        auto n_bars = spectrum_mag.size();
        auto x = util::linspace(0, width, n_bars);

        for (uint n = 0; n < n_bars; n++) {
            auto bar_height = spectrum_mag[n] * height;

            ctx->rectangle(x[n], height - bar_height, width / n_bars,
                           bar_height);
        }

        auto color = Gdk::RGBA();

        style_ctx->lookup_color("theme_selected_bg_color", color);
        ctx->set_source_rgba(color.get_red(), color.get_green(),
                             color.get_blue(), 1.0);
        ctx->set_line_width(1.1);
        ctx->stroke();

        if (mouse_inside) {
            std::ostringstream msg;

            msg.precision(0);
            msg << std::fixed << mouse_freq << " Hz, ";
            msg << std::fixed << mouse_intensity << " dB";

            Pango::FontDescription font;
            font.set_family("Monospace");
            font.set_weight(Pango::WEIGHT_BOLD);

            int text_width;
            int text_height;
            auto layout = create_pango_layout(msg.str());
            layout->set_font_description(font);
            layout->get_pixel_size(text_width, text_height);

            ctx->move_to(width - text_width, 0);

            layout->show_in_cairo_context(ctx);
        }
    }

    return false;
}

bool ApplicationUi::on_spectrum_enter_notify_event(GdkEventCrossing* event) {
    mouse_inside = true;
    return false;
}

bool ApplicationUi::on_spectrum_leave_notify_event(GdkEventCrossing* event) {
    mouse_inside = false;
    return false;
}

bool ApplicationUi::on_spectrum_motion_notify_event(GdkEventMotion* event) {
    auto allocation = spectrum->get_allocation();

    auto width = allocation.get_width();
    auto height = allocation.get_height();

    // frequency axis is logarithmic
    // 20 Hz = 10^(1.3), 20000 Hz = 10^(4.3)

    mouse_freq = pow(10, 1.3 + event->x * 3.0 / width);

    // intensity scale is in decibel
    // minimum intensity is -120 dB and maximum is 0 dB

    mouse_intensity = -event->y * 120 / height;

    return false;
}

void ApplicationUi::on_stack_visible_child_changed() {
    auto name = stack->get_visible_child_name();

    if (name == std::string("sink_inputs")) {
        spectrum_connection.disconnect();

        spectrum_connection = app->sie->new_spectrum.connect(
            sigc::mem_fun(*this, &ApplicationUi::on_new_spectrum));
    } else if (name == std::string("source_outputs")) {
        spectrum_connection.disconnect();

        spectrum_connection = app->soe->new_spectrum.connect(
            sigc::mem_fun(*this, &ApplicationUi::on_new_spectrum));
    }

    clear_spectrum();
}

void ApplicationUi::on_sink_added(std::shared_ptr<mySinkInfo> info) {
    bool add_to_list = true;

    auto children = sink_list->children();

    for (auto c : children) {
        uint i;
        std::string name;

        c.get_value(0, i);
        c.get_value(1, name);

        if (info->index == i) {
            add_to_list = false;

            break;
        }
    }

    if (add_to_list) {
        Gtk::TreeModel::Row row = *(sink_list->append());

        row->set_value(0, info->index);
        row->set_value(1, info->name);

        if (app->pm->use_default_sink) {
            if (info->name == app->pm->server_info.default_sink_name) {
                output_device->set_active(row);
            }
        } else {
            auto iter = output_device->get_active();

            if (iter) {
                if (info->name == app->pm->server_info.default_sink_name) {
                    output_device->set_active(iter);
                }
            }
        }

        util::debug(log_tag + "added sink: " + info->name);
    }
}

void ApplicationUi::on_sink_removed(uint idx) {
    Gtk::TreeIter default_iter;
    Gtk::TreeIter remove_iter;
    std::string remove_name;

    auto children = sink_list->children();

    for (auto c : children) {
        uint i;
        std::string name;

        c.get_value(0, i);
        c.get_value(1, name);

        if (idx == i) {
            remove_iter = c;
            remove_name = name;
        }

        if (name == app->pm->server_info.default_sink_name) {
            default_iter = c;
        }
    }

    sink_list->erase(remove_iter);

    util::debug(log_tag + "removed sink: " + remove_name);

    // auto iter = output_device->get_active();
    //
    // if (!iter) {
    //     output_device->set_active(default_iter);
    // }
}

void ApplicationUi::on_source_added(std::shared_ptr<mySourceInfo> info) {
    bool add_to_list = true;

    auto children = source_list->children();

    for (auto c : children) {
        uint i;
        std::string name;

        c.get_value(0, i);
        c.get_value(1, name);

        if (info->index == i) {
            add_to_list = false;

            break;
        }
    }

    if (add_to_list) {
        Gtk::TreeModel::Row row = *(source_list->append());

        row->set_value(0, info->index);
        row->set_value(1, info->name);

        if (app->pm->use_default_sink) {
            if (info->name == app->pm->server_info.default_source_name) {
                input_device->set_active(row);
            }
        } else {
            auto iter = input_device->get_active();

            if (iter) {
                if (info->name == app->pm->server_info.default_source_name) {
                    input_device->set_active(iter);
                }
            }
        }

        util::debug(log_tag + "added source: " + info->name);
    }
}

void ApplicationUi::on_source_removed(uint idx) {
    Gtk::TreeIter default_iter;
    Gtk::TreeIter remove_iter;
    std::string remove_name;

    auto children = source_list->children();

    for (auto c : children) {
        uint i;
        std::string name;

        c.get_value(0, i);
        c.get_value(1, name);

        if (idx == i) {
            remove_iter = c;
            remove_name = name;
        }

        if (name == app->pm->server_info.default_source_name) {
            default_iter = c;
        }
    }

    source_list->erase(remove_iter);

    util::debug(log_tag + "removed source: " + remove_name);

    // auto iter = input_device->get_active();
    //
    // if (!iter) {
    //     input_device->set_active(default_iter);
    // }
}

void ApplicationUi::on_use_default_sink_toggled() {
    if (use_default_sink->get_active()) {
        auto children = sink_list->children();

        for (auto c : children) {
            std::string name;

            c.get_value(1, name);

            if (name == app->pm->server_info.default_sink_name) {
                output_device->set_active(c);
            }
        }
    }
}

void ApplicationUi::on_use_default_source_toggled() {
    if (use_default_source->get_active()) {
        auto children = source_list->children();

        for (auto c : children) {
            std::string name;

            c.get_value(1, name);

            if (name == app->pm->server_info.default_source_name) {
                input_device->set_active(c);
            }
        }
    }
}

void ApplicationUi::on_input_device_changed() {
    Gtk::TreeModel::Row row = *(input_device->get_active());

    if (row) {
        uint index;
        std::string name;

        row.get_value(0, index);
        row.get_value(1, name);

        util::debug(log_tag + "input device changed: " + name);
    }
}

void ApplicationUi::on_output_device_changed() {
    Gtk::TreeModel::Row row = *(output_device->get_active());

    if (row) {
        uint index;
        std::string name;

        row.get_value(0, index);
        row.get_value(1, name);

        util::debug(log_tag + "output device changed: " + name);
    }
}
