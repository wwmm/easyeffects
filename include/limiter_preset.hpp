#ifndef LIMITER_PRESET_HPP
#define LIMITER_PRESET_HPP

#include <giomm/settings.h>
#include <boost/property_tree/ptree.hpp>

class LimiterPreset {
   public:
    LimiterPreset(const boost::property_tree::ptree& treeroot);

    void save();
    void load();

   private:
    boost::property_tree::ptree root;
    Glib::RefPtr<Gio::Settings> input_settings, output_settings;

    void save_input_settings();
    void save_output_settings();
    void load_input_settings();
    void load_output_settings();
};

#endif
