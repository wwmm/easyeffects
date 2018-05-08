#include <gstreamermm/init.h>
#include "pipeline_base.hpp"
#include "util.hpp"

PipelineBase::PipelineBase() {
    Gst::init();

    pipeline = Gst::Pipeline::create();

    bus = pipeline->get_bus();

    bus->add_watch(sigc::mem_fun(*this, &PipelineBase::on_message));

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

    util::critical(log_tag + base_tag + msg->parse_error().what());
    util::debug(log_tag + base_tag + msg->parse_debug());
}

void PipelineBase::on_message_info(const Glib::RefPtr<Gst::Bus>& gst_bus,
                                   const Glib::RefPtr<Gst::Message>& message) {
    auto msg = Glib::RefPtr<Gst::MessageInfo>::cast_static(message);

    util::critical(log_tag + base_tag + msg->parse_error().what());
    util::debug(log_tag + base_tag + msg->parse_debug());
}

void PipelineBase::on_message_state_changed(
    const Glib::RefPtr<Gst::Bus>& gst_bus,
    const Glib::RefPtr<Gst::Message>& message) {
    auto msg = Glib::RefPtr<Gst::MessageStateChanged>::cast_static(message);

    util::debug(log_tag + base_tag + "new pipeline state:" +
                Gst::Enums::get_name(msg->parse_new_state()));
}

void PipelineBase::on_message_latency(
    const Glib::RefPtr<Gst::Bus>& gst_bus,
    const Glib::RefPtr<Gst::Message>& message) {
    auto msg = Glib::RefPtr<Gst::MessageLatency>::cast_static(message);

    util::debug(log_tag + base_tag + "latency msg");
}
