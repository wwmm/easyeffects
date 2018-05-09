#ifndef PIPELINE_BASE_HPP
#define PIPELINE_BASE_HPP

#include <gst/gst.h>
#include "pulse_manager.hpp"

class PipelineBase {
   public:
    PipelineBase(const uint& sampling_rate);
    virtual ~PipelineBase();

   protected:
    std::string log_tag;

    // Glib::RefPtr<Gst::Element> effects_bin;

    void set_source_monitor_name(std::string name);
    void set_output_sink_name(std::string name);
    void set_pulseaudio_props(std::string props);

    void on_app_added(const std::shared_ptr<AppInfo>& app_info);
    void on_app_changed(const std::shared_ptr<AppInfo>& app_info);
    void on_app_removed(uint idx);

   private:
    GstElement *pipeline, *source, *sink, *spectrum;
    GstBus* bus;

    std::vector<std::shared_ptr<AppInfo>> apps_list;

    void update_pipeline_state();

    // bool on_message(const Glib::RefPtr<Gst::Bus>& gst_bus,
    //                 const Glib::RefPtr<Gst::Message>& message);
    //
    // void on_message_error(const Glib::RefPtr<Gst::Bus>& gst_bus,
    //                       const Glib::RefPtr<Gst::Message>& message);
    //
    // void on_message_info(const Glib::RefPtr<Gst::Bus>& gst_bus,
    //                      const Glib::RefPtr<Gst::Message>& message);
    //
    // void on_message_state_changed(const Glib::RefPtr<Gst::Bus>& gst_bus,
    //                               const Glib::RefPtr<Gst::Message>& message);
    //
    // void on_message_latency(const Glib::RefPtr<Gst::Bus>& gst_bus,
    //                         const Glib::RefPtr<Gst::Message>& message);
};

#endif
