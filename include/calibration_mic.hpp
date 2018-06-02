#ifndef CALIBRATION_MIC_HPP
#define CALIBRATION_MIC_HPP

#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <vector>

class CalibrationMic {
   public:
    CalibrationMic();

    ~CalibrationMic();

    std::string log_tag = "calibration_mic: ";

    GstElement *pipeline, *source, *sink, *spectrum;

    uint min_spectrum_freq = 20;     // Hz
    uint max_spectrum_freq = 20000;  // Hz
    int spectrum_threshold = -120;   // dB
    uint spectrum_nbands = 3200, spectrum_nfreqs;
    uint spectrum_npoints = 250;  // number of points displayed
    float spline_f0, spline_df;
    bool measure_noise, subtract_noise;
    std::vector<float> spectrum_freqs, spectrum_x_axis;
    std::vector<float> spectrum_mag_tmp, spectrum_mag, noise;

    sigc::signal<void, std::vector<float>> new_spectrum;
    sigc::signal<void> noise_measured;

    void start();
    void stop();
    void set_window(const double& value);

   private:
    GstBus* bus;
};

#endif
