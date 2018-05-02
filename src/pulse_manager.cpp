#include <glibmm.h>
#include <memory>
#include "parse_app_info.hpp"
#include "pulse_manager.hpp"
#include "util.hpp"

PulseManager::PulseManager()
    : main_loop(pa_threaded_mainloop_new()),
      main_loop_api(pa_threaded_mainloop_get_api(main_loop)),
      context(pa_context_new(main_loop_api, "PulseEffects")),
      pai(std::unique_ptr<ParseAppInfo>(new ParseAppInfo(this))) {
    pa_context_set_state_callback(context, &PulseManager::context_state_cb,
                                  this);

    pa_context_connect(context, nullptr, PA_CONTEXT_NOFAIL, nullptr);

    pa_threaded_mainloop_start(main_loop);

    while (!context_ready) {
    }

    get_server_info();
    load_apps_sink();
    load_mic_sink();
}

PulseManager::~PulseManager() {
    quit();
}

void PulseManager::quit() {
    unload_sinks();

    drain_context();

    util::debug(log_tag + "disconnecting Pulseaudio context");
    pa_context_disconnect(context);

    while (context_ready) {
    }

    util::debug(log_tag + "unreferencing Pulseaudio context");
    pa_context_unref(context);

    util::debug(log_tag + "stopping pulseaudio threaded main loop");
    pa_threaded_mainloop_stop(main_loop);

    util::debug(log_tag + "freeing Pulseaudio threaded main loop");
    pa_threaded_mainloop_free(main_loop);
}

