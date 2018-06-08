#include <glibmm/main.h>
#include "maximizer.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, PluginBase* l) {
    bool enable = g_settings_get_boolean(settings, key);

    if (enable) {
        l->enable();
    } else {
        l->disable();
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

Maximizer::Maximizer(const std::string& tag, const std::string& schema)
    : PluginBase(tag, "maximizer", schema) {
    maximizer = gst_element_factory_make("ladspa-zamaximx2-ladspa-so-zamaximx2",
                                         nullptr);

    if (is_installed(maximizer)) {
        bin = gst_bin_new("maximizer_bin");

        auto in_level =
            gst_element_factory_make("level", "maximizer_input_level");
        auto out_level =
            gst_element_factory_make("level", "maximizer_output_level");
        auto audioconvert = gst_element_factory_make("audioconvert", nullptr);

        gst_bin_add_many(GST_BIN(bin), in_level, audioconvert, maximizer,
                         out_level, nullptr);
        gst_element_link_many(in_level, audioconvert, maximizer, out_level,
                              nullptr);

        auto pad_sink = gst_element_get_static_pad(in_level, "sink");
        auto pad_src = gst_element_get_static_pad(out_level, "src");

        gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
        gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

        gst_object_unref(GST_OBJECT(pad_sink));
        gst_object_unref(GST_OBJECT(pad_src));

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
