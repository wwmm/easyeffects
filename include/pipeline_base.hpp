#ifndef PIPELINE_BASE_HPP
#define PIPELINE_BASE_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <vector>
#include "pulse_manager.hpp"

class PipelineBase {
   public:
    PipelineBase(const uint& sampling_rate);
    virtual ~PipelineBase();

    std::string log_tag;

    GstElement *pipeline, *source, *sink, *spectrum;
    GstBus* bus;

    bool resizing_spectrum = false;
    uint min_spectrum_freq = 20;     // Hz
    uint max_spectrum_freq = 20000;  // Hz
    int spectrum_threshold = -120;   // dB
    uint spectrum_nbands = 1600, spectrum_nfreqs;
    float spline_f0, spline_df;
    std::vector<float> spectrum_freqs, spectrum_x_axis;
    std::vector<float> spectrum_mag_tmp, spectrum_mag;

    sigc::signal<void, std::vector<float>> new_spectrum;

   protected:
    GSettings* settings;

    void set_source_monitor_name(std::string name);
    void set_output_sink_name(std::string name);
    void set_pulseaudio_props(std::string props);

    void on_app_added(const std::shared_ptr<AppInfo>& app_info);
    void on_app_changed(const std::shared_ptr<AppInfo>& app_info);
    void on_app_removed(uint idx);

   private:
    uint rate;

    std::vector<std::shared_ptr<AppInfo>> apps_list;

    void update_pipeline_state();
    void calc_spectrum_freqs();
};

#endif
