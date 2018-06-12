#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/listboxrow.h>
#include <gtkmm/settings.h>
#include <boost/filesystem.hpp>
#include "application_ui.hpp"
#include "util.hpp"

ApplicationUi::ApplicationUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& builder,
                             Application* application)
    : Gtk::ApplicationWindow(cobject),
      app(application),
      settings(app->settings),
      sie_ui(SinkInputEffectsUi::create(app->sie)),
      soe_ui(SourceOutputEffectsUi::create(app->soe)) {
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
    builder->get_widget("presets_listbox", presets_listbox);
    builder->get_widget("presets_menu_button", presets_menu_button);
    builder->get_widget("presets_menu_label", presets_menu_label);
    builder->get_widget("preset_name", preset_name);
    builder->get_widget("add_preset", add_preset);
    builder->get_widget("import_preset", import_preset);
    builder->get_widget("use_custom_color", use_custom_color);
    builder->get_widget("spectrum_color_button", spectrum_color_button);
    builder->get_widget("calibration_button", calibration_button);

    get_object(builder, "buffer_in", buffer_in);
    get_object(builder, "buffer_out", buffer_out);
    get_object(builder, "latency_in", latency_in);
    get_object(builder, "latency_out", latency_out);
    get_object(builder, "spectrum_n_points", spectrum_n_points);
    get_object(builder, "sink_list", sink_list);
    get_object(builder, "source_list", source_list);

    // signals connection

    enable_autostart->signal_state_set().connect(
        sigc::mem_fun(*this, &ApplicationUi::on_enable_autostart), false);

    reset_settings->signal_clicked().connect(
        sigc::mem_fun(*this, &ApplicationUi::on_reset_settings));

    // spectrum

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

    spectrum_color_button->signal_color_set().connect([=]() {
        spectrum_color = spectrum_color_button->get_rgba();

        auto v = Glib::Variant<std::vector<double>>::create(std::vector<double>{
            spectrum_color.get_red(), spectrum_color.get_green(),
            spectrum_color.get_blue(), spectrum_color.get_alpha()});

        settings->set_value("spectrum-color", v);
    });

    use_custom_color->signal_state_set().connect(
        sigc::mem_fun(*this, &ApplicationUi::on_use_custom_color), false);

    // pulseaudio device selection

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

    // presets widgets callbacks

    presets_menu_button->signal_clicked().connect(
        sigc::mem_fun(*this, &ApplicationUi::on_presets_menu_button_clicked));

    presets_listbox->set_sort_func(
        sigc::mem_fun(*this, &ApplicationUi::on_listbox_sort));

    presets_listbox->signal_row_activated().connect([&](auto row) {
        presets_menu_label->set_text(row->get_name());
        app->presets_manager->load(row->get_name());
    });

    add_preset->signal_clicked().connect([=]() {
        app->presets_manager->add(preset_name->get_text());
        preset_name->set_text("");
        populate_presets_listbox();
    });

    import_preset->signal_clicked().connect(
        sigc::mem_fun(*this, &ApplicationUi::on_import_preset_clicked));

    // calibration

    calibration_button->signal_clicked().connect([=]() {
        auto calibration_ui = CalibrationUi::create();

        auto c = app->pm->new_default_source.connect(
            [=](auto name) { calibration_ui->set_source_monitor_name(name); });

        calibration_ui->signal_hide().connect([calibration_ui, c]() {
            c->disconnect();
            delete calibration_ui;
        });

        calibration_ui->show_all();
    });

    // pulseaudio signals

    app->pm->sink_added.connect(
        sigc::mem_fun(*this, &ApplicationUi::on_sink_added));
    app->pm->sink_removed.connect(
        sigc::mem_fun(*this, &ApplicationUi::on_sink_removed));
    app->pm->source_added.connect(
        sigc::mem_fun(*this, &ApplicationUi::on_source_added));
    app->pm->source_removed.connect(
        sigc::mem_fun(*this, &ApplicationUi::on_source_removed));

    // sink inputs interface

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

    settings->bind("buffer-out", buffer_out.get(), "value", flag);
    settings->bind("latency-out", latency_out.get(), "value", flag);

    settings->bind("buffer-in", buffer_in.get(), "value", flag);
    settings->bind("latency-in", latency_in.get(), "value", flag);

    settings->bind("show-spectrum", show_spectrum, "active", flag);
    settings->bind("show-spectrum", spectrum_box, "visible", flag_get);
    settings->bind("spectrum-n-points", spectrum_n_points.get(), "value", flag);
    settings->bind("use-custom-color", use_custom_color, "active", flag);
    settings->bind("use-custom-color", spectrum_color_button, "sensitive",
                   flag);

    init_autostart_switch();
}

