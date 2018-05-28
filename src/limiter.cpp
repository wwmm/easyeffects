#include <glibmm/main.h>
#include "limiter.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, Limiter* l) {
    auto enable = g_settings_get_boolean(settings, key);
    if (enable) {
        gst_pad_add_probe(
            gst_element_get_static_pad(l->identity_in, "sink"),
            GST_PAD_PROBE_TYPE_IDLE,
            [](auto pad, auto info, auto d) {
                auto l = static_cast<Limiter*>(d);

                auto plugin =
                    gst_bin_get_by_name(GST_BIN(l->plugin), "limiter_bin");

                if (!plugin) {
                    gst_element_unlink(l->identity_in, l->identity_out);

                    gst_bin_add(GST_BIN(l->plugin), l->bin);

                    gst_element_sync_state_with_parent(l->identity_in);
                    gst_element_sync_state_with_parent(l->bin);
                    gst_element_sync_state_with_parent(l->identity_out);

                    gst_element_link_many(l->identity_in, l->bin,
                                          l->identity_out, nullptr);

                    util::debug(l->log_tag + "limiter enabled");
                }

                return GST_PAD_PROBE_REMOVE;
            },
            l, nullptr);
    } else {
        gst_pad_add_probe(
            gst_element_get_static_pad(l->identity_in, "sink"),
            GST_PAD_PROBE_TYPE_IDLE,
            [](auto pad, auto info, auto d) {
                auto l = static_cast<Limiter*>(d);

                auto plugin =
                    gst_bin_get_by_name(GST_BIN(l->plugin), "limiter_bin");

                if (plugin) {
                    gst_element_unlink_many(l->identity_in, l->bin,
                                            l->identity_out, nullptr);

                    gst_bin_remove(GST_BIN(l->plugin), l->bin);
                    gst_element_set_state(l->bin, GST_STATE_NULL);

                    gst_element_sync_state_with_parent(l->identity_in);
                    gst_element_sync_state_with_parent(l->identity_out);

                    gst_element_link(l->identity_in, l->identity_out);

                    util::debug(l->log_tag + "limiter disabled");
                }

                return GST_PAD_PROBE_REMOVE;
            },
            l, nullptr);
    }
}

void on_post_messages_changed(GSettings* settings, gchar* key, Limiter* l) {
    auto post = g_settings_get_boolean(settings, key);

    if (post) {
        l->input_level_connection = Glib::signal_timeout().connect(
            [l]() {
                float inL, inR;

                g_object_get(l->limiter, "meter-inL", &inL, nullptr);
                g_object_get(l->limiter, "meter-inR", &inR, nullptr);

                std::array<double, 2> in_peak = {inL, inR};

                l->input_level.emit(in_peak);

                return true;
            },
            100);

        l->output_level_connection = Glib::signal_timeout().connect(
            [l]() {
                float outL, outR;

                g_object_get(l->limiter, "meter-outL", &outL, nullptr);
                g_object_get(l->limiter, "meter-outR", &outR, nullptr);

                std::array<double, 2> out_peak = {outL, outR};

                l->output_level.emit(out_peak);

                return true;
            },
            100);

        l->attenuation_connection = Glib::signal_timeout().connect(
            [l]() {
                float att;

                g_object_get(l->limiter, "att", &att, nullptr);

                l->attenuation.emit(att);

                return true;
            },
            100);
    } else {
        l->input_level_connection.disconnect();
        l->output_level_connection.disconnect();
        l->attenuation_connection.disconnect();
    }
}

}  // namespace

Limiter::Limiter(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    limiter = gst_element_factory_make("calf-sourceforge-net-plugins-Limiter",
                                       nullptr);

    plugin = gst_bin_new("limiter_plugin");

    identity_in = gst_element_factory_make("identity", nullptr);
    identity_out = gst_element_factory_make("identity", nullptr);

    gst_bin_add_many(GST_BIN(plugin), identity_in, identity_out, nullptr);
    gst_element_link_many(identity_in, identity_out, nullptr);

    gst_element_add_pad(
        plugin, gst_ghost_pad_new(
                    "sink", gst_element_get_static_pad(identity_in, "sink")));
    gst_element_add_pad(
        plugin, gst_ghost_pad_new(
                    "src", gst_element_get_static_pad(identity_out, "src")));

    if (limiter != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("Limiter plugin was not found!");
    }

    if (is_installed) {
        bin = gst_bin_new("limiter_bin");

        autovolume = gst_element_factory_make("level", "autovolume");

        gst_bin_add_many(GST_BIN(bin), limiter, autovolume, nullptr);
        gst_element_link(limiter, autovolume);

        auto pad_sink = gst_element_get_static_pad(limiter, "sink");
        auto pad_src = gst_element_get_static_pad(autovolume, "src");

        gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
        gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

        gst_object_unref(GST_OBJECT(pad_sink));
        gst_object_unref(GST_OBJECT(pad_src));

        g_object_set(limiter, "bypass", false, nullptr);

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

Limiter::~Limiter() {}

void Limiter::bind_to_gsettings() {
    g_settings_bind_with_mapping(
        settings, "input-gain", limiter, "level-in", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "limit", limiter, "limit",
                                 G_SETTINGS_BIND_GET, util::db20_gain_to_linear,
                                 nullptr, nullptr, nullptr);

    // calf limiter does automatic makeup gain by the same amount given as
    // limit. See https://github.com/calf-studio-gear/calf/issues/162
    // that is why we reduce the output level accordingly

    g_settings_bind_with_mapping(settings, "limit", limiter, "level-out",
                                 G_SETTINGS_BIND_GET, util::db20_gain_to_linear,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "lookahead", limiter, "attack",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "release", limiter, "release",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind(settings, "asc", limiter, "asc", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(settings, "asc-level", limiter, "asc-coeff",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind(settings, "oversampling", limiter, "oversampling",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "autovolume-state", autovolume, "post-messages",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(settings, "autovolume-window", autovolume,
                                 "interval", G_SETTINGS_BIND_GET,
                                 util::ms_to_ns, nullptr, nullptr, nullptr);
}

void Limiter::on_new_autovolume_level(const std::array<double, 2>& peak) {
    float gain;

    auto max_value = (peak[0] > peak[1]) ? peak[0] : peak[1];
    auto target = g_settings_get_int(settings, "autovolume-target");
    auto tolerance = g_settings_get_int(settings, "autovolume-tolerance");

    g_object_get(limiter, "level-in", &gain, nullptr);

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

    g_object_set(limiter, "level-in", gain, nullptr);
}
