#ifndef WEBRTC_HPP
#define WEBRTC_HPP

#include "plugin_base.hpp"

class Webrtc : public PluginBase {
 public:
  Webrtc(const std::string& tag, const std::string& schema, const std::string& schema_path, const int& sampling_rate);
  Webrtc(const Webrtc&) = delete;
  auto operator=(const Webrtc&) -> Webrtc& = delete;
  Webrtc(const Webrtc&&) = delete;
  auto operator=(const Webrtc &&) -> Webrtc& = delete;
  ~Webrtc() override;

  GstElement *webrtc = nullptr, *probe_bin = nullptr, *probe_src = nullptr;

  int rate;

  void set_probe_src_device(const std::string& name);

 private:
  void build_probe_bin();
  void build_dsp_bin();
  void bind_to_gsettings();
};

#endif
