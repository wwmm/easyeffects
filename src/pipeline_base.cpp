#include <glib-object.h>
#include <gobject/gvaluecollector.h>
#include <algorithm>
#include <boost/math/interpolators/cubic_b_spline.hpp>
#include <cmath>
#include "pipeline_base.hpp"
#include "util.hpp"

namespace {

void on_message_error(const GstBus* gst_bus,
                      GstMessage* message,
                      PipelineBase* pb) {
    GError* err;
    gchar* debug;

    gst_message_parse_error(message, &err, &debug);

    util::critical(pb->log_tag + err->message);
    util::debug(pb->log_tag + debug);

    g_error_free(err);
    g_free(debug);
}

void on_message_state_changed(const GstBus* gst_bus,
                              GstMessage* message,
                              PipelineBase* pb) {
    if (GST_OBJECT_NAME(message->src) == std::string("pipeline")) {
        GstState old_state, new_state;

        gst_message_parse_state_changed(message, &old_state, &new_state,
                                        nullptr);

        util::debug(pb->log_tag + gst_element_state_get_name(new_state));
    }
}

void on_message_latency(const GstBus* gst_bus,
                        GstMessage* message,
                        PipelineBase* pb) {
    if (GST_OBJECT_NAME(message->src) == std::string("source")) {
        int latency, buffer;

        g_object_get(pb->source, "latency-time", &latency, nullptr);
        g_object_get(pb->source, "buffer-time", &buffer, nullptr);

        util::debug(pb->log_tag +
                    "pulsesrc latency [us]: " + std::to_string(latency));
        util::debug(pb->log_tag +
                    "pulsesrc buffer [us]: " + std::to_string(buffer));
    } else if (GST_OBJECT_NAME(message->src) == std::string("sink")) {
        int latency, buffer;

        g_object_get(pb->sink, "latency-time", &latency, nullptr);
        g_object_get(pb->sink, "buffer-time", &buffer, nullptr);

        util::debug(pb->log_tag +
                    "pulsesink latency [us]: " + std::to_string(latency));
        util::debug(pb->log_tag +
                    "pulsesink buffer [us]: " + std::to_string(buffer));
    }
}

void on_message_element(const GstBus* gst_bus,
                        GstMessage* message,
                        PipelineBase* pb) {
    if (GST_OBJECT_NAME(message->src) == std::string("spectrum") &&
        !pb->resizing_spectrum) {
        const GstStructure* s = gst_message_get_structure(message);

        const GValue* magnitudes;

        magnitudes = gst_structure_get_value(s, "magnitude");

        for (uint n = 0; n < pb->spectrum_freqs.size(); n++) {
            pb->spectrum_mag_tmp[n] =
                g_value_get_float(gst_value_list_get_value(magnitudes, n));
        }

        boost::math::cubic_b_spline<float> spline(pb->spectrum_mag_tmp.begin(),
                                                  pb->spectrum_mag_tmp.end(),
                                                  pb->spline_f0, pb->spline_df);

        for (uint n = 0; n < pb->spectrum_mag.size(); n++) {
            pb->spectrum_mag[n] = spline(pb->spectrum_x_axis[n]);
        }

        auto min_mag = pb->spectrum_threshold;
        auto max_mag =
            *std::max_element(pb->spectrum_mag.begin(), pb->spectrum_mag.end());

        if (max_mag > min_mag) {
            for (uint n = 0; n < pb->spectrum_mag.size(); n++) {
                pb->spectrum_mag[n] = (min_mag - pb->spectrum_mag[n]) / min_mag;
            }

            Glib::signal_idle().connect([pb]() {
                pb->new_spectrum.emit(pb->spectrum_mag);
                return false;
            });
        }
    }
}

void on_spectrum_n_points_changed(GSettings* settings,
                                  gchar* key,
                                  PipelineBase* pb) {
    pb->resizing_spectrum = true;

    auto npoints = g_settings_get_int(settings, "spectrum-n-points");

    pb->spectrum_mag.resize(npoints);

    pb->spectrum_x_axis = util::logspace(log10(pb->min_spectrum_freq),
                                         log10(pb->max_spectrum_freq), npoints);

    pb->resizing_spectrum = false;
}

}  // namespace

