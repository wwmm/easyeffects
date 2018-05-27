#ifndef WEBRTC_HPP
#define WEBRTC_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>

class Webrtc {
   public:
    Webrtc(std::string tag, std::string schema);
    ~Webrtc();

    std::string log_tag, name = "webrtc";
    GstElement *plugin, *bin, *webrtc, *probe_bin, *probe_src;

   private:
    bool is_installed;

    GSettings* settings;

    void build_probe_bin();
    void build_dsp_bin();
    void set_probe_src_device(std::string name);
    void bind_to_gsettings();
};

#endif
