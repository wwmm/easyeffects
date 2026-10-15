#include "harmonizer_preset.hpp"
#include "easyeffects_db_harmonizer.h"
#include "presets_macros.hpp"

HarmonizerPreset::HarmonizerPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<DbHarmonizer>(pipeline_type);
}

void HarmonizerPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();
  json[section][instance_name]["input-gain"] = settings->inputGain();
  json[section][instance_name]["output-gain"] = settings->outputGain();
  json[section][instance_name]["dry-wet"] = settings->dryWet();
  json[section][instance_name]["interval-left"] = settings->intervalLeft();
  json[section][instance_name]["interval-right"] = settings->intervalRight();
  json[section][instance_name]["note"] = settings->note();
  json[section][instance_name]["chord"] = settings->chord();
  json[section][instance_name]["chord-mode"] = settings->chordMode();
  json[section][instance_name]["gain-left"] = settings->gainLeft();
  json[section][instance_name]["gain-right"] = settings->gainRight();
}

void HarmonizerPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("dry-wet", DryWet);
  UPDATE_PROPERTY("interval-left", IntervalLeft);
  UPDATE_PROPERTY("interval-right", IntervalRight);
  UPDATE_PROPERTY("note", Note);
  UPDATE_PROPERTY("chord", Chord);
  UPDATE_PROPERTY("chord-mode", ChordMode);
  UPDATE_PROPERTY("gain-left", GainLeft);
  UPDATE_PROPERTY("gain-right", GainRight);
}