PipelineBase::PipelineBase(const std::string& tag, const uint& sampling_rate)
    : log_tag(tag),
      settings(g_settings_new("com.github.wwmm.pulseeffects")),
      rate(sampling_rate) {
    gst_init(nullptr, nullptr);

    pipeline = gst_pipeline_new("pipeline");

    bus = gst_element_get_bus(pipeline);

    gst_bus_add_signal_watch(bus);

    // bus callbacks

    g_signal_connect(bus, "message::error", G_CALLBACK(on_message_error), this);
    g_signal_connect(bus, "message::state-changed",
                     G_CALLBACK(on_message_state_changed), this);
    g_signal_connect(bus, "message::latency", G_CALLBACK(on_message_latency),
                     this);
    g_signal_connect(bus, "message::element", G_CALLBACK(on_message_element),
                     this);

    // creating elements common to all pipelines

    source = gst_element_factory_make("pulsesrc", "source");
    sink = gst_element_factory_make("pulsesink", "sink");
    spectrum = gst_element_factory_make("spectrum", "spectrum");

    auto capsfilter = gst_element_factory_make("capsfilter", nullptr);
    auto queue = gst_element_factory_make("queue", nullptr);

    effects_bin = GST_INSERT_BIN(gst_insert_bin_new("effects_bin"));
    spectrum_wrapper = GST_INSERT_BIN(gst_insert_bin_new("spectrum_wrapper"));

    auto caps_str =
        "audio/x-raw,format=F32LE,channels=2,rate=" + std::to_string(rate);

    auto caps = gst_caps_from_string(caps_str.c_str());

    // building the pipeline

    gst_bin_add_many(GST_BIN(pipeline), source, capsfilter, queue, effects_bin,
                     spectrum_wrapper, sink, nullptr);

    gst_element_link_many(source, capsfilter, queue, effects_bin,
                          spectrum_wrapper, sink, nullptr);

    // initializing properties

    g_object_set(source, "volume", 1.0, nullptr);
    g_object_set(source, "mute", false, nullptr);
    g_object_set(source, "provide-clock", false, nullptr);
    g_object_set(source, "slave-method", 1, nullptr);  // re-timestamp

    g_object_set(sink, "volume", 1.0, nullptr);
    g_object_set(sink, "mute", false, nullptr);
    g_object_set(sink, "provide-clock", true, nullptr);

    g_object_set(capsfilter, "caps", caps, nullptr);

    g_object_set(queue, "silent", true, nullptr);

    g_object_set(spectrum, "bands", spectrum_nbands, nullptr);
    g_object_set(spectrum, "threshold", spectrum_threshold, nullptr);

    init_spectrum();
}

PipelineBase::~PipelineBase() {
    gst_element_set_state(pipeline, GST_STATE_NULL);

    gst_object_unref(bus);
    gst_object_unref(pipeline);
}

void PipelineBase::set_source_monitor_name(std::string name) {
    std::string current_device;

    g_object_get(source, "current-device", &current_device, nullptr);

    if (name != current_device) {
        GstState state, pending;

        gst_element_get_state(pipeline, &state, &pending, GST_CLOCK_TIME_NONE);

        if (state == GST_STATE_PLAYING) {
            gst_element_set_state(pipeline, GST_STATE_NULL);

            g_object_set(source, "device", name.c_str(), nullptr);

            gst_element_set_state(pipeline, GST_STATE_PLAYING);
        } else {
            g_object_set(source, "device", name.c_str(), nullptr);
        }
    }
}

void PipelineBase::set_output_sink_name(std::string name) {
    g_object_set(sink, "device", name.c_str(), nullptr);
}

void PipelineBase::set_pulseaudio_props(std::string props) {
    auto str = "props," + props;

    auto s = gst_structure_from_string(str.c_str(), nullptr);

    g_object_set(source, "stream-properties", s, nullptr);
}

