#pragma once

#include <string>
#include "easyeffects_db_harmonizer.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"

class HarmonizerPreset : public PluginPresetBase {
 public:
  HarmonizerPreset(PipelineType pipeline_type, const std::string& instance_name);
  void save(nlohmann::json& json) override;
  void load(const nlohmann::json& json) override;

 private:
  DbHarmonizer* settings = nullptr;
};
