#ifndef PARSE_APP_INFO_HPP
#define PARSE_APP_INFO_HPP

#include <glibmm.h>
#include <pulse/pulseaudio.h>
#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>
#include "pulse_manager.hpp"

struct AppInfo {
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
};

class PulseManager;

class ParseAppInfo {
   public:
    ParseAppInfo(PulseManager* manager) : pm(manager){};

    template <typename T>
    void new_app(const T& info) {
        auto app_info = parse_app_info(info);

        if (typeid(T) == typeid(pa_sink_input_info)) {
        } else if (typeid(T) == typeid(pa_source_output_info)) {
        }
    }

    template <typename T>
    void changed_app(const T& info) {
        auto app_info = parse_app_info(info);

        if (typeid(T) == typeid(pa_sink_input_info)) {
        } else if (typeid(T) == typeid(pa_source_output_info)) {
        }
    }

    void print_info(std::unique_ptr<AppInfo> info) {
        std::cout << info->index << std::endl;
        std::cout << info->name << std::endl;
        std::cout << info->icon_name << std::endl;
        std::cout << info->channels << std::endl;
        std::cout << info->volume << std::endl;
        std::cout << info->rate << std::endl;
        std::cout << info->resampler << std::endl;
        std::cout << info->format << std::endl;
    }

   private:
    PulseManager* pm;

    std::vector<std::string> blacklist_apps = {"PulseEffects",
                                               "pulseeffects",
                                               "PulseEffectsWebrtcProbe",
                                               "gsd-media-keys",
                                               "GNOME Shell",
                                               "libcanberra",
                                               "gnome-pomodoro",
                                               "PulseAudio Volume Control",
                                               "Screenshot",
                                               "speech-dispatcher"};
    std::vector<std::string> blacklist_media_name = {
        "pulsesink probe", "bell-window-system", "audio-volume-change",
        "Peak detect", "screen-capture"};
    std::vector<std::string> blacklist_media_role = {"event"};

    bool is_connected(const pa_sink_input_info* info) {
        if (info->sink == pm->apps_sink_info.index) {
            return true;
        } else {
            return false;
        }
    }

    bool is_connected(const pa_source_output_info* info) {
        if (info->source == pm->mic_sink_info.index) {
            return true;
        } else {
            return false;
        }
    }

    uint get_latency(const pa_sink_input_info* info) { return info->sink_usec; }

    uint get_latency(const pa_source_output_info* info) {
        return info->source_usec;
    }

    template <typename T>
    std::unique_ptr<AppInfo> parse_app_info(const T& info) {
        std::string app_name, media_name, media_role;
        auto ai = std::make_unique<AppInfo>();

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

            ai->connected = is_connected(info);

            // todo: volume section

            if (info->resample_method) {
                ai->resampler = info->resample_method;
            } else {
                ai->resampler = "null";
            }

            ai->index = info->index;
            ai->name = app_name;
            ai->icon_name = icon_name;
            ai->channels = info->volume.channels;
            ai->rate = info->sample_spec.rate;
            ai->mute = info->mute;
            ai->buffer = info->buffer_usec;
            ai->latency = get_latency(info);
            ai->corked = info->corked;

            print_info(move(ai));

            return ai;
        }
    }
};

#endif
