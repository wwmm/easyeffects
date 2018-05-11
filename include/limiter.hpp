#ifndef LIMITER_HPP
#define LIMITER_HPP

#include <gio/gio.h>
#include <gst/insertbin/gstinsertbin.h>
#include <iostream>

class Limiter {
   public:
    Limiter(std::string schema);
    ~Limiter();

   private:
    GSettings* settings;
};

#endif
