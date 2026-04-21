/**
 * Copyright © 2017-2026 Wellington Wallace
 * Crosstalk Canceller plugin developed by Antti S. Lankila <alankila@bel.fi>
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

#include "crosstalk_canceller_preset.hpp"
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_crosstalk_canceller.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

CrosstalkCancellerPreset::CrosstalkCancellerPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<DbCrosstalkCanceller>(pipeline_type);
}

void CrosstalkCancellerPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["phantom-center-only"] = settings->phantomCenterOnly();

  json[section][instance_name]["delay-us"] = settings->delayUs();

  json[section][instance_name]["decay-db"] = settings->decayDb();
}

void CrosstalkCancellerPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("phantom-center-only", PhantomCenterOnly);
  UPDATE_PROPERTY("delay-us", DelayUs);
  UPDATE_PROPERTY("decay-db", DecayDb);
}
