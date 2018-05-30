#ifndef PLUGIN_PRESET_BASE_HPP
#define PLUGIN_PRESET_BASE_HPP

#include <giomm/settings.h>
#include <boost/property_tree/ptree.hpp>

class PluginPresetBase {
   public:
    PluginPresetBase(const std::string& InputSettings,
                     const std::string& OutputSettings);
    virtual ~PluginPresetBase();

    void write(boost::property_tree::ptree& root);
    void read(boost::property_tree::ptree& root);

   protected:
    Glib::RefPtr<Gio::Settings> input_settings, output_settings;

    virtual void save(boost::property_tree::ptree& root,
                      const std::string& section,
                      const Glib::RefPtr<Gio::Settings>& settings) = 0;
    virtual void load(boost::property_tree::ptree& root,
                      const std::string& section,
                      const Glib::RefPtr<Gio::Settings>& settings) = 0;
};

#endif