void PulseManager::context_state_cb(pa_context* ctx, void* data) {
    auto pm = static_cast<PulseManager*>(data);

    auto state = pa_context_get_state(ctx);

    if (state == PA_CONTEXT_UNCONNECTED) {
        util::debug(pm->log_tag + "context is unconnected");
    } else if (state == PA_CONTEXT_CONNECTING) {
        util::debug(pm->log_tag + "context is connecting");
    } else if (state == PA_CONTEXT_AUTHORIZING) {
        util::debug(pm->log_tag + "context is authorizing");
    } else if (state == PA_CONTEXT_SETTING_NAME) {
        util::debug(pm->log_tag + "context is setting name");
    } else if (state == PA_CONTEXT_READY) {
        util::debug(pm->log_tag + "context is ready");
        util::debug(pm->log_tag +
                    "connected to: " + pa_context_get_server(ctx));
        util::debug(pm->log_tag + "protocol version: " +
                    std::to_string(pa_context_get_protocol_version(ctx)));

        pa_context_set_subscribe_callback(
            ctx,
            [](auto c, auto t, auto idx, auto d) {
                auto f = t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;

                auto pm = static_cast<PulseManager*>(d);

                if (f == PA_SUBSCRIPTION_EVENT_SINK_INPUT) {
                    auto e = t & PA_SUBSCRIPTION_EVENT_TYPE_MASK;

                    if (e == PA_SUBSCRIPTION_EVENT_NEW) {
                        pa_context_get_sink_input_info(
                            c, idx,
                            [](auto cx, auto info, auto eol, auto d) {
                                if (eol == 0 && info != nullptr) {
                                    auto pm = static_cast<PulseManager*>(d);

                                    pm->pai->new_app(info);
                                }
                            },
                            pm);
                    } else if (e == PA_SUBSCRIPTION_EVENT_CHANGE) {
                        pa_context_get_sink_input_info(
                            c, idx,
                            [](auto cx, auto info, auto eol, auto d) {
                                if (eol == 0 && info != nullptr) {
                                    auto pm = static_cast<PulseManager*>(d);

                                    pm->pai->changed_app(info);
                                }
                            },
                            pm);
                    } else if (e == PA_SUBSCRIPTION_EVENT_REMOVE) {
                        Glib::signal_idle().connect([pm, idx]() {
                            util::debug(pm->log_tag +
                                        "removed si: " + std::to_string(idx));
                            return false;
                        });
                    }
                } else if (f == PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT) {
                    auto e = t & PA_SUBSCRIPTION_EVENT_TYPE_MASK;

                    if (e == PA_SUBSCRIPTION_EVENT_NEW) {
                        pa_context_get_source_output_info(
                            c, idx,
                            [](auto cx, auto info, auto eol, auto d) {
                                if (eol == 0 && info != nullptr) {
                                    auto pm = static_cast<PulseManager*>(d);

                                    pm->pai->new_app(info);
                                }
                            },
                            pm);
                    } else if (e == PA_SUBSCRIPTION_EVENT_CHANGE) {
                        pa_context_get_source_output_info(
                            c, idx,
                            [](auto cx, auto info, auto eol, auto d) {
                                if (eol == 0 && info != nullptr) {
                                    auto pm = static_cast<PulseManager*>(d);

                                    pm->pai->changed_app(info);
                                }
                            },
                            pm);
                    } else if (e == PA_SUBSCRIPTION_EVENT_REMOVE) {
                        Glib::signal_idle().connect([pm, idx]() {
                            util::debug(pm->log_tag +
                                        "removed so: " + std::to_string(idx));
                            return false;
                        });
                    }
                } else if (f == PA_SUBSCRIPTION_EVENT_SOURCE) {
                    auto e = t & PA_SUBSCRIPTION_EVENT_TYPE_MASK;

                    if (e == PA_SUBSCRIPTION_EVENT_NEW) {
                        pa_context_get_source_info_by_index(
                            c, idx,
                            [](auto cx, auto info, auto eol, auto d) {
                                if (eol == 0 && info != nullptr) {
                                    std::string s1 =
                                        "PulseEffects_apps.monitor";
                                    std::string s2 = "PulseEffects_mic.monitor";

                                    if (info->name != s1 && info->name != s2) {
                                        auto pm = static_cast<PulseManager*>(d);

                                        auto si =
                                            std::make_shared<mySourceInfo>();

                                        si->name = info->name;
                                        si->index = info->index;
                                        si->description = info->description;
                                        si->rate = info->sample_spec.rate;
                                        si->format = pa_sample_format_to_string(
                                            info->sample_spec.format);

                                        Glib::signal_idle().connect(
                                            [pm, si = move(si)] {
                                                pm->source_added.emit(move(si));
                                                return false;
                                            });
                                    }
                                }
                            },
                            pm);
                    } else if (e == PA_SUBSCRIPTION_EVENT_CHANGE) {
                    } else if (e == PA_SUBSCRIPTION_EVENT_REMOVE) {
                        Glib::signal_idle().connect([pm, idx]() {
                            pm->source_removed.emit(idx);
                            return false;
                        });
                    }
                } else if (f == PA_SUBSCRIPTION_EVENT_SINK) {
                    auto e = t & PA_SUBSCRIPTION_EVENT_TYPE_MASK;

                    if (e == PA_SUBSCRIPTION_EVENT_NEW) {
                        pa_context_get_sink_info_by_index(
                            c, idx,
                            [](auto cx, auto info, auto eol, auto d) {
                                if (eol == 0 && info != nullptr) {
                                    std::string s1 = "PulseEffects_apps";
                                    std::string s2 = "PulseEffects_mic";

                                    if (info->name != s1 && info->name != s2) {
                                        auto pm = static_cast<PulseManager*>(d);

                                        auto si =
                                            std::make_shared<mySinkInfo>();

                                        si->name = info->name;
                                        si->index = info->index;
                                        si->description = info->description;
                                        si->rate = info->sample_spec.rate;
                                        si->format = pa_sample_format_to_string(
                                            info->sample_spec.format);

                                        Glib::signal_idle().connect(
                                            [pm, si = move(si)] {
                                                pm->sink_added.emit(move(si));
                                                return false;
                                            });
                                    }
                                }
                            },
                            pm);
                    } else if (e == PA_SUBSCRIPTION_EVENT_CHANGE) {
                    } else if (e == PA_SUBSCRIPTION_EVENT_REMOVE) {
                        Glib::signal_idle().connect([pm, idx]() {
                            pm->sink_removed.emit(idx);
                            return false;
                        });
                    }
                } else if (f == PA_SUBSCRIPTION_EVENT_SERVER) {
                    auto e = t & PA_SUBSCRIPTION_EVENT_TYPE_MASK;

                    if (e == PA_SUBSCRIPTION_EVENT_CHANGE) {
                        pa_context_get_server_info(
                            c,
                            [](auto cx, auto info, auto d) {
                                if (info != nullptr) {
                                    auto pm = static_cast<PulseManager*>(d);

                                    pm->server_info.server_name =
                                        info->server_name;
                                    pm->server_info.server_version =
                                        info->server_version;

                                    auto sink = info->default_sink_name;
                                    auto source = info->default_source_name;

                                    pm->server_info.default_sink_name = sink;
                                    pm->server_info.default_source_name =
                                        source;

                                    if (sink !=
                                            std::string("PulseEffects_apps") &&
                                        pm->use_default_sink) {
                                        Glib::signal_idle().connect(
                                            [pm, sink]() {
                                                pm->new_default_sink.emit(sink);
                                                return false;
                                            });
                                    }

                                    if (source !=
                                            std::string(
                                                "PulseEffects_mic.monitor") &&
                                        pm->use_default_source) {
                                        Glib::signal_idle().connect([pm,
                                                                     source]() {
                                            pm->new_default_source.emit(source);
                                            return false;
                                        });
                                    }
                                }
                            },
                            pm);
                    }
                }
            },
            pm);

        auto mask = static_cast<pa_subscription_mask_t>(
            PA_SUBSCRIPTION_MASK_SINK_INPUT |
            PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT | PA_SUBSCRIPTION_MASK_SOURCE |
            PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SERVER);

        pa_context_subscribe(
            ctx, mask,
            [](auto c, auto success, auto d) {
                auto pm = static_cast<PulseManager*>(d);

                if (success == 0) {
                    util::error(pm->log_tag +
                                "context event subscribe failed!");
                }
            },
            pm);

        pm->context_ready = true;
    } else if (state == PA_CONTEXT_FAILED) {
        util::debug(pm->log_tag + "failed to connect context");
    } else if (state == PA_CONTEXT_TERMINATED) {
        util::debug(pm->log_tag + "context was terminated");

        pm->context_ready = false;
    }
}

