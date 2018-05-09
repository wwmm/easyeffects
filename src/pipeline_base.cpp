#include "pipeline_base.hpp"
#include "util.hpp"

PipelineBase::PipelineBase(const uint& sampling_rate) {
    gst_init(nullptr, nullptr);

    pipeline = gst_pipeline_new("");

    bus = gst_element_get_bus(pipeline);

    source = gst_element_factory_make("pulsesrc", "source");
    sink = gst_element_factory_make("pulsesink", "sink");
    spectrum = gst_element_factory_make("spectrum", "spectrum");

    auto capsfilter = gst_element_factory_make("capsfilter", nullptr);
    auto queue = gst_element_factory_make("queue", nullptr);

    auto caps_str = "audio/x-raw,format=F32LE,channels=2,rate=" +
                    std::to_string(sampling_rate);

    auto caps = gst_caps_from_string(caps_str.c_str());

    g_object_set(source, "volume", 1.0, nullptr);
    g_object_set(source, "mute", false, nullptr);
    g_object_set(source, "provide-clock", false, nullptr);
    g_object_set(source, "slave-method", 1, nullptr);  // re-timestamp

    g_object_set(sink, "volume", 1.0, nullptr);
    g_object_set(sink, "mute", false, nullptr);
    g_object_set(sink, "provide-clock", true, nullptr);

    g_object_set(capsfilter, "caps", caps, nullptr);

    g_object_set(queue, "silent", true, nullptr);

    gst_bin_add_many(GST_BIN(pipeline), source, capsfilter, queue, sink,
                     nullptr);

    gst_element_link_many(source, capsfilter, queue, sink, nullptr);

    /*
        bus->add_watch(sigc::mem_fun(*this, &PipelineBase::on_message));
    */
}

PipelineBase::~PipelineBase() {
    gst_element_set_state(pipeline, GST_STATE_NULL);

    gst_object_unref(bus);
    gst_object_unref(pipeline);
}
/*
bool PipelineBase::on_message(const Glib::RefPtr<Gst::Bus>& gst_bus,
                              const Glib::RefPtr<Gst::Message>& message) {
    switch (message->get_message_type()) {
        case Gst::MESSAGE_ERROR: {
            on_message_error(gst_bus, message);
            break;
        }
        case Gst::MESSAGE_INFO: {
            on_message_info(gst_bus, message);
            break;
        }
        case Gst::MESSAGE_STATE_CHANGED: {
            on_message_state_changed(gst_bus, message);
            break;
        }
        case Gst::MESSAGE_LATENCY: {
            on_message_latency(gst_bus, message);
            break;
        }
        default:
            break;
    }

    return true;
}

void PipelineBase::on_message_error(const Glib::RefPtr<Gst::Bus>& gst_bus,
                                    const Glib::RefPtr<Gst::Message>& message) {
    auto msg = Glib::RefPtr<Gst::MessageError>::cast_static(message);

    util::critical(log_tag + msg->parse_error().what());
    util::debug(log_tag + msg->parse_debug());
}

void PipelineBase::on_message_info(const Glib::RefPtr<Gst::Bus>& gst_bus,
                                   const Glib::RefPtr<Gst::Message>& message) {
    auto msg = Glib::RefPtr<Gst::MessageInfo>::cast_static(message);

    util::critical(log_tag + msg->parse_error().what());
    util::debug(log_tag + msg->parse_debug());
}

void PipelineBase::on_message_state_changed(
    const Glib::RefPtr<Gst::Bus>& gst_bus,
    const Glib::RefPtr<Gst::Message>& message) {
    auto msg = Glib::RefPtr<Gst::MessageStateChanged>::cast_static(message);

    util::debug(log_tag + Gst::Enums::get_name(msg->parse_new_state()));
}

void PipelineBase::on_message_latency(
    const Glib::RefPtr<Gst::Bus>& gst_bus,
    const Glib::RefPtr<Gst::Message>& message) {
    auto msg = Glib::RefPtr<Gst::MessageLatency>::cast_static(message);

    if (msg->get_source()) {
        int latency, buffer;

        auto name = msg->get_source()->get_name();

        if (name == "source") {
            msg->get_source()->get_property("latency-time", latency);
            msg->get_source()->get_property("buffer-time", buffer);

            util::debug(log_tag +
                        "pulsesrc latency [us]: " + std::to_string(latency));
            util::debug(log_tag +
                        "pulsesrc buffer [us]: " + std::to_string(buffer));
        } else if (name == "sink") {
            msg->get_source()->get_property("latency-time", latency);
            msg->get_source()->get_property("buffer-time", buffer);

            util::debug(log_tag +
                        "pulsesink latency [us]: " + std::to_string(latency));
            util::debug(log_tag +
                        "pulsesink buffer [us]: " + std::to_string(buffer));
        }
    }
}
*/
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
