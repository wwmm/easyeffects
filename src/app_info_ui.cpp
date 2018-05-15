#include <glibmm/i18n.h>
#include <sstream>
#include "app_info_ui.hpp"
#include "util.hpp"

AppInfoUi::AppInfoUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& refBuilder,
                     const std::shared_ptr<AppInfo>& info,
                     const std::shared_ptr<PulseManager>& pulse_manager)
    : Gtk::Grid(cobject),
      app_info(info),
      builder(refBuilder),
      pm(pulse_manager) {
    // loading glade widgets

    builder->get_widget("enable", enable);
    builder->get_widget("app_icon", app_icon);
    builder->get_widget("app_name", app_name);
    builder->get_widget("volume", volume);
    builder->get_widget("mute", mute);
    builder->get_widget("mute_icon", mute_icon);
    builder->get_widget("format", format);
    builder->get_widget("rate", rate);
    builder->get_widget("channels", channels);
    builder->get_widget("resampler", resampler);
    builder->get_widget("buffer", buffer);
    builder->get_widget("latency", latency);
    builder->get_widget("state", state);
    builder->get_widget("level", level);

    init_widgets();
    connect_signals();

    if (app_info->app_type == "sink_input") {
        timeout_connection = Glib::signal_timeout().connect_seconds(
            [&]() {
                if (app_info != nullptr) {
                    pm->get_sink_input_info(app_info->index);
                }

                return running;
            },
            5);
    }
}

AppInfoUi::~AppInfoUi() {
    running = false;

    timeout_connection.disconnect();

    if (stream != nullptr) {
        auto o = pa_stream_flush(
            stream,
            [](auto s, auto success, auto ptr) {
                auto aiu = static_cast<AppInfoUi*>(ptr);

                if (success) {
                    util::debug(aiu->log_tag + aiu->app_info->name +
                                " level meter stream was flushed");

                    pa_stream_disconnect(s);
                } else {
                    util::debug(aiu->log_tag + "failed to flush " +
                                aiu->app_info->name + " level meter stream");
                }

                pa_threaded_mainloop_signal(aiu->pm->main_loop, false);
            },
            this);

        if (o != nullptr) {
            pm->wait_operation(o);

            while (stream != nullptr) {
            }
        } else {
            util::debug(log_tag + app_info->name +
                        " level meter stream does not need flushing");

            pa_stream_disconnect(stream);

            while (stream != nullptr) {
            }
        }
    }
}

std::unique_ptr<AppInfoUi> AppInfoUi::create(std::shared_ptr<AppInfo> app_info,
                                             std::shared_ptr<PulseManager> pm) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/app_info.glade");

    AppInfoUi* app_info_ui = nullptr;

    builder->get_widget_derived("widgets_grid", app_info_ui, app_info, pm);

    return std::unique_ptr<AppInfoUi>(app_info_ui);
}

std::string AppInfoUi::latency_to_str(uint value) {
    std::ostringstream msg;

    msg.precision(1);
    msg << std::fixed << value / 1000.0 << " ms";

    return msg.str();
}

void AppInfoUi::init_widgets() {
    enable->set_active(app_info->connected);

    app_icon->set_from_icon_name(app_info->icon_name, Gtk::ICON_SIZE_BUTTON);

    app_name->set_text(app_info->name);

    volume->set_value(app_info->volume);

    mute->set_active(app_info->mute);

    format->set_text(app_info->format);

    rate->set_text(std::to_string(app_info->rate) + " Hz");

    channels->set_text(std::to_string(app_info->channels));

    resampler->set_text(app_info->resampler);

    buffer->set_text(latency_to_str(app_info->buffer));

    latency->set_text(latency_to_str(app_info->latency));

    if (app_info->corked) {
        state->set_text(_("paused"));
    } else {
        state->set_text(_("playing"));
    }

    if (app_info->wants_to_play && stream == nullptr) {
        create_stream();
    } else if (!app_info->wants_to_play && stream != nullptr) {
        pa_stream_disconnect(stream);
    }
}

void AppInfoUi::connect_signals() {
    enable_connection = enable->signal_state_set().connect(
        sigc::mem_fun(*this, &AppInfoUi::on_enable_app), false);

    volume_connection = volume->signal_value_changed().connect(
        sigc::mem_fun(*this, &AppInfoUi::on_volume_changed));

    mute_connection = mute->signal_toggled().connect(
        sigc::mem_fun(*this, &AppInfoUi::on_mute));
}

