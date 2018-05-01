#ifndef PARSE_APP_INFO_HPP
#define PARSE_APP_INFO_HPP

#include <glibmm.h>
#include <pulse/pulseaudio.h>
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
    void new_app(T info) {
        auto app_info = parse_app_info(info);

        if (typeid(T) == typeid(pa_sink_input_info)) {
        } else if (typeid(T) == typeid(pa_source_output_info)) {
        }
    }

    template <typename T>
    void changed_app(T info) {
        auto app_info = parse_app_info(info);

        if (typeid(T) == typeid(pa_sink_input_info)) {
        } else if (typeid(T) == typeid(pa_source_output_info)) {
        }
    }

   private:
    PulseManager* pm;

    std::vector<std::string> blacklist_apps;
    std::vector<std::string> blacklist_media_name;
    std::vector<std::string> blacklist_media_role;

    template <typename T>
    AppInfo parse_app_info(T info) {
        auto ai = AppInfo();

        auto proplist = info->proplist;

        std::string app_name = pa_proplist_gets(proplist, "application.name");
        std::string media_name = pa_proplist_gets(proplist, "media.name");
        std::string media_role = pa_proplist_gets(proplist, "media.role");

        for (auto a : blacklist_apps) {
            if (a == app_name) {
                // return;
            }
        }

        if (typeid(T) == typeid(pa_sink_input_info)) {
        } else if (typeid(T) == typeid(pa_source_output_info)) {
        }

        return ai;
    }
};

#endif
