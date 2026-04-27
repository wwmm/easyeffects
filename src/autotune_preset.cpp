/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "autotune_preset.hpp"
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_autotune.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

AutotunePreset::AutotunePreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<DbAutotune>(pipeline_type);
}

void AutotunePreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["mode"] = settings->defaultModeLabelsValue()[settings->mode()].toStdString();

  json[section][instance_name]["tuning"] = settings->tuning();

  json[section][instance_name]["bias"] = settings->bias();

  json[section][instance_name]["filter"] = settings->filter();

  json[section][instance_name]["correction"] = settings->correction();

  json[section][instance_name]["offset"] = settings->offset();

  json[section][instance_name]["bend-range"] = settings->bendRange();

  json[section][instance_name]["channel-filter"] = settings->channelFilter();

  json[section][instance_name]["fast-mode"] = settings->fastMode();

  json[section][instance_name]["note-c"] = settings->noteC();

  json[section][instance_name]["note-c-sharp"] = settings->noteCSharp();

  json[section][instance_name]["note-d"] = settings->noteD();

  json[section][instance_name]["note-d-sharp"] = settings->noteDSharp();

  json[section][instance_name]["note-e"] = settings->noteE();

  json[section][instance_name]["note-f"] = settings->noteF();

  json[section][instance_name]["note-f-sharp"] = settings->noteFSharp();

  json[section][instance_name]["note-g"] = settings->noteG();

  json[section][instance_name]["note-g-sharp"] = settings->noteGSharp();

  json[section][instance_name]["note-a"] = settings->noteA();

  json[section][instance_name]["note-a-sharp"] = settings->noteASharp();

  json[section][instance_name]["note-b"] = settings->noteB();
}

void AutotunePreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("tuning", Tuning);
  UPDATE_PROPERTY("bias", Bias);
  UPDATE_PROPERTY("filter", Filter);
  UPDATE_PROPERTY("correction", Correction);
  UPDATE_PROPERTY("offset", Offset);
  UPDATE_PROPERTY("bend-range", BendRange);
  UPDATE_PROPERTY("channel-filter", ChannelFilter);
  UPDATE_PROPERTY("fast-mode", FastMode);
  UPDATE_PROPERTY("note-c", NoteC);
  UPDATE_PROPERTY("note-c-sharp", NoteCSharp);
  UPDATE_PROPERTY("note-d", NoteD);
  UPDATE_PROPERTY("note-d-sharp", NoteDSharp);
  UPDATE_PROPERTY("note-e", NoteE);
  UPDATE_PROPERTY("note-f", NoteF);
  UPDATE_PROPERTY("note-f-sharp", NoteFSharp);
  UPDATE_PROPERTY("note-g", NoteG);
  UPDATE_PROPERTY("note-g-sharp", NoteGSharp);
  UPDATE_PROPERTY("note-a", NoteA);
  UPDATE_PROPERTY("note-a-sharp", NoteASharp);
  UPDATE_PROPERTY("note-b", NoteB);

  UPDATE_ENUM_LIKE_PROPERTY("mode", Mode);
}
