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
            auto msg = Glib::RefPtr<Gst::MessageError>::cast_static(message);

            util::critical(log_tag + msg->parse_error().what());
            util::debug(log_tag + msg->parse_debug());

            break;
        }
        default:
            break;
    }

    return true;
}