ApplicationUi::~ApplicationUi() {
    app->sie->disable_spectrum();
    app->soe->disable_spectrum();

    for (auto c : connections) {
        c.disconnect();
    }

    spectrum_connection.disconnect();

    delete sie_ui;
    delete soe_ui;
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
    namespace fs = boost::filesystem;

    fs::path autostart_dir{Glib::get_user_config_dir() + "/autostart"};

    if (!fs::is_directory(autostart_dir)) {
        fs::create_directories(autostart_dir);
    }

    fs::path autostart_file{Glib::get_user_config_dir() +
                            "/autostart/pulseeffects-service.desktop"};

    if (state) {
        if (!fs::exists(autostart_file)) {
            fs::ofstream ofs{autostart_file};

            ofs << "[Desktop Entry]\n";
            ofs << "Name=PulseEffects\n";
            ofs << "Comment=PulseEffects Service\n";
            ofs << "Exec=pulseeffects --gapplication-service\n";
            ofs << "Icon=pulseeffects\n";
            ofs << "StartupNotify=false\n";
            ofs << "Terminal=false\n";
            ofs << "Type=Application\n";

            ofs.close();

            util::debug(log_tag + "autostart file created");
        }
    } else {
        if (fs::exists(autostart_file)) {
            fs::remove(autostart_file);

            util::debug(log_tag + "autostart file removed");
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

bool ApplicationUi::on_use_custom_color(bool state) {
    if (state) {
        Glib::Variant<std::vector<double>> v;

        settings->get_value("spectrum-color", v);

        auto rgba = v.get();

        spectrum_color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);

        spectrum_color_button->set_rgba(spectrum_color);
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
        auto n_bars = spectrum_mag.size();
        auto x = util::linspace(0, width, n_bars);

        for (uint n = 0; n < n_bars; n++) {
            auto bar_height = spectrum_mag[n] * height;

            ctx->rectangle(x[n], height - bar_height, width / n_bars,
                           bar_height);
        }

        if (settings->get_boolean("use-custom-color")) {
            ctx->set_source_rgba(
                spectrum_color.get_red(), spectrum_color.get_green(),
                spectrum_color.get_blue(), spectrum_color.get_alpha());
        } else {
            auto color = Gdk::RGBA();
            auto style_ctx = spectrum->get_style_context();

            style_ctx->lookup_color("theme_selected_bg_color", color);

            ctx->set_source_rgba(color.get_red(), color.get_green(),
                                 color.get_blue(), 1.0);
        }

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

    spectrum->queue_draw();

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

        app->soe->set_source_monitor_name(name);

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

        app->sie->set_output_sink_name(name);
        app->soe->webrtc->set_probe_src_device(name + ".monitor");

        util::debug(log_tag + "output device changed: " + name);
    }
}

int ApplicationUi::on_listbox_sort(Gtk::ListBoxRow* row1,
                                   Gtk::ListBoxRow* row2) {
    auto name1 = row1->get_name();
    auto name2 = row2->get_name();

    std::vector<std::string> names = {name1, name2};

    std::sort(names.begin(), names.end());

    if (name1 == names[0]) {
        return -1;
    } else if (name2 == names[0]) {
        return 1;
    } else {
        return 0;
    }
}

void ApplicationUi::on_presets_menu_button_clicked() {
    populate_presets_listbox();
}

void ApplicationUi::on_import_preset_clicked() {
    // gtkmm 3.22 does not have FileChooseNative so we have to use C api :-(

    GtkFileChooserNative* native;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    native = gtk_file_chooser_native_new(_("Import Presets"),
                                         (GtkWindow*)this->gobj(), action,
                                         _("Open"), _("Cancel"));

    res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(native));

    if (res == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser* chooser = GTK_FILE_CHOOSER(native);

        auto file_list = gtk_file_chooser_get_filenames(chooser);

        g_slist_foreach(file_list,
                        [](auto data, auto user_data) {
                            auto aui = static_cast<ApplicationUi*>(user_data);

                            auto file_path = static_cast<char*>(data);

                            aui->app->presets_manager->import(file_path);
                        },
                        this);

        g_slist_free(file_list);
    }

    g_object_unref(native);

    populate_presets_listbox();
}

void ApplicationUi::populate_presets_listbox() {
    auto children = presets_listbox->get_children();

    for (auto c : children) {
        presets_listbox->remove(*c);
    }

    auto names = app->presets_manager->get_names();

    for (auto name : names) {
        auto b = Gtk::Builder::create_from_resource(
            "/com/github/wwmm/pulseeffects/preset_row.glade");

        Gtk::ListBoxRow* row;
        Gtk::Button *save_btn, *remove_btn;
        Gtk::Label* label;

        b->get_widget("preset_row", row);
        b->get_widget("save", save_btn);
        b->get_widget("remove", remove_btn);
        b->get_widget("name", label);

        row->set_name(name);
        label->set_text(name);

        connections.push_back(save_btn->signal_clicked().connect(
            [=]() { app->presets_manager->save(name); }));
        connections.push_back(remove_btn->signal_clicked().connect([=]() {
            app->presets_manager->remove(name);
            populate_presets_listbox();
        }));

        presets_listbox->add(*row);
        presets_listbox->show_all();
    }
}