void PulseManager::get_server_info() {
    auto o = pa_context_get_server_info(
        context,
        [](auto c, auto info, auto d) {
            if (info != nullptr) {
                auto pm = static_cast<PulseManager*>(d);

                pm->server_info.server_name = info->server_name;
                pm->server_info.server_version = info->server_version;
                pm->server_info.default_sink_name = info->default_sink_name;
                pm->server_info.default_source_name = info->default_source_name;

                util::debug(pm->log_tag +
                            "Pulseaudio version: " + info->server_version);
                util::debug(pm->log_tag + "default pulseaudio source: " +
                            info->default_source_name);
                util::debug(pm->log_tag + "default pulseaudio sink: " +
                            info->default_sink_name);

                pa_threaded_mainloop_signal(pm->main_loop, false);
            }
        },
        this);

    wait_operation(o);
}

std::shared_ptr<mySinkInfo> PulseManager::get_sink_info(std::string name) {
    auto si = std::make_shared<mySinkInfo>();

    struct Data {
        bool failed;
        PulseManager* pm;
        std::shared_ptr<mySinkInfo> si;
    };

    Data data = {false, this, si};

    auto o = pa_context_get_sink_info_by_name(
        context, name.c_str(),
        [](auto c, auto info, auto eol, auto data) {
            auto d = static_cast<Data*>(data);

            if (eol == -1) {
                d->failed = true;

                pa_threaded_mainloop_signal(d->pm->main_loop, false);
            } else if (eol == 0 && info != nullptr) {
                d->si->name = info->name;
                d->si->index = info->index;
                d->si->description = info->description;
                d->si->owner_module = info->owner_module;
                d->si->monitor_source = info->monitor_source;
                d->si->monitor_source_name = info->monitor_source_name;
                d->si->rate = info->sample_spec.rate;
                d->si->format =
                    pa_sample_format_to_string(info->sample_spec.format);
            } else if (eol == 1) {
                pa_threaded_mainloop_signal(d->pm->main_loop, false);
            }
        },
        &data);

    wait_operation(o);

    if (!data.failed) {
        return si;
    } else {
        return nullptr;
    }
}

