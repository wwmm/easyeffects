#ifndef PIPELINE_BASE_HPP
#define PIPELINE_BASE_HPP

#include <gstreamermm/bus.h>
#include <gstreamermm/pipeline.h>

class PipelineBase {
   public:
    PipelineBase();
    virtual ~PipelineBase();

   protected:
    std::string log_tag;
    std::string base_tag = "pipeline_base: ";

    Glib::RefPtr<Gst::Pipeline> pipeline;

   private:
    Glib::RefPtr<Gst::Bus> bus;

    bool on_message(const Glib::RefPtr<Gst::Bus>& gst_bus,
                    const Glib::RefPtr<Gst::Message>& message);

    void on_message_error(const Glib::RefPtr<Gst::Bus>& gst_bus,
                          const Glib::RefPtr<Gst::Message>& message);

    void on_message_info(const Glib::RefPtr<Gst::Bus>& gst_bus,
                         const Glib::RefPtr<Gst::Message>& message);

    void on_message_state_changed(const Glib::RefPtr<Gst::Bus>& gst_bus,
                                  const Glib::RefPtr<Gst::Message>& message);

    void on_message_latency(const Glib::RefPtr<Gst::Bus>& gst_bus,
                            const Glib::RefPtr<Gst::Message>& message);
};

#endif
