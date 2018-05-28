#ifndef WEBRTC_HPP
#define WEBRTC_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <iostream>

class Webrtc {
   public:
    Webrtc(std::string tag, std::string schema);
    ~Webrtc();

    std::string log_tag, name = "webrtc";
    GstElement *plugin, *bin, *identity_in, *identity_out, *webrtc, *probe_bin,
        *probe_src;

    void set_probe_src_device(std::string name);

   private:
    GSettings* settings;

    bool is_installed(GstElement* e);
    void build_probe_bin();
    void build_dsp_bin();
    void bind_to_gsettings();
};

#endif
