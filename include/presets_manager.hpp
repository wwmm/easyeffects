#ifndef PRESETS_MANAGER_HPP
#define PRESETS_MANAGER_HPP

#include <boost/filesystem.hpp>
#include <memory>
#include "limiter_preset.hpp"

class PresetsManager {
   public:
    PresetsManager();
    virtual ~PresetsManager();

    std::vector<std::string> get_names();
    void add(const std::string& name);
    void save(const std::string& name);
    void remove(const std::string& name);
    void load(const std::string& name);
    void import(const std::string& name);

   private:
    std::string log_tag = "presets_manager: ";

    boost::filesystem::path presets_dir;

    std::unique_ptr<LimiterPreset> limiter;
};

#endif
