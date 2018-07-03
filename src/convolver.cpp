#include <glibmm/main.h>
#include <gst/fft/gstfftf32.h>
#include "convolver.hpp"
#include "util.hpp"

namespace {

int k_size = 1024;

static void on_rate_changed(GstElement* element, gint rate, Convolver* l) {
    GValueArray* va;

    GValue v = {
        0,
    };

    GstFFTF32* fft;
    GstFFTF32Complex frequency_response[k_size / 2 + 1];
    gfloat tmp[k_size];
    gfloat filter_kernel[k_size];
    guint i;

    std::cout << rate << std::endl;

    /* Create the frequency response: zero outside
     * a small frequency band */
    for (i = 0; i < k_size / 2 + 1; i++) {
        if (i >= k_size / 4)
            frequency_response[i].r = 0.0;
        else
            frequency_response[i].r = 1.0;

        // frequency_response[i].r = 1.0;
        frequency_response[i].i = 0.0;
    }

    /* Calculate the inverse FT of the frequency response */
    fft = gst_fft_f32_new(k_size, true);
    gst_fft_f32_inverse_fft(fft, frequency_response, tmp);
    gst_fft_f32_free(fft);

    /* Shift the inverse FT of the frequency response by 16,
     * i.e. the half of the kernel length to get the
     * impulse response. See http://www.dspguide.com/ch17/1.htm
     * for more information.
     */
    for (i = 0; i < k_size; i++)
        filter_kernel[i] = tmp[(i + k_size / 2) % k_size];

    /* Apply the hamming window to the impulse response to get
     * a better result than given from the rectangular window
     */
    for (i = 0; i < k_size; i++)
        filter_kernel[i] *= (0.54 - 0.46 * cos(2 * G_PI * i / k_size));

    va = g_value_array_new(k_size);

    g_value_init(&v, G_TYPE_DOUBLE);
    for (i = 0; i < k_size; i++) {
        g_value_set_double(&v, filter_kernel[i]);
        g_value_array_append(va, &v);
        g_value_reset(&v);
    }

    g_object_set(G_OBJECT(element), "kernel", va, nullptr);

    /* Latency is 1/2 of the kernel length for this method of
     * calculating a filter kernel from the frequency response
     */
    g_object_set(G_OBJECT(element), "latency", (gint64)(k_size / 2), nullptr);
    g_value_array_free(va);
}

}  // namespace

Convolver::Convolver(const std::string& tag, const std::string& schema)
    : PluginBase(tag, "convolver", schema) {
    convolver = gst_element_factory_make("audiofirfilter", "convolver");

    if (is_installed(convolver)) {
        auto input_gain = gst_element_factory_make("volume", nullptr);
        auto in_level =
            gst_element_factory_make("level", "convolver_input_level");
        auto out_level =
            gst_element_factory_make("level", "convolver_output_level");
        auto output_gain = gst_element_factory_make("volume", nullptr);

        gst_bin_add_many(GST_BIN(bin), input_gain, in_level, convolver,
                         output_gain, out_level, nullptr);
        gst_element_link_many(input_gain, in_level, convolver, output_gain,
                              out_level, nullptr);

        auto pad_sink = gst_element_get_static_pad(input_gain, "sink");
        auto pad_src = gst_element_get_static_pad(out_level, "src");

        gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
        gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

        gst_object_unref(GST_OBJECT(pad_sink));
        gst_object_unref(GST_OBJECT(pad_src));

        bind_to_gsettings();

        // g_object_set(convolver, "low-latency", true, NULL);

        g_signal_connect(convolver, "rate-changed", G_CALLBACK(on_rate_changed),
                         this);
        // g_signal_connect(settings, "changed::post-messages",
        //                  G_CALLBACK(on_post_messages_changed), this);

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

        // useless write just to force callback call

        auto enable = g_settings_get_boolean(settings, "state");

        g_settings_set_boolean(settings, "state", enable);
    }
}

Convolver::~Convolver() {
    util::debug(log_tag + name + " destroyed");
}

void Convolver::bind_to_gsettings() {
    // g_settings_bind_with_mapping(
    //     settings, "input-gain", convolver, "level-in",
    //     G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear,
    //     util::linear_gain_to_db20, nullptr, nullptr);
    //
    // g_settings_bind_with_mapping(settings, "output-gain", convolver,
    //                              "level-out", G_SETTINGS_BIND_DEFAULT,
    //                              util::db20_gain_to_linear,
    //                              util::linear_gain_to_db20, nullptr,
    //                              nullptr);
}
