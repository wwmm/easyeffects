#include <gstreamermm/audiobasesrc.h>
#include <gstreamermm/caps.h>
#include <gstreamermm/elementfactory.h>
#include <gstreamermm/init.h>
#include <gstreamermm/structure.h>
#include "pipeline_base.hpp"
#include "util.hpp"

PipelineBase::PipelineBase(const uint& sampling_rate) {
    Gst::init();

    pipeline = Gst::Pipeline::create();

    bus = pipeline->get_bus();

    bus->add_watch(sigc::mem_fun(*this, &PipelineBase::on_message));

    source = Gst::ElementFactory::create_element("pulsesrc", "source");
    sink = Gst::ElementFactory::create_element("pulsesink", "sink");
    spectrum = Gst::ElementFactory::create_element("spectrum", "spectrum");

    auto capsfilter = Gst::ElementFactory::create_element("capsfilter");
    auto queue = Gst::ElementFactory::create_element("queue");

    auto caps = Gst::Caps::create_from_string(
        "audio/x-raw,format=F32LE,channels=2,rate=" +
        std::to_string(sampling_rate));

    source->set_property("volume", 1.0);
    source->set_property("mute", false);
    source->set_property("provide-clock", false);
    source->set_property(
        "slave-method",
        Gst::AudioBaseSrcSlaveMethod::AUDIO_BASE_SRC_SLAVE_RETIMESTAMP);

    sink->set_property("volume", 1.0);
    sink->set_property("mute", false);
    sink->set_property("provide-clock", true);

    capsfilter->set_property("caps", caps);

    queue->set_property("silent", true);

    try {
        pipeline->add(source)->add(capsfilter)->add(queue)->add(sink);

        source->link(capsfilter)->link(queue)->link(sink);
    } catch (const std::runtime_error& ex) {
        util::error(log_tag + ex.what());
    }

    pipeline->set_state(Gst::STATE_PLAYING);
}

PipelineBase::~PipelineBase() {
    pipeline->set_state(Gst::STATE_NULL);
}

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

void PipelineBase::set_source_monitor_name(std::string name) {
    std::string current_device;

    source->get_property("current-device", current_device);

    if (name != current_device) {
        Gst::State state, pending;

        pipeline->get_state(state, pending, Gst::CLOCK_TIME_NONE);

        if (state == Gst::STATE_PLAYING) {
            pipeline->set_state(Gst::STATE_NULL);

            source->set_property("device", name);

            pipeline->set_state(Gst::STATE_PLAYING);
        } else {
            source->set_property("device", name);
        }
    }
}

void PipelineBase::set_output_sink_name(std::string name) {
    sink->set_property("device", name);
}

void PipelineBase::set_pulseaudio_props(std::string props) {
    auto s = Gst::Structure::create_from_string("props," + props);

    source->set_property("stream-properties", s);
}
