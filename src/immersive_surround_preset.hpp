#pragma once

#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_immersive_surround.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"

class ImmersiveSurroundPreset : public PluginPresetBase {
 public:
  explicit ImmersiveSurroundPreset(PipelineType pipeline_type, const std::string& instance_name);

 private:
  db::ImmersiveSurround* settings = nullptr;

  void save(nlohmann::json& json) override;

  void load(const nlohmann::json& json) override;
};
