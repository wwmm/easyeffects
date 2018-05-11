#ifndef LIMITER_HPP
#define LIMITER_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <iostream>

class Limiter {
   public:
    Limiter(std::string tag, std::string schema);
    ~Limiter();

   private:
    std::string log_tag;
    bool is_installed;

    GstElement* limiter;

    GSettings* settings;
};

#endif
