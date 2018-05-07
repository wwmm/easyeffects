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

   private:
    Glib::RefPtr<Gst::Bus> bus;
};

#endif
