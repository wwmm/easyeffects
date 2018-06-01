#ifndef PULSE_MANAGER_HPP
#define PULSE_MANAGER_HPP

#include <glibmm.h>
#include <pulse/pulseaudio.h>
#include <pulse/thread-mainloop.h>
#include <sigc++/sigc++.h>
#include <algorithm>
#include <array>
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

struct AppInfo {
    std::string app_type;
    uint index;
    std::string name;
    std::string icon_name;
    uint8_t channels;
    float volume;
    uint rate;
    std::string resampler;
    std::string format;
    int mute;
    bool connected;
    uint buffer;
    uint latency;
    int corked;
    bool wants_to_play;
};

class ParseAppInfo;

class PulseManager {
   public:
    PulseManager();
    ~PulseManager();

    bool use_default_sink = true;
    bool use_default_source = true;

    pa_threaded_mainloop* main_loop;

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

    sigc::signal<void, std::shared_ptr<mySourceInfo>> source_added;
    sigc::signal<void, uint> source_removed;
    sigc::signal<void, std::shared_ptr<mySinkInfo>> sink_added;
    sigc::signal<void, uint> sink_removed;
    sigc::signal<void, std::string> new_default_sink;
    sigc::signal<void, std::string> new_default_source;
    sigc::signal<void, std::shared_ptr<AppInfo>> sink_input_added;
    sigc::signal<void, std::shared_ptr<AppInfo>> sink_input_changed;
    sigc::signal<void, uint> sink_input_removed;
    sigc::signal<void, std::shared_ptr<AppInfo>> source_output_added;
    sigc::signal<void, std::shared_ptr<AppInfo>> source_output_changed;
    sigc::signal<void, uint> source_output_removed;

   private:
    std::string log_tag = "pulse_manager: ";

    bool context_ready = false;

    pa_mainloop_api* main_loop_api;
    pa_context* context;

    std::array<std::string, 10> blacklist_apps = {"PulseEffects",
                                                  "pulseeffects",
                                                  "PulseEffectsWebrtcProbe",
                                                  "gsd-media-keys",
                                                  "GNOME Shell",
                                                  "libcanberra",
                                                  "gnome-pomodoro",
                                                  "PulseAudio Volume Control",
                                                  "Screenshot",
                                                  "speech-dispatcher"};

    std::array<std::string, 5> blacklist_media_name = {
        "pulsesink probe", "bell-window-system", "audio-volume-change",
        "Peak detect", "screen-capture"};

    std::array<std::string, 1> blacklist_media_role = {"event"};

    static void context_state_cb(pa_context* ctx, void* data);

    void subscribe_to_events();

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

    void new_app(const pa_sink_input_info* info);

    void new_app(const pa_source_output_info* info);

    void changed_app(const pa_sink_input_info* info);

    void changed_app(const pa_source_output_info* info);

    void print_app_info(std::shared_ptr<AppInfo> info);

    bool app_is_connected(const pa_sink_input_info* info);

    bool app_is_connected(const pa_source_output_info* info);

    uint get_latency(const pa_sink_input_info* info) { return info->sink_usec; }

    uint get_latency(const pa_source_output_info* info) {
        return info->source_usec;
    }

    template <typename T>
    std::shared_ptr<AppInfo> parse_app_info(const T& info) {
        std::string app_name, media_name, media_role;
        auto ai = std::make_shared<AppInfo>();

        auto prop = pa_proplist_gets(info->proplist, "application.name");

        if (prop != nullptr) {
            app_name = prop;
        }

        prop = pa_proplist_gets(info->proplist, "media.name");

        if (prop != nullptr) {
            media_name = prop;
        }

        prop = pa_proplist_gets(info->proplist, "media.role");

        if (prop != nullptr) {
            media_role = prop;
        }

        auto forbidden_app =
            std::find(std::begin(blacklist_apps), std::end(blacklist_apps),
                      app_name) != std::end(blacklist_apps);

        auto forbidden_media_name =
            std::find(std::begin(blacklist_media_name),
                      std::end(blacklist_media_name),
                      media_name) != std::end(blacklist_media_name);

        auto forbidden_media_role =
            std::find(std::begin(blacklist_media_role),
                      std::end(blacklist_media_role),
                      media_role) != std::end(blacklist_media_role);

        if (forbidden_app || forbidden_media_name || forbidden_media_role) {
            return nullptr;
        } else {
            auto prop =
                pa_proplist_gets(info->proplist, "application.icon_name");

            std::string icon_name;

            if (prop != nullptr) {
                icon_name = prop;
            } else {
                icon_name = "audio-x-generic-symbolic";
            }

            ai->connected = app_is_connected(info);

            // linear volume
            ai->volume = 100 * pa_cvolume_max(&info->volume) / PA_VOLUME_NORM;

            if (info->resample_method) {
                ai->resampler = info->resample_method;
            } else {
                ai->resampler = "null";
            }

            ai->format = pa_sample_format_to_string(info->sample_spec.format);

            ai->index = info->index;
            ai->name = app_name;
            ai->icon_name = icon_name;
            ai->channels = info->volume.channels;
            ai->rate = info->sample_spec.rate;
            ai->mute = info->mute;
            ai->buffer = info->buffer_usec;
            ai->latency = get_latency(info);
            ai->corked = info->corked;
            ai->wants_to_play = (ai->connected && !ai->corked) ? true : false;

            return ai;
        }
    }
};

#endif
