#include <gstreamermm/elementfactory.h>
#include <gstreamermm/init.h>
#include "pipeline_base.hpp"
#include "util.hpp"

PipelineBase::PipelineBase() {
    Gst::init();

    pipeline = Gst::Pipeline::create();

    bus = pipeline->get_bus();

    bus->add_watch(sigc::mem_fun(*this, &PipelineBase::on_message));

    source = Gst::ElementFactory::create_element("pulsesrc", "source");
    sink = Gst::ElementFactory::create_element("pulsesink", "sink");

    pipeline->add(source)->add(sink);

    source->link(sink);

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
