#ifndef WEBRTC_HPP
#define WEBRTC_HPP

#include "plugin_base.hpp"

class Webrtc : public PluginBase {
   public:
    Webrtc(std::string tag, std::string schema);
    ~Webrtc();

    GstElement *webrtc, *probe_bin, *probe_src;

    void set_probe_src_device(std::string name);

   private:
    void build_probe_bin();
    void build_dsp_bin();
    void bind_to_gsettings();
};

#endif
