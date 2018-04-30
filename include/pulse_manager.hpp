#ifndef PULSE_MANAGER_HPP
#define PULSE_MANAGER_HPP

#include <pulse/pulseaudio.h>
#include <pulse/thread-mainloop.h>
#include <iostream>

struct myServerInfo {
    std::string server_name;
    std::string server_version;
    std::string default_sink_name;
    std::string default_source_name;
};

struct mySinkInfo {
    std::string name;
    uint index;
    std::string description;
    uint owner_module;
    uint monitor_source;
    std::string monitor_source_name;
    uint rate;
    std::string format;
};

struct mySourceInfo {
    uint index;
    std::string description;
    uint rate;
    std::string format;
};

class PulseManager {
   public:
    PulseManager();
    ~PulseManager();

    myServerInfo server_info;
    mySinkInfo apps_sink_info;
    mySinkInfo mic_sink_info;

   private:
    std::string log_tag = "pulse_manager.cpp: ";

    bool context_ready;

    pa_threaded_mainloop* main_loop;
    pa_mainloop_api* main_loop_api;
    pa_context* context;

    static void context_state_cb(pa_context* ctx, void* data);

    void get_server_info();

    void wait_operation(pa_operation* o);
};

#endif
