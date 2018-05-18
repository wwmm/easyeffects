#ifndef COMPRESSOR_HPP
#define COMPRESSOR_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <gst/insertbin/gstinsertbin.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>

class Compressor {
   public:
    Compressor(std::string tag, std::string schema);
    ~Compressor();

    std::string log_tag, name = "compressor";
    GstElement *plugin, *bin, *compressor;

    sigc::connection compression_connection;

    double get_compression();

    sigc::signal<void, double> compression;

   private:
    bool is_installed;

    GSettings* settings;

    void bind_to_gsettings();
};

#endif
