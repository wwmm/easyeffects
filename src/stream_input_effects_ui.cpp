/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "stream_input_effects_ui.hpp"

#include <utility>

StreamInputEffectsUi::StreamInputEffectsUi(BaseObjectType* cobject,
                                           const Glib::RefPtr<Gtk::Builder>& refBuilder,
                                           Glib::RefPtr<Gtk::IconTheme> icon_ptr,
                                           StreamInputEffects* sie_ptr,
                                           const std::string& schema)
    : Gtk::Box(cobject), EffectsBaseUi(refBuilder, std::move(icon_ptr), sie_ptr, schema), sie(sie_ptr) {
  toggle_listen_mic->signal_toggled().connect([&, this]() { sie->set_listen_to_mic(toggle_listen_mic->get_active()); });

  connections.push_back(
      sie->pm->stream_input_changed.connect(sigc::mem_fun(*this, &StreamInputEffectsUi::on_app_changed)));
}

StreamInputEffectsUi::~StreamInputEffectsUi() {
  util::debug(log_tag + "destroyed");
}

auto StreamInputEffectsUi::add_to_stack(Gtk::Stack* stack,
                                        StreamInputEffects* sie_ptr,
                                        Glib::RefPtr<Gtk::IconTheme> icon_ptr) -> StreamInputEffectsUi* {
  const auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/effects_base.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<StreamInputEffectsUi>(builder, "top_box", icon_ptr, sie_ptr,
                                                                          "com.github.wwmm.easyeffects.streaminputs");

  auto stack_page = stack->add(*ui, "stream_input");

  stack_page->set_title(_("Input"));

  return ui;
}
