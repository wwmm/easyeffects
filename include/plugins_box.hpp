/*
 *  Copyright © 2017-2023 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "application.hpp"
#include "autogain_ui.hpp"
#include "bass_enhancer_ui.hpp"
#include "bass_loudness_ui.hpp"
#include "compressor_ui.hpp"
#include "convolver_ui.hpp"
#include "crossfeed_ui.hpp"
#include "crystalizer_ui.hpp"
#include "deesser_ui.hpp"
#include "delay_ui.hpp"
#include "echo_canceller_ui.hpp"
#include "equalizer_ui.hpp"
#include "exciter_ui.hpp"
#include "filter_ui.hpp"
#include "gate_ui.hpp"
#include "limiter_ui.hpp"
#include "loudness_ui.hpp"
#include "maximizer_ui.hpp"
#include "multiband_compressor_ui.hpp"
#include "multiband_gate_ui.hpp"
#include "pipeline_type.hpp"
#include "pitch_ui.hpp"
#include "plugins_menu.hpp"
#include "reverb_ui.hpp"
#include "rnnoise_ui.hpp"
#include "speex_ui.hpp"
#include "stereo_tools_ui.hpp"

namespace ui::plugins_box {

G_BEGIN_DECLS

#define EE_TYPE_PLUGINS_BOX (plugins_box_get_type())

G_DECLARE_FINAL_TYPE(PluginsBox, plugins_box, EE, PLUGINS_BOX, GtkBox)

G_END_DECLS

auto create() -> PluginsBox*;

void setup(PluginsBox* self, app::Application* application, PipelineType pipeline_type);

}  // namespace ui::plugins_box