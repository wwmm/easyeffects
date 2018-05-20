#include <glibmm/main.h>
#include <gst/insertbin/gstinsertbin.h>
#include <cmath>
#include "equalizer.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, Equalizer* l) {
    auto enable = g_settings_get_boolean(settings, key);
    auto plugin = gst_bin_get_by_name(GST_BIN(l->plugin), "equalizer_bin");

    if (enable) {
        if (!plugin) {
            gst_insert_bin_append(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Equalizer*>(d);

                    if (success) {
                        util::debug(l->log_tag + "equalizer enabled");

                        l->is_enabled = true;
                    } else {
                        util::debug(l->log_tag +
                                    "failed to enable the equalizer");

                        l->is_enabled = false;
                    }
                },
                l);
        }
    } else {
        if (plugin) {
            gst_insert_bin_remove(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Equalizer*>(d);

                    if (success) {
                        util::debug(l->log_tag + "equalizer disabled");

                        l->is_enabled = false;
                    } else {
                        util::debug(l->log_tag +
                                    "failed to disable the equalizer");

                        l->is_enabled = true;
                    }
                },
                l);
        }
    }
}

void on_num_bands_changed(GSettings* settings, gchar* key, Equalizer* l) {
    l->init_equalizer();
}

}  // namespace

Equalizer::Equalizer(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    equalizer = gst_element_factory_make("equalizer-nbands", nullptr);

    plugin = gst_insert_bin_new("equalizer_plugin");

    if (equalizer != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("Equalizer plugin was not found!");
    }

    if (is_installed) {
        bin = gst_insert_bin_new("equalizer_bin");

        in_level = gst_element_factory_make("level", "equalizer_input_level");
        out_level = gst_element_factory_make("level", "equalizer_output_level");

        gst_insert_bin_append(GST_INSERT_BIN(bin), in_level, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), equalizer, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), out_level, nullptr, nullptr);

        g_signal_connect(settings, "changed::state",
                         G_CALLBACK(on_state_changed), this);
        g_signal_connect(settings, "changed::num-bands",
                         G_CALLBACK(on_num_bands_changed), this);

        g_settings_bind(settings, "post-messages", in_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);
        g_settings_bind(settings, "post-messages", out_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);

        init_equalizer();

        // useless write just to force callback call

        auto enable = g_settings_get_boolean(settings, "state");

        g_settings_set_boolean(settings, "state", enable);
    }
}

Equalizer::~Equalizer() {}

void Equalizer::init_equalizer() {
    long unsigned int nbands = g_settings_get_int(settings, "num-bands");

    if (nbands != bands.size()) {
        for (auto b : bands) {
            g_object_unref(b);
        }

        bands.clear();

        g_object_set(equalizer, "num-bands", nbands, nullptr);

        for (long unsigned int n = 0; n < nbands; n++) {
            bands.push_back(gst_child_proxy_get_child_by_index(
                GST_CHILD_PROXY(equalizer), n));

            g_settings_bind(
                settings,
                std::string("band" + std::to_string(n) + "-gain").c_str(),
                bands[n], "gain", G_SETTINGS_BIND_DEFAULT);

            g_settings_bind(
                settings,
                std::string("band" + std::to_string(n) + "-frequency").c_str(),
                bands[n], "freq", G_SETTINGS_BIND_DEFAULT);

            g_settings_bind(
                settings,
                std::string("band" + std::to_string(n) + "-width").c_str(),
                bands[n], "bandwidth", G_SETTINGS_BIND_DEFAULT);
        }
    }
}