std::shared_ptr<mySourceInfo> PulseManager::get_source_info(std::string name) {
    auto si = std::make_shared<mySourceInfo>();

    struct Data {
        bool failed;
        PulseManager* pm;
        std::shared_ptr<mySourceInfo> si;
    };

    Data data = {false, this, si};

    auto o = pa_context_get_source_info_by_name(
        context, name.c_str(),
        [](auto c, auto info, auto eol, auto data) {
            auto d = static_cast<Data*>(data);

            if (eol == -1) {
                d->failed = true;

                pa_threaded_mainloop_signal(d->pm->main_loop, false);
            } else if (eol == 0 && info != nullptr) {
                d->si->name = info->name;
                d->si->index = info->index;
                d->si->description = info->description;
                d->si->rate = info->sample_spec.rate;
                d->si->format =
                    pa_sample_format_to_string(info->sample_spec.format);
            } else if (eol == 1) {
                pa_threaded_mainloop_signal(d->pm->main_loop, false);
            }
        },
        &data);

    wait_operation(o);

    if (!data.failed) {
        return si;
    } else {
        return nullptr;
    }
}

std::shared_ptr<mySinkInfo> PulseManager::get_default_sink_info() {
    auto info = get_sink_info(server_info.default_sink_name);

    if (info != nullptr) {
        util::debug(log_tag + "default pulseaudio sink sampling rate: " +
                    std::to_string(info->rate) + " Hz");
        util::debug(log_tag +
                    "default pulseaudio sink audio format: " + info->format);

        return info;
    } else {
        util::error(log_tag + "could not get default sink info");

        return nullptr;
    }
}

std::shared_ptr<mySourceInfo> PulseManager::get_default_source_info() {
    auto info = get_source_info(server_info.default_source_name);

    if (info != nullptr) {
        util::debug(log_tag + "default pulseaudio source sampling rate: " +
                    std::to_string(info->rate) + " Hz");
        util::debug(log_tag +
                    "default pulseaudio source audio format: " + info->format);

        return info;
    } else {
        util::error(log_tag + "could not get default source info");

        return nullptr;
    }
}

std::shared_ptr<mySinkInfo> PulseManager::load_sink(std::string name,
                                                    std::string description,
                                                    uint rate) {
    auto si = get_sink_info(name);

    if (si == nullptr) {  // sink is not loaded
        std::string argument = "sink_name=" + name + " " +
                               "sink_properties=" + description +
                               "device.class=\"sound\"" + " " + "channels=2" +
                               " " + "rate=" + std::to_string(rate);

        auto o = pa_context_load_module(
            context, "module-null-sink", argument.c_str(),
            [](auto c, auto idx, auto d) {
                auto pm = static_cast<PulseManager*>(d);

                util::debug(pm->log_tag + "sink loaded");

                pa_threaded_mainloop_signal(pm->main_loop, false);
            },
            this);

        wait_operation(o);

        // now that the sink is loaded we get its info
        si = get_sink_info(name);
    }

    return si;
}

void PulseManager::load_apps_sink() {
    util::debug(log_tag + "loading Pulseeffects applications output sink...");

    auto info = get_default_sink_info();

    if (info != nullptr) {
        std::string name = "PulseEffects_apps";
        std::string description = "device.description=\"PulseEffects(apps)\"";
        auto rate = info->rate;

        apps_sink_info = load_sink(name, description, rate);
    }
}

void PulseManager::load_mic_sink() {
    util::debug(log_tag + "loading Pulseeffects microphone output sink...");

    auto info = get_default_source_info();

    if (info != nullptr) {
        std::string name = "PulseEffects_mic";
        std::string description = "device.description=\"PulseEffects(mic)\"";
        auto rate = info->rate;

        mic_sink_info = load_sink(name, description, rate);
    }
}

