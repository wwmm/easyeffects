#include <glibmm/main.h>
#include <chrono>
#include <thread>
#include "equalizer.hpp"
#include "util.hpp"

namespace {

void on_num_bands_changed(GSettings* settings, gchar* key, Equalizer* l) {
    l->init_equalizer();
}

}  // namespace

Equalizer::Equalizer(const std::string& tag, const std::string& schema)
    : PluginBase(tag, "equalizer", schema) {
    equalizer = gst_element_factory_make("equalizer-nbands", nullptr);

    if (is_installed(equalizer)) {
        auto input_gain = gst_element_factory_make("volume", nullptr);
        auto in_level =
            gst_element_factory_make("level", "equalizer_input_level");
        auto out_level =
            gst_element_factory_make("level", "equalizer_output_level");
        auto output_gain = gst_element_factory_make("volume", nullptr);

        gst_bin_add_many(GST_BIN(bin), input_gain, in_level, equalizer,
                         output_gain, out_level, nullptr);
        gst_element_link_many(input_gain, in_level, equalizer, output_gain,
                              out_level, nullptr);

        auto pad_sink = gst_element_get_static_pad(input_gain, "sink");
        auto pad_src = gst_element_get_static_pad(out_level, "src");

        gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
        gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

        gst_object_unref(GST_OBJECT(pad_sink));
        gst_object_unref(GST_OBJECT(pad_src));

        g_signal_connect(settings, "changed::num-bands",
                         G_CALLBACK(on_num_bands_changed), this);

        g_settings_bind(settings, "post-messages", in_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);
        g_settings_bind(settings, "post-messages", out_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);

        g_settings_bind_with_mapping(
            settings, "input-gain", input_gain, "volume",
            G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear_double,
            util::linear_double_gain_to_db20, nullptr, nullptr);

        g_settings_bind_with_mapping(
            settings, "output-gain", output_gain, "volume",
            G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear_double,
            util::linear_double_gain_to_db20, nullptr, nullptr);

        init_equalizer();

        // useless write just to force on_state_changed callback call

        auto enable = g_settings_get_boolean(settings, "state");

        g_settings_set_boolean(settings, "state", enable);
    }
}

Equalizer::~Equalizer() {
    util::debug(log_tag + name + " destroyed");
}

void Equalizer::bind_band(const int index) {
    auto band =
        gst_child_proxy_get_child_by_index(GST_CHILD_PROXY(equalizer), index);

    g_settings_bind(
        settings, std::string("band" + std::to_string(index) + "-gain").c_str(),
        band, "gain", G_SETTINGS_BIND_GET);

    g_settings_bind(
        settings,
        std::string("band" + std::to_string(index) + "-frequency").c_str(),
        band, "freq", G_SETTINGS_BIND_GET);

    g_settings_bind(
        settings,
        std::string("band" + std::to_string(index) + "-width").c_str(), band,
        "bandwidth", G_SETTINGS_BIND_GET);

    g_settings_bind(
        settings, std::string("band" + std::to_string(index) + "-type").c_str(),
        band, "type", G_SETTINGS_BIND_GET);

    g_object_unref(band);
}

void Equalizer::unbind_band(const int index) {
    auto band =
        gst_child_proxy_get_child_by_index(GST_CHILD_PROXY(equalizer), index);

    g_settings_unbind(
        band, std::string("band" + std::to_string(index) + "-gain").c_str());

    g_settings_unbind(
        band,
        std::string("band" + std::to_string(index) + "-frequency").c_str());

    g_settings_unbind(
        band, std::string("band" + std::to_string(index) + "-width").c_str());

    g_settings_unbind(
        band, std::string("band" + std::to_string(index) + "-type").c_str());

    g_object_unref(band);
}

void Equalizer::init_equalizer() {
    int nbands = g_settings_get_int(settings, "num-bands");
    int current_nbands;

    g_object_get(equalizer, "num-bands", &current_nbands, nullptr);

    if (nbands != current_nbands) {
        bool is_enabled = false;
        GstState state;

        /*Sometimes the equalizer crashes when its number of bands is changed
        while it is running. I don't know if this is a bug or not. Maybe we are
        not supposed to change the number of bands on the fly. In any case it
        does no harm to disable it before doing this change.
        */

        gst_element_get_state(equalizer, &state, nullptr, GST_CLOCK_TIME_NONE);

        if (state == GST_STATE_PLAYING) {
            is_enabled = true;

            disable();

            do {
                gst_element_get_state(equalizer, &state, nullptr,
                                      GST_CLOCK_TIME_NONE);

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            } while (state == GST_STATE_PLAYING);
        }

        for (int n = 0; n < current_nbands; n++) {
            unbind_band(n);
        }

        g_object_set(equalizer, "num-bands", nbands, nullptr);

        for (int n = 0; n < nbands; n++) {
            bind_band(n);
        }

        if (is_enabled) {
            enable();
        }
    }
}
