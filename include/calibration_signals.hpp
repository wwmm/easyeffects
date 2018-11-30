#ifndef CALIBRATION_SIGNALS_HPP
#define CALIBRATION_SIGNALS_HPP

#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <iostream>
#include <vector>

class CalibrationSignals {
 public:
  CalibrationSignals();

  ~CalibrationSignals();

  std::string log_tag = "calibration_signals: ";

  GstElement *pipeline = nullptr, *source = nullptr, *sink = nullptr,
             *spectrum = nullptr;

  uint min_spectrum_freq = 20;     // Hz
  uint max_spectrum_freq = 20000;  // Hz
  int spectrum_threshold = -120;   // dB
  uint spectrum_nbands = 3200, spectrum_nfreqs;
  uint spectrum_npoints = 300;  // number of points displayed
  float spline_f0, spline_df;
  std::vector<float> spectrum_freqs, spectrum_x_axis;
  std::vector<float> spectrum_mag_tmp, spectrum_mag;

  sigc::signal<void, std::vector<float>> new_spectrum;

  void start();
  void stop();
  void set_freq(const double& value);
  void set_volume(const double& value);

 private:
  GstBus* bus = nullptr;
};

#endif
