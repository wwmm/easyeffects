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

#include "plugin_preset_base.hpp"
#include <string>
#include <utility>
#include "pipeline_type.hpp"

PluginPresetBase::PluginPresetBase(PipelineType pipeline_type, std::string instance_name)
    : instance_name(std::move(instance_name)), pipeline_type(pipeline_type) {
  switch (pipeline_type) {
    case PipelineType::input:
      section = "input";
      break;
    case PipelineType::output:
      section = "output";
      break;
  }
}
