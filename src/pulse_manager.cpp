#include <glibmm.h>
#include <functional>
#include "pulse_manager.hpp"
#include "util.hpp"

PulseManager::PulseManager()
    : context_ready(false),
      main_loop(pa_threaded_mainloop_new()),
      main_loop_api(pa_threaded_mainloop_get_api(main_loop)),
      context(pa_context_new(main_loop_api, "PulseEffects")) {
    pa_context_set_state_callback(context, &PulseManager::context_state_cb,
                                  this);

    pa_context_connect(context, nullptr, PA_CONTEXT_NOFAIL, nullptr);

    pa_threaded_mainloop_start(main_loop);

    while (!context_ready) {
    }
}

PulseManager::~PulseManager() {
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
                                    // this.pai.new_sink_input(info);
                                }
                            },
                            pm);
                    } else if (e == PA_SUBSCRIPTION_EVENT_CHANGE) {
                        pa_context_get_sink_input_info(
                            c, idx,
                            [](auto cx, auto info, auto eol, auto d) {
                                if (eol == 0 && info != nullptr) {
                                    // this.pai.changed_sink_input(info);
                                }
                            },
                            pm);
                    } else if (e == PA_SUBSCRIPTION_EVENT_REMOVE) {
                        Glib::signal_idle().connect([&]() {
                            util::debug(pm->log_tag +
                                        "removed si: " + std::to_string(idx));
                            return false;
                        });
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
                    util::critical(pm->log_tag +
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
