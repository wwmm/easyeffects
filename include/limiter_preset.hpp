#ifndef LIMITER_PRESET_HPP
#define LIMITER_PRESET_HPP

#include "plugin_preset_base.hpp"

class LimiterPreset : public PluginPresetBase {
   public:
    LimiterPreset();

   private:
    void save(boost::property_tree::ptree& root,
              const std::string& section,
              const Glib::RefPtr<Gio::Settings>& settings) override;
    void load(boost::property_tree::ptree& root,
              const std::string& section,
              const Glib::RefPtr<Gio::Settings>& settings) override;
};

#endif
