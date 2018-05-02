#ifndef PULSE_MANAGER_HPP
#define PULSE_MANAGER_HPP

#include <pulse/pulseaudio.h>
#include <pulse/thread-mainloop.h>
#include <sigc++/sigc++.h>
#include <iostream>
#include <memory>

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
    std::string name;
    uint index;
    std::string description;
    uint rate;
    std::string format;
};

class ParseAppInfo;

class PulseManager {
   public:
    PulseManager();
    ~PulseManager();

    bool use_default_sink = true;
    bool use_default_source = true;

    myServerInfo server_info;
    std::shared_ptr<mySinkInfo> apps_sink_info;
    std::shared_ptr<mySinkInfo> mic_sink_info;

    void find_sink_inputs();
    void find_source_outputs();
    void find_sinks();
    void find_sources();
    void move_sink_input_to_pulseeffects(uint idx);
    void remove_sink_input_from_pulseeffects(uint idx);
    void move_source_output_to_pulseeffects(uint idx);
    void remove_source_output_from_pulseeffects(uint idx);
    void set_sink_input_volume(uint idx, uint8_t channels, uint value);
    void set_sink_input_mute(uint idx, bool state);
    void set_source_output_volume(uint idx, uint8_t channels, uint value);
    void set_source_output_mute(uint idx, bool state);
    void get_sink_input_info(uint idx);
    void quit();

    sigc::signal<void, std::shared_ptr<mySourceInfo>> source_added;
    sigc::signal<void, uint> source_removed;
    sigc::signal<void, std::shared_ptr<mySinkInfo>> sink_added;
    sigc::signal<void, uint> sink_removed;
    sigc::signal<void, std::string> new_default_sink;
    sigc::signal<void, std::string> new_default_source;

   private:
    std::string log_tag = "pulse_manager.cpp: ";

    bool context_ready = false;

    pa_threaded_mainloop* main_loop;
    pa_mainloop_api* main_loop_api;
    pa_context* context;

    std::unique_ptr<ParseAppInfo> pai;

    static void context_state_cb(pa_context* ctx, void* data);

    void get_server_info();

    std::shared_ptr<mySinkInfo> get_sink_info(std::string name);

    std::shared_ptr<mySourceInfo> get_source_info(std::string name);

    std::shared_ptr<mySinkInfo> get_default_sink_info();

    std::shared_ptr<mySourceInfo> get_default_source_info();

    std::shared_ptr<mySinkInfo> load_sink(std::string name,
                                          std::string description,
                                          uint rate);

    void load_apps_sink();

    void load_mic_sink();

    void unload_module(uint idx);

    void unload_sinks();

    void drain_context();

    void wait_operation(pa_operation* o);
};

#endif
