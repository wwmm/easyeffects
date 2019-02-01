#ifndef CRYSTALIZER_HPP
#define CRYSTALIZER_HPP

#include <sigc++/sigc++.h>
#include <array>
#include "plugin_base.hpp"

class Crystalizer : public PluginBase {
 public:
  Crystalizer(const std::string& tag, const std::string& schema);
  ~Crystalizer();

  GstElement *crystalizer_low = nullptr, *crystalizer_mid = nullptr,
             *crystalizer_high = nullptr, *lowpass = nullptr,
             *bandpass = nullptr, *highpass = nullptr, *tee = nullptr,
             *mixer = nullptr;

  GstPad *tee_src0, *tee_src1, *tee_src2, *mixer_sink0, *mixer_sink1,
      *mixer_sink2;

 private:
  void bind_to_gsettings();
};

#endif
