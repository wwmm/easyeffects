#ifndef LIMITER_PRESET_HPP
#define LIMITER_PRESET_HPP

#include <giomm/settings.h>
#include <boost/property_tree/ptree.hpp>

class LimiterPreset {
   public:
    LimiterPreset();

    void write(boost::property_tree::ptree& root);
    void read(boost::property_tree::ptree& root);

   private:
    Glib::RefPtr<Gio::Settings> input_settings, output_settings;

    void save(boost::property_tree::ptree& root,
              const std::string& section,
              const Glib::RefPtr<Gio::Settings>& settings);
    void load(boost::property_tree::ptree& root,
              const std::string& section,
              const Glib::RefPtr<Gio::Settings>& settings);
};

#endif
