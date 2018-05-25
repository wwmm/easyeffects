#include <glibmm/main.h>
#include <gst/insertbin/gstinsertbin.h>
#include "maximizer.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, Maximizer* l) {
    auto enable = g_settings_get_boolean(settings, key);
    auto plugin = gst_bin_get_by_name(GST_BIN(l->plugin), "maximizer_bin");

    if (enable) {
        if (!plugin) {
            gst_insert_bin_append(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Maximizer*>(d);

                    if (success) {
                        util::debug(l->log_tag + "maximizer enabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to enable the maximizer");
                    }
                },
                l);
        }
    } else {
        if (plugin) {
            gst_insert_bin_remove(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Maximizer*>(d);

                    if (success) {
                        util::debug(l->log_tag + "maximizer disabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to disable the maximizer");
                    }
                },
                l);
        }
    }
}

void on_post_messages_changed(GSettings* settings, gchar* key, Maximizer* l) {
    auto post = g_settings_get_boolean(settings, key);

    if (post) {
        l->reduction_connection = Glib::signal_timeout().connect(
            [l]() {
                float reduction;

                g_object_get(l->maximizer, "gain-reduction", &reduction,
                             nullptr);

                l->reduction.emit(reduction);

                return true;
            },
            100);
    } else {
        l->reduction_connection.disconnect();
    }
}

}  // namespace

Maximizer::Maximizer(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    maximizer = gst_element_factory_make("ladspa-zamaximx2-ladspa-so-zamaximx2",
                                         nullptr);

    plugin = gst_insert_bin_new("maximizer_plugin");

    if (maximizer != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("Maximizer plugin was not found!");
    }

    if (is_installed) {
        bin = gst_insert_bin_new("maximizer_bin");

        auto in_level =
            gst_element_factory_make("level", "maximizer_input_level");
        auto out_level =
            gst_element_factory_make("level", "maximizer_output_level");

        gst_insert_bin_append(GST_INSERT_BIN(bin), in_level, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), maximizer, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), out_level, nullptr, nullptr);

        g_object_set(maximizer, "bypass", false, nullptr);

        bind_to_gsettings();

        g_signal_connect(settings, "changed::state",
                         G_CALLBACK(on_state_changed), this);
        g_signal_connect(settings, "changed::post-messages",
                         G_CALLBACK(on_post_messages_changed), this);

        g_settings_bind(settings, "post-messages", in_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);
        g_settings_bind(settings, "post-messages", out_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);

        // useless write just to force callback call

        auto enable = g_settings_get_boolean(settings, "state");

        g_settings_set_boolean(settings, "state", enable);
    }
}

Maximizer::~Maximizer() {}

void Maximizer::bind_to_gsettings() {
    g_settings_bind_with_mapping(settings, "release", maximizer, "release",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "ceiling", maximizer, "output-ceiling", G_SETTINGS_BIND_GET,
        util::double_to_float, nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "threshold", maximizer, "threshold",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);
}
