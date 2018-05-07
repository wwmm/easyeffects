#include <gstreamermm/init.h>
#include "pipeline_base.hpp"

PipelineBase::PipelineBase() {
    Gst::init();

    pipeline = Gst::Pipeline::create();

    bus = pipeline->get_bus();

    // bus->add_signal_watch();
}

PipelineBase::~PipelineBase() {}
