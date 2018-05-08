#ifndef PIPELINE_BASE_HPP
#define PIPELINE_BASE_HPP

#include <gstreamermm/bus.h>
#include <gstreamermm/element.h>
#include <gstreamermm/pipeline.h>
#include "pulse_manager.hpp"

class PipelineBase {
   public:
    PipelineBase(const uint& sampling_rate);
    virtual ~PipelineBase();

   protected:
    std::string log_tag;

    Glib::RefPtr<Gst::Pipeline> pipeline;

    void set_source_monitor_name(std::string name);
    void set_output_sink_name(std::string name);
    void set_pulseaudio_props(std::string props);

    void on_app_added(std::shared_ptr<AppInfo> app_info);
    void on_app_changed(std::shared_ptr<AppInfo> app_info);
    void on_app_removed(uint idx);

   private:
    Glib::RefPtr<Gst::Bus> bus;

    Glib::RefPtr<Gst::Element> source;
    Glib::RefPtr<Gst::Element> sink;
    Glib::RefPtr<Gst::Element> spectrum;

    std::vector<std::shared_ptr<AppInfo>> apps_list;

    void update_pipeline_state();

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
