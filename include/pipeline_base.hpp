#ifndef PIPELINE_BASE_HPP
#define PIPELINE_BASE_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <vector>
#include "pulse_manager.hpp"

class PipelineBase {
   public:
    PipelineBase(const std::string& tag, const uint& sampling_rate);
    virtual ~PipelineBase();

    bool playing = false;
    std::string log_tag;

    GstElement *pipeline, *source, *sink, *spectrum, *spectrum_bin,
        *spectrum_identity_in, *spectrum_identity_out, *effects_bin,
        *identity_in, *identity_out;
    GstBus* bus;

    GstClockTime state_check_timeout = 5 * GST_SECOND;

    bool resizing_spectrum = false;
    uint min_spectrum_freq = 20;     // Hz
    uint max_spectrum_freq = 20000;  // Hz
    int spectrum_threshold = -120;   // dB
    uint spectrum_nbands = 1600, spectrum_nfreqs;
    float spline_f0, spline_df;
    std::vector<float> spectrum_freqs, spectrum_x_axis;
    std::vector<float> spectrum_mag_tmp, spectrum_mag;

    void enable_spectrum();
    void disable_spectrum();
    std::array<double, 2> get_peak(GstMessage* message);

    void set_source_monitor_name(std::string name);
    void set_output_sink_name(std::string name);
    void set_null_pipeline();
    void update_pipeline_state();

    sigc::signal<void, std::vector<float>> new_spectrum;

   protected:
    GSettings* settings;

    void set_pulseaudio_props(std::string props);

    void on_app_added(const std::shared_ptr<AppInfo>& app_info);
    void on_app_changed(const std::shared_ptr<AppInfo>& app_info);
    void on_app_removed(uint idx);

   private:
    uint rate;

    std::vector<std::shared_ptr<AppInfo>> apps_list;

    void init_spectrum_bin();
    void init_effects_bin();
    void init_spectrum();
};

#endif
