#include <glibmm/main.h>
#include <gst/insertbin/gstinsertbin.h>
#include "stereo_enhancer.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, StereoEnhancer* l) {
    auto enable = g_settings_get_boolean(settings, key);
    auto plugin = gst_bin_get_by_name(GST_BIN(l->plugin), "stereo_enhancer");

    if (enable) {
        if (!plugin) {
            gst_insert_bin_append(
                GST_INSERT_BIN(l->plugin), l->stereo_enhancer,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<StereoEnhancer*>(d);

                    if (success) {
                        util::debug(l->log_tag + "stereo_enhancer enabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to enable the stereo_enhancer");
                    }
                },
                l);
        }
    } else {
        if (plugin) {
            gst_insert_bin_remove(
                GST_INSERT_BIN(l->plugin), l->stereo_enhancer,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<StereoEnhancer*>(d);

                    if (success) {
                        util::debug(l->log_tag + "stereo_enhancer disabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to disable the stereo_enhancer");
                    }
                },
                l);
        }
    }
}

void on_post_messages_changed(GSettings* settings,
                              gchar* key,
                              StereoEnhancer* l) {
    auto post = g_settings_get_boolean(settings, key);

    if (post) {
        l->input_level_connection = Glib::signal_timeout().connect(
            [l]() {
                float inL, inR;

                g_object_get(l->stereo_enhancer, "meter-inL", &inL, nullptr);
                g_object_get(l->stereo_enhancer, "meter-inR", &inR, nullptr);

                std::array<double, 2> in_peak = {inL, inR};

                l->input_level.emit(in_peak);

                return true;
            },
            100);

        l->output_level_connection = Glib::signal_timeout().connect(
            [l]() {
                float outL, outR;

                g_object_get(l->stereo_enhancer, "meter-outL", &outL, nullptr);
                g_object_get(l->stereo_enhancer, "meter-outR", &outR, nullptr);

                std::array<double, 2> out_peak = {outL, outR};

                l->output_level.emit(out_peak);

                return true;
            },
            100);

        l->side_level_connection = Glib::signal_timeout().connect(
            [l]() {
                float sideL, sideR;

                g_object_get(l->stereo_enhancer, "meter-sideL", &sideL,
                             nullptr);
                g_object_get(l->stereo_enhancer, "meter-sideR", &sideR,
                             nullptr);

                std::array<double, 2> out_peak = {sideL, sideR};

                l->side_level.emit(out_peak);

                return true;
            },
            100);

    } else {
        l->input_level_connection.disconnect();
        l->output_level_connection.disconnect();
        l->side_level_connection.disconnect();
    }
}

}  // namespace

StereoEnhancer::StereoEnhancer(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    stereo_enhancer = gst_element_factory_make(
        "calf-sourceforge-net-plugins-HaasEnhancer", "stereo_enhancer");

    plugin = gst_insert_bin_new("stereo_enhancer_plugin");

    if (stereo_enhancer != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("StereoEnhancer plugin was not found!");
    }

    if (is_installed) {
        g_object_set(stereo_enhancer, "bypass", false, nullptr);

        bind_to_gsettings();

        g_signal_connect(settings, "changed::state",
                         G_CALLBACK(on_state_changed), this);
        g_signal_connect(settings, "changed::post-messages",
                         G_CALLBACK(on_post_messages_changed), this);

        // useless write just to force callback call

        auto enable = g_settings_get_boolean(settings, "state");

        g_settings_set_boolean(settings, "state", enable);
    }
}

StereoEnhancer::~StereoEnhancer() {}

void StereoEnhancer::bind_to_gsettings() {
    g_settings_bind_with_mapping(settings, "input-gain", stereo_enhancer,
                                 "level-in", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear,
                                 util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "output-gain", stereo_enhancer,
                                 "level-out", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear,
                                 util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind(settings, "middle-source", stereo_enhancer, "m-source",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(settings, "side-gain", stereo_enhancer,
                                 "s-gain", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear,
                                 util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind(settings, "middle-phase", stereo_enhancer, "m-phase",
                    G_SETTINGS_BIND_DEFAULT);

    // left

    g_settings_bind(settings, "left-phase", stereo_enhancer, "s-phase1",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(
        settings, "left-balance", stereo_enhancer, "s-balance1",
        G_SETTINGS_BIND_GET, util::double_to_float, nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "left-delay", stereo_enhancer, "s-delay1",
        G_SETTINGS_BIND_GET, util::double_to_float, nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "left-gain", stereo_enhancer,
                                 "s-gain1", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear,
                                 util::linear_gain_to_db20, nullptr, nullptr);

    // right

    g_settings_bind(settings, "right-phase", stereo_enhancer, "s-phase2",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(
        settings, "right-balance", stereo_enhancer, "s-balance2",
        G_SETTINGS_BIND_GET, util::double_to_float, nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "right-delay", stereo_enhancer, "s-delay2",
        G_SETTINGS_BIND_GET, util::double_to_float, nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "right-gain", stereo_enhancer,
                                 "s-gain2", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear,
                                 util::linear_gain_to_db20, nullptr, nullptr);
}
