#ifndef PIPELINE_BASE_HPP
#define PIPELINE_BASE_HPP

#include <gstreamermm/bus.h>
#include <gstreamermm/pipeline.h>

class PipelineBase {
   public:
    PipelineBase();
    virtual ~PipelineBase();

   protected:
    Glib::RefPtr<Gst::Pipeline> pipeline;

    bool on_message(const Glib::RefPtr<Gst::Bus>& gst_bus,
                    const Glib::RefPtr<Gst::Message>& message);

   private:
    std::string log_tag = "pipeline_base.cpp: ";

    Glib::RefPtr<Gst::Bus> bus;
};

#endif