void PipelineBase::update_pipeline_state() {
    bool wants_to_play = false;

    for (auto a : apps_list) {
        if (a->wants_to_play) {
            wants_to_play = true;

            break;
        }
    }

    GstState state, pending;

    gst_element_get_state(pipeline, &state, &pending, GST_CLOCK_TIME_NONE);

    if (state != GST_STATE_PLAYING && wants_to_play) {
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
    } else if (state == GST_STATE_PLAYING && !wants_to_play) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
    }
}

void PipelineBase::on_app_added(const std::shared_ptr<AppInfo>& app_info) {
    apps_list.push_back(move(app_info));

    update_pipeline_state();
}

void PipelineBase::on_app_changed(const std::shared_ptr<AppInfo>& app_info) {
    for (auto it = apps_list.begin(); it != apps_list.end(); it++) {
        auto n = it - apps_list.begin();

        if (apps_list[n]->index == app_info->index) {
            apps_list[n] = move(app_info);
        }
    }

    update_pipeline_state();
}

void PipelineBase::on_app_removed(uint idx) {
    for (auto it = apps_list.begin(); it != apps_list.end(); it++) {
        auto n = it - apps_list.begin();

        if (apps_list[n]->index == idx) {
            auto app = move(apps_list[n]);

            apps_list.erase(it);

            break;
        }
    }

    update_pipeline_state();
}

void PipelineBase::init_spectrum() {
    g_signal_connect(settings, "changed::spectrum-n-points",
                     G_CALLBACK(on_spectrum_n_points_changed), this);

    for (uint n = 0; n < spectrum_nbands; n++) {
        auto f = rate * (0.5 * n + 0.25) / spectrum_nbands;

        if (f > max_spectrum_freq) {
            break;
        }

        if (f > min_spectrum_freq) {
            spectrum_freqs.push_back(f);
        }
    }

    spectrum_mag_tmp.resize(spectrum_freqs.size());

    auto npoints = g_settings_get_int(settings, "spectrum-n-points");

    spectrum_x_axis = util::logspace(log10(min_spectrum_freq),
                                     log10(max_spectrum_freq), npoints);

    spectrum_mag.resize(npoints);

    spline_f0 = spectrum_freqs[0];
    spline_df = spectrum_freqs[1] - spectrum_freqs[0];
}

void PipelineBase::enable_spectrum() {
    auto plugin = gst_bin_get_by_name(GST_BIN(spectrum_wrapper), "spectrum");

    if (!plugin) {
        gst_insert_bin_append(
            GST_INSERT_BIN(spectrum_wrapper), spectrum,
            [](auto bin, auto elem, auto success, auto d) {
                auto pb = static_cast<PipelineBase*>(d);

                if (success) {
                    util::debug(pb->log_tag + "spectrum enabled");
                } else {
                    util::debug(pb->log_tag + "failed to enable the spectrum");
                }
            },
            this);
    }
}

void PipelineBase::disable_spectrum() {
    auto plugin = gst_bin_get_by_name(GST_BIN(spectrum_wrapper), "spectrum");

    if (plugin) {
        gst_insert_bin_remove(
            GST_INSERT_BIN(spectrum_wrapper), spectrum,
            [](auto bin, auto elem, auto success, auto d) {
                auto pb = static_cast<PipelineBase*>(d);

                if (success) {
                    util::debug(pb->log_tag + "spectrum disabled");
                } else {
                    util::debug(pb->log_tag + "failed to disable the spectrum");
                }
            },
            this);
    }
}

std::array<double, 2> PipelineBase::get_peak(GstMessage* message) {
    std::array<double, 2> peak;

    const GstStructure* s = gst_message_get_structure(message);

    auto gpeak =
        (GValueArray*)g_value_get_boxed(gst_structure_get_value(s, "peak"));

    peak[0] = g_value_get_double(gpeak->values);      // left
    peak[1] = g_value_get_double(gpeak->values + 1);  // right

    return peak;
}
