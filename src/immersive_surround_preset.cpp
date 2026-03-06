#include "immersive_surround_preset.hpp"
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_immersive_surround.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

ImmersiveSurroundPreset::ImmersiveSurroundPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::ImmersiveSurround>(pipeline_type);
}

void ImmersiveSurroundPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["surround-amount"] = settings->surroundAmount();

  json[section][instance_name]["stereo-width"] = settings->stereoWidth();

  json[section][instance_name]["rear-delay"] = settings->rearDelay();

  json[section][instance_name]["rear-mix"] = settings->rearMix();

  json[section][instance_name]["center-level"] = settings->centerLevel();

  json[section][instance_name]["damping"] = settings->damping();

  json[section][instance_name]["rear-phase-invert"] = settings->rearPhaseInvert();
}

void ImmersiveSurroundPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("surround-amount", SurroundAmount);
  UPDATE_PROPERTY("stereo-width", StereoWidth);
  UPDATE_PROPERTY("rear-delay", RearDelay);
  UPDATE_PROPERTY("rear-mix", RearMix);
  UPDATE_PROPERTY("center-level", CenterLevel);
  UPDATE_PROPERTY("damping", Damping);
  UPDATE_PROPERTY("rear-phase-invert", RearPhaseInvert);
}