void AppInfoUi::create_stream() {
    std::string source_name;

    if (app_info->app_type == "sink_input") {
        source_name = "PulseEffects_apps.monitor";

        stream = pm->create_stream(source_name.c_str(), app_info->index,
                                   app_info->name);

        pa_stream_set_monitor_stream(stream, app_info->index);
    } else {
        source_name = "PulseEffects_mic.monitor";

        stream = pm->create_stream(source_name.c_str(), app_info->index,
                                   app_info->name);
    }

    pa_stream_set_state_callback(
        stream,
        [](auto s, auto data) {
            auto aiu = static_cast<AppInfoUi*>(data);

            auto state = pa_stream_get_state(s);

            if (state == PA_STREAM_UNCONNECTED) {
                util::debug(aiu->log_tag + aiu->app_info->name +
                            " volume meter stream is unconnected");
            } else if (state == PA_STREAM_CREATING) {
                util::debug(aiu->log_tag + aiu->app_info->name +
                            " volume meter stream is being created");
            } else if (state == PA_STREAM_READY) {
                util::debug(aiu->log_tag + aiu->app_info->name +
                            " volume meter stream is ready");
            } else if (state == PA_STREAM_FAILED) {
                util::debug(aiu->log_tag + aiu->app_info->name +
                            " volume meter" +
                            " stream has failed. Did you disable this app?");

                pa_stream_disconnect(aiu->stream);
                pa_stream_unref(aiu->stream);
                aiu->stream = nullptr;
            } else if (state == PA_STREAM_TERMINATED) {
                util::debug(aiu->log_tag + aiu->app_info->name +
                            " volume meter stream was terminated");

                pa_stream_unref(aiu->stream);
                aiu->stream = nullptr;
            }
        },
        this);

    pa_stream_set_read_callback(
        stream,
        [](auto s, auto nbytes, auto class_ptr) {
            auto aiu = static_cast<AppInfoUi*>(class_ptr);
            const void* sdata;
            double v;

            if (pa_stream_peek(s, &sdata, &nbytes) < 0) {
                util::warning(aiu->log_tag + "Failed to read data from " +
                              aiu->app_info->name + " volume meter stream");
                return;
            }

            if (!sdata) {
                // taken from pavucontrol sources:
                /* NULL data means either a hole or empty buffer.
                 * Only drop the stream when there is a hole (length > 0) */
                if (nbytes)
                    pa_stream_drop(s);
                return;
            }

            if (nbytes > 0) {
                v = ((const float*)sdata)[nbytes / sizeof(float) - 1];

                pa_stream_drop(s);

                if (v < 0) {
                    v = 0;
                }
                if (v > 1) {
                    v = 1;
                }

                Glib::signal_idle().connect_once([aiu, v]() {
                    if (aiu != nullptr) {
                        if (aiu->running) {
                            aiu->level->set_value(v);
                        }
                    }
                });
            }
        },
        this);

    auto flags =
        (pa_stream_flags_t)(PA_STREAM_DONT_MOVE | PA_STREAM_PEAK_DETECT);

    if (pa_stream_connect_record(stream, source_name.c_str(), nullptr, flags) <
        0) {
        util::warning(log_tag + "failed to create level monitor stream " +
                      "for " + app_info->name);

        pa_stream_unref(stream);

        stream = nullptr;
    }
}

bool AppInfoUi::on_enable_app(bool state) {
    if (state) {
        if (app_info->app_type == "sink_input") {
            pm->move_sink_input_to_pulseeffects(app_info->index);
        } else {
            pm->move_source_output_to_pulseeffects(app_info->index);
        }
    } else {
        if (app_info->app_type == "sink_input") {
            pm->remove_sink_input_from_pulseeffects(app_info->index);
        } else {
            pm->remove_source_output_from_pulseeffects(app_info->index);
        }
    }

    return false;
}

void AppInfoUi::on_volume_changed() {
    auto value = volume->get_value();

    if (app_info->app_type == "sink_input") {
        pm->set_sink_input_volume(app_info->index, app_info->channels, value);
    } else {
        pm->set_source_output_volume(app_info->index, app_info->channels,
                                     value);
    }
}

void AppInfoUi::on_mute() {
    bool state = mute->get_active();

    if (state) {
        mute_icon->set_from_icon_name("audio-volume-muted-symbolic",
                                      Gtk::ICON_SIZE_BUTTON);

        volume->set_sensitive(false);
    } else {
        mute_icon->set_from_icon_name("audio-volume-high-symbolic",
                                      Gtk::ICON_SIZE_BUTTON);

        volume->set_sensitive(true);
    }

    if (app_info->app_type == "sink_input") {
        pm->set_sink_input_mute(app_info->index, state);
    } else {
        pm->set_source_output_mute(app_info->index, state);
    }
}

void AppInfoUi::update(std::shared_ptr<AppInfo> info) {
    app_info = info;

    enable_connection.disconnect();
    volume_connection.disconnect();
    mute_connection.disconnect();

    init_widgets();
    connect_signals();
}
