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

    sigc::connection input_level_connection;
    sigc::connection output_level_connection;
    sigc::connection compression_connection;

    double get_compression();

    sigc::signal<void, std::array<double, 2>> input_level;
    sigc::signal<void, std::array<double, 2>> output_level;
    sigc::signal<void, double> compression;

   private:
    bool is_installed;

    GSettings* settings;

    void bind_to_gsettings();
};

#endif
