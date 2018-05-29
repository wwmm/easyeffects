#ifndef PRESETS_MANAGER_HPP
#define PRESETS_MANAGER_HPP

#include <boost/filesystem.hpp>

class PresetsManager {
   public:
    PresetsManager();
    virtual ~PresetsManager();

    std::vector<std::string> get_names();

   private:
    std::string log_tag = "presets_manager: ";

    boost::filesystem::path presets_dir;
};

#endif