void PulseManager::find_sink_inputs() {
    pa_context_get_sink_input_info_list(
        context,
        [](auto c, auto info, auto eol, auto d) {
            if (eol == 0 && info != nullptr) {
                auto pm = static_cast<PulseManager*>(d);

                pm->pai->new_app(info);
            }
        },
        this);
}

void PulseManager::find_source_outputs() {
    pa_context_get_source_output_info_list(
        context,
        [](auto c, auto info, auto eol, auto d) {
            if (eol == 0 && info != nullptr) {
                auto pm = static_cast<PulseManager*>(d);

                pm->pai->new_app(info);
            }
        },
        this);
}

void PulseManager::find_sinks() {
    pa_context_get_sink_info_list(
        context,
        [](auto c, auto info, auto eol, auto d) {
            if (eol == 0 && info != nullptr) {
                std::string s1 = "PulseEffects_apps";
                std::string s2 = "PulseEffects_mic";

                if (info->name != s1 && info->name != s2) {
                    auto pm = static_cast<PulseManager*>(d);

                    auto si = std::make_shared<mySinkInfo>();

                    si->name = info->name;
                    si->index = info->index;
                    si->description = info->description;
                    si->rate = info->sample_spec.rate;
                    si->format =
                        pa_sample_format_to_string(info->sample_spec.format);

                    Glib::signal_idle().connect([pm, si = move(si)] {
                        pm->sink_added.emit(move(si));
                        return false;
                    });
                }
            }
        },
        this);
}

void PulseManager::find_sources() {
    pa_context_get_source_info_list(
        context,
        [](auto c, auto info, auto eol, auto d) {
            if (eol == 0 && info != nullptr) {
                std::string s1 = "PulseEffects_apps.monitor";
                std::string s2 = "PulseEffects_mic.monitor";

                if (info->name != s1 && info->name != s2) {
                    auto pm = static_cast<PulseManager*>(d);

                    auto si = std::make_shared<mySourceInfo>();

                    si->name = info->name;
                    si->index = info->index;
                    si->description = info->description;
                    si->rate = info->sample_spec.rate;
                    si->format =
                        pa_sample_format_to_string(info->sample_spec.format);

                    Glib::signal_idle().connect([pm, si = move(si)] {
                        pm->source_added.emit(move(si));
                        return false;
                    });
                }
            }
        },
        this);
}

void PulseManager::unload_module(uint idx) {
    struct Data {
        uint idx;
        PulseManager* pm;
    };

    Data data = {idx, this};

    auto o = pa_context_unload_module(
        context, idx,
        [](auto c, auto success, auto data) {
            auto d = static_cast<Data*>(data);

            if (success) {
                util::debug(d->pm->log_tag + "module " +
                            std::to_string(d->idx) + " unloaded");
            } else {
                util::debug(d->pm->log_tag + "failed to unload module " +
                            std::to_string(d->idx));
            }

            pa_threaded_mainloop_signal(d->pm->main_loop, false);
        },
        &data);

    wait_operation(o);
}

void PulseManager::unload_sinks() {
    util::debug(log_tag + "unloading PulseEffects sinks...");

    unload_module(apps_sink_info->owner_module);
    unload_module(mic_sink_info->owner_module);
}

void PulseManager::drain_context() {
    auto o = pa_context_drain(
        context,
        [](auto c, auto d) {
            auto pm = static_cast<PulseManager*>(d);

            if (pa_context_get_state(c) == PA_CONTEXT_READY) {
                pa_threaded_mainloop_signal(pm->main_loop, false);
            }
        },
        this);

    if (o != nullptr) {
        wait_operation(o);

        util::debug(log_tag + "Context was drained");
    } else {
        util::debug(log_tag + "Context did not need draining");
    }
}

void PulseManager::wait_operation(pa_operation* o) {
    pa_threaded_mainloop_lock(main_loop);

    while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(main_loop);
    }

    pa_threaded_mainloop_unlock(main_loop);
}
