#ifndef WEBRTC_HPP
#define WEBRTC_HPP

#include "plugin_base.hpp"

class Webrtc : public PluginBase {
 public:
  Webrtc(const std::string& tag,
         const std::string& schema,
         const int& sampling_rate);
  ~Webrtc();

  GstElement *webrtc = nullptr, *probe_bin = nullptr, *probe_src = nullptr;

  int rate;

  void set_probe_src_device(std::string name);

 private:
  void build_probe_bin();
  void build_dsp_bin();
  void bind_to_gsettings();
};

#endif
