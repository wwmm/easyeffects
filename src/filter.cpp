#include <glibmm/main.h>
#include <gst/insertbin/gstinsertbin.h>
#include <cmath>
#include "filter.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, Filter* l) {
    auto enable = g_settings_get_boolean(settings, key);
    auto plugin = gst_bin_get_by_name(GST_BIN(l->plugin), "filter_bin");

    if (enable) {
        if (!plugin) {
            gst_insert_bin_append(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Filter*>(d);

                    if (success) {
                        util::debug(l->log_tag + "filter enabled");
                    } else {
                        util::debug(l->log_tag + "failed to enable the filter");
                    }
                },
                l);
        }
    } else {
        if (plugin) {
            gst_insert_bin_remove(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Filter*>(d);

                    if (success) {
                        util::debug(l->log_tag + "filter disabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to disable the filter");
                    }
                },
                l);
        }
    }
}

void on_post_messages_changed(GSettings* settings, gchar* key, Filter* l) {
    auto post = g_settings_get_boolean(settings, key);

    if (post) {
        l->input_level_connection = Glib::signal_timeout().connect(
            [l]() {
                float inL, inR;

                g_object_get(l->filter, "meter-inL", &inL, nullptr);
                g_object_get(l->filter, "meter-inR", &inR, nullptr);

                std::array<double, 2> in_peak = {inL, inR};

                l->input_level.emit(in_peak);

                return true;
            },
            100);

        l->output_level_connection = Glib::signal_timeout().connect(
            [l]() {
                float outL, outR;

                g_object_get(l->filter, "meter-outL", &outL, nullptr);
                g_object_get(l->filter, "meter-outR", &outR, nullptr);

                std::array<double, 2> out_peak = {outL, outR};

                l->output_level.emit(out_peak);

                return true;
            },
            100);

    } else {
        l->input_level_connection.disconnect();
        l->output_level_connection.disconnect();
    }
}

}  // namespace

Filter::Filter(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    filter = gst_element_factory_make("calf-sourceforge-net-plugins-Filter",
                                      nullptr);

    plugin = gst_insert_bin_new("filter_plugin");

    if (filter != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("Filter plugin was not found!");
    }

    if (is_installed) {
        bin = gst_insert_bin_new("filter_bin");

        gst_insert_bin_append(GST_INSERT_BIN(bin), filter, nullptr, nullptr);

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

Filter::~Filter() {}

void Filter::bind_to_gsettings() {
    g_settings_bind_with_mapping(
        settings, "input-gain", filter, "level-in", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "output-gain", filter, "level-out", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "frequency", filter, "freq",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "ressonance", filter, "res", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind(settings, "mode", filter, "mode", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "inertia", filter, "inertia",
                    G_SETTINGS_BIND_DEFAULT);
}

void Filter::on_new_autovolume_level(const std::array<double, 2>& peak) {
    float gain;

    auto max_value = (peak[0] > peak[1]) ? peak[0] : peak[1];
    auto target = g_settings_get_int(settings, "autovolume-target");
    auto tolerance = g_settings_get_int(settings, "autovolume-tolerance");

    g_object_get(filter, "level-in", &gain, nullptr);

    gain = util::linear_to_db(gain);

    if (max_value > target + tolerance) {
        if (gain - 1 >= -36) {  // -36 = minimum input gain
            gain--;
        }
    } else if (max_value < target - tolerance) {
        if (gain + 1 <= 36) {  // 36 = maximum input gain
            gain++;
        }
    }

    gain = util::db_to_linear(gain);

    g_object_set(filter, "level-in", gain, nullptr);
}
