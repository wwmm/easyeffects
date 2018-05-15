#ifndef LIMITER_HPP
#define LIMITER_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <gst/insertbin/gstinsertbin.h>
#include <iostream>

class Limiter {
   public:
    Limiter(std::string tag, std::string schema);
    ~Limiter();

    std::string log_tag;
    GstElement *plugin, *bin;

   private:
    bool is_installed;

    GstElement *limiter, *autovolume;

    GSettings* settings;

    void bind_to_gsettings();
};

#endif
