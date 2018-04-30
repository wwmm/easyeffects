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

void PulseManager::context_state_cb(pa_context* context, void* data) {
    auto pm = static_cast<PulseManager*>(data);

    auto state = pa_context_get_state(context);

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

        pm->context_ready = true;
    } else if (state == PA_CONTEXT_FAILED) {
        util::debug(pm->log_tag + "failed to connect context");
    } else if (state == PA_CONTEXT_TERMINATED) {
        util::debug(pm->log_tag + "context was terminated");

        pm->context_ready = false;
    }
}
