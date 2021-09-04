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

#include "effects_base_ui.hpp"

EffectsBaseUi::EffectsBaseUi(const Glib::RefPtr<Gtk::Builder>& builder,
                             EffectsBase* effects_base,
                             const std::string& schema)
    : effects_base(effects_base),
      schema(schema),
      settings(Gio::Settings::create(schema)),
      pm(effects_base->pm),
      players_model(Gio::ListStore<NodeInfoHolder>::create()),
      all_players_model(Gio::ListStore<NodeInfoHolder>::create()),
      blocklist(Gtk::StringList::create({"initial_value"})),
      plugins(Gtk::StringList::create({"initial_value"})),
      selected_plugins(Gtk::StringList::create({"initial_value"})) {
  // loading builder widgets

  global_output_level_left = builder->get_widget<Gtk::Label>("global_output_level_left");
  global_output_level_right = builder->get_widget<Gtk::Label>("global_output_level_right");
  device_state = builder->get_widget<Gtk::Label>("device_state");
  latency_status = builder->get_widget<Gtk::Label>("latency_status");
  saturation_icon = builder->get_widget<Gtk::Image>("saturation_icon");
  listview_players = builder->get_widget<Gtk::ListView>("listview_players");
  menubutton_blocklist = builder->get_widget<Gtk::MenuButton>("menubutton_blocklist");
  stack_top = builder->get_widget<Gtk::Stack>("stack_top");
  toggle_players = builder->get_widget<Gtk::ToggleButton>("toggle_players");
  toggle_plugins = builder->get_widget<Gtk::ToggleButton>("toggle_plugins");
  toggle_listen_mic = builder->get_widget<Gtk::ToggleButton>("toggle_listen_mic");

  popover_blocklist = builder->get_widget<Gtk::Popover>("popover_blocklist");
  blocklist_scrolled_window = builder->get_widget<Gtk::ScrolledWindow>("blocklist_scrolled_window");
  blocklist_player_name = builder->get_widget<Gtk::Text>("blocklist_player_name");
  button_add_to_blocklist = builder->get_widget<Gtk::Button>("button_add_to_blocklist");
  show_blocklisted_apps = builder->get_widget<Gtk::Switch>("show_blocklisted_apps");
  listview_blocklist = builder->get_widget<Gtk::ListView>("listview_blocklist");

  popover_plugins = builder->get_widget<Gtk::Popover>("popover_plugins");
  scrolled_window_plugins = builder->get_widget<Gtk::ScrolledWindow>("scrolled_window_plugins");
  listview_plugins = builder->get_widget<Gtk::ListView>("listview_plugins");
  listview_selected_plugins = builder->get_widget<Gtk::ListView>("listview_selected_plugins");
  entry_plugins_search = builder->get_widget<Gtk::SearchEntry>("entry_plugins_search");
  stack_plugins = builder->get_widget<Gtk::Stack>("stack_plugins");

  add_plugins_to_stack_plugins();

  // configuring widgets

  setup_listview_players();
  setup_listview_blocklist();
  setup_listview_plugins();
  setup_listview_selected_plugins();

  settings->signal_changed("plugins").connect([&, this](const auto& key) { add_plugins_to_stack_plugins(); });

  // spectrum

  auto* box_spectrum = builder->get_widget<Gtk::Box>("box_spectrum");

  spectrum_ui = SpectrumUi::add_to_box(box_spectrum);

  // gsettings

  settings->bind("show-blocklisted-apps", show_blocklisted_apps, "active");

  // signals connections

  stack_top->connect_property_changed("visible-child", [=, this]() {
    const auto& name = stack_top->get_visible_child_name();

    menubutton_blocklist->set_visible((name == "page_players") ? true : false);
  });

  toggle_players->signal_toggled().connect([&, this]() {
    if (toggle_players->get_active()) {
      stack_top->get_pages()->select_item(0, true);
    }
  });

  toggle_plugins->signal_toggled().connect([&, this]() {
    if (toggle_plugins->get_active()) {
      stack_top->get_pages()->select_item(1, true);
    }
  });

  button_add_to_blocklist->signal_clicked().connect([=, this]() {
    if (add_new_blocklist_entry(blocklist_player_name->get_text())) {
      blocklist_player_name->set_text("");
    }
  });

  show_blocklisted_apps->signal_state_set().connect(
      [=, this](const auto& state) {
        if (state) {
          players_model->remove_all();

          listview_players->set_model(nullptr);

          for (guint n = 0U; n < all_players_model->get_n_items(); n++) {
            players_model->append(all_players_model->get_item(n));
          }

          listview_players->set_model(Gtk::NoSelection::create(players_model));
        } else {
          players_model->remove_all();

          listview_players->set_model(nullptr);

          for (guint n = 0U; n < all_players_model->get_n_items(); n++) {
            if (const auto& item = all_players_model->get_item(n); !app_is_blocklisted(item->info.name)) {
              players_model->append(item);
            }
          }

          listview_players->set_model(Gtk::NoSelection::create(players_model));
        }

        return false;
      },
      false);

  popover_blocklist->signal_show().connect([=, this]() {
    const int height = static_cast<int>(0.5F * static_cast<float>(stack_top->get_allocated_height()));

    blocklist_scrolled_window->set_max_content_height(height);
  });

  popover_plugins->signal_show().connect([=, this]() {
    const int height = static_cast<int>(0.5F * static_cast<float>(stack_top->get_allocated_height()));

    scrolled_window_plugins->set_max_content_height(height);
  });

  // enabling notifications

  effects_base->autogain->post_messages = true;
  effects_base->bass_enhancer->post_messages = true;
  effects_base->bass_loudness->post_messages = true;
  effects_base->compressor->post_messages = true;
  effects_base->convolver->post_messages = true;
  effects_base->crossfeed->post_messages = true;
  effects_base->crystalizer->post_messages = true;
  effects_base->deesser->post_messages = true;
  effects_base->delay->post_messages = true;
  effects_base->echo_canceller->post_messages = true;
  effects_base->equalizer->post_messages = true;
  effects_base->exciter->post_messages = true;
  effects_base->filter->post_messages = true;
  effects_base->gate->post_messages = true;
  effects_base->limiter->post_messages = true;
  effects_base->loudness->post_messages = true;
  effects_base->maximizer->post_messages = true;
  effects_base->multiband_compressor->post_messages = true;
  effects_base->multiband_gate->post_messages = true;
  effects_base->output_level->post_messages = true;
  effects_base->pitch->post_messages = true;
  effects_base->reverb->post_messages = true;
  effects_base->rnnoise->post_messages = true;
  effects_base->spectrum->post_messages = true;
  effects_base->stereo_tools->post_messages = true;

  connections.emplace_back(effects_base->pipeline_latency.connect([=, this](const auto& v) {
    const auto& lv = Glib::ustring::format(std::setprecision(1), std::fixed, v);

    latency_status->set_text(lv + " ms" + Glib::ustring(5, ' '));
  }));

  const auto& lv = Glib::ustring::format(std::setprecision(1), std::fixed, effects_base->get_pipeline_latency());

  latency_status->set_text(lv + " ms" + Glib::ustring(5, ' '));
}

EffectsBaseUi::~EffectsBaseUi() {
  util::debug("effects_base_ui: destroyed");

  for (auto& c : connections) {
    c.disconnect();
  }

  // do not send notifications when the window is closed

  effects_base->autogain->post_messages = false;
  effects_base->bass_enhancer->post_messages = false;
  effects_base->bass_loudness->post_messages = false;
  effects_base->compressor->post_messages = false;
  effects_base->convolver->post_messages = false;
  effects_base->crossfeed->post_messages = false;
  effects_base->crystalizer->post_messages = false;
  effects_base->deesser->post_messages = false;
  effects_base->delay->post_messages = false;
  effects_base->echo_canceller->post_messages = false;
  effects_base->equalizer->post_messages = false;
  effects_base->exciter->post_messages = false;
  effects_base->filter->post_messages = false;
  effects_base->gate->post_messages = false;
  effects_base->limiter->post_messages = false;
  effects_base->loudness->post_messages = false;
  effects_base->maximizer->post_messages = false;
  effects_base->multiband_compressor->post_messages = false;
  effects_base->multiband_gate->post_messages = false;
  effects_base->output_level->post_messages = false;
  effects_base->pitch->post_messages = false;
  effects_base->reverb->post_messages = false;
  effects_base->rnnoise->post_messages = false;
  effects_base->spectrum->post_messages = false;
  effects_base->stereo_tools->post_messages = false;

  // disabling bypass when closing the window

  effects_base->autogain->bypass = false;
  effects_base->bass_enhancer->bypass = false;
  effects_base->bass_loudness->bypass = false;
  effects_base->compressor->bypass = false;
  effects_base->convolver->bypass = false;
  effects_base->crossfeed->bypass = false;
  effects_base->crystalizer->bypass = false;
  effects_base->deesser->bypass = false;
  effects_base->delay->bypass = false;
  effects_base->echo_canceller->bypass = false;
  effects_base->equalizer->bypass = false;
  effects_base->exciter->bypass = false;
  effects_base->filter->bypass = false;
  effects_base->gate->bypass = false;
  effects_base->limiter->bypass = false;
  effects_base->loudness->bypass = false;
  effects_base->maximizer->bypass = false;
  effects_base->multiband_compressor->bypass = false;
  effects_base->multiband_gate->bypass = false;
  effects_base->output_level->bypass = false;
  effects_base->pitch->bypass = false;
  effects_base->reverb->bypass = false;
  effects_base->rnnoise->bypass = false;
  effects_base->spectrum->bypass = false;
  effects_base->stereo_tools->bypass = false;
}

void EffectsBaseUi::add_plugins_to_stack_plugins() {
  std::string path = "/" + schema + "/";

  std::replace(path.begin(), path.end(), '.', '/');

  // removing plugins that are not in the list

  for (auto* child = stack_plugins->get_first_child(); child != nullptr;) {
    auto found = false;

    for (const auto& name : settings->get_string_array("plugins")) {
      if (name == stack_plugins->get_page(*child)->get_name()) {
        found = true;

        break;
      }
    }

    auto* next_child = child->get_next_sibling();

    if (!found) {
      stack_plugins->remove(*child);
    }

    child = next_child;
  }

  // Adding to the stack the plugins in the list that are not there yet

  for (const auto& name : settings->get_string_array("plugins")) {
    auto found = false;

    for (auto* child = stack_plugins->get_first_child(); child != nullptr; child = child->get_next_sibling()) {
      if (name == stack_plugins->get_page(*child)->get_name()) {
        found = true;

        break;
      }
    }

    if (found) {
      continue;
    }

    if (name == plugin_name::autogain) {
      auto* const autogain_ui = AutoGainUi::add_to_stack(stack_plugins, path);

      autogain_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->autogain->bypass = autogain_ui->bypass->get_active(); });

      effects_base->autogain->input_level.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_input_level));
      effects_base->autogain->output_level.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_output_level));
      effects_base->autogain->results.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_results));

      effects_base->autogain->bypass = false;
    } else if (name == plugin_name::bass_enhancer) {
      auto* const bass_enhancer_ui = BassEnhancerUi::add_to_stack(stack_plugins, path);

      bass_enhancer_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->bass_enhancer->bypass = bass_enhancer_ui->bypass->get_active(); });

      effects_base->bass_enhancer->input_level.connect(
          sigc::mem_fun(*bass_enhancer_ui, &BassEnhancerUi::on_new_input_level));
      effects_base->bass_enhancer->output_level.connect(
          sigc::mem_fun(*bass_enhancer_ui, &BassEnhancerUi::on_new_output_level));
      effects_base->bass_enhancer->harmonics.connect(
          sigc::mem_fun(*bass_enhancer_ui, &BassEnhancerUi::on_new_harmonics_level));

      effects_base->bass_enhancer->bypass = false;
    } else if (name == plugin_name::bass_loudness) {
      auto* const bass_loudness_ui = BassLoudnessUi::add_to_stack(stack_plugins, path);

      bass_loudness_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->bass_loudness->bypass = bass_loudness_ui->bypass->get_active(); });

      effects_base->bass_loudness->input_level.connect(
          sigc::mem_fun(*bass_loudness_ui, &BassLoudnessUi::on_new_input_level));
      effects_base->bass_loudness->output_level.connect(
          sigc::mem_fun(*bass_loudness_ui, &BassLoudnessUi::on_new_output_level));

      effects_base->bass_loudness->bypass = false;
    } else if (name == plugin_name::compressor) {
      auto* const compressor_ui = CompressorUi::add_to_stack(stack_plugins, path);

      compressor_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->compressor->bypass = compressor_ui->bypass->get_active(); });

      compressor_ui->set_pipe_manager_ptr(pm);

      effects_base->compressor->input_level.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_input_level));
      effects_base->compressor->output_level.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_output_level));
      effects_base->compressor->reduction.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_reduction));
      effects_base->compressor->envelope.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_envelope));
      effects_base->compressor->sidechain.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_sidechain));
      effects_base->compressor->curve.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_curve));

      effects_base->compressor->bypass = false;
    } else if (name == plugin_name::convolver) {
      auto* const convolver_ui = ConvolverUi::add_to_stack(stack_plugins, path);

      convolver_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->convolver->bypass = convolver_ui->bypass->get_active(); });

      convolver_ui->set_transient_window(transient_window);

      effects_base->convolver->input_level.connect(sigc::mem_fun(*convolver_ui, &ConvolverUi::on_new_input_level));
      effects_base->convolver->output_level.connect(sigc::mem_fun(*convolver_ui, &ConvolverUi::on_new_output_level));

      effects_base->convolver->bypass = false;
    } else if (name == plugin_name::crossfeed) {
      auto* const crossfeed_ui = CrossfeedUi::add_to_stack(stack_plugins, path);

      crossfeed_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->crossfeed->bypass = crossfeed_ui->bypass->get_active(); });

      effects_base->crossfeed->input_level.connect(sigc::mem_fun(*crossfeed_ui, &CrossfeedUi::on_new_input_level));
      effects_base->crossfeed->output_level.connect(sigc::mem_fun(*crossfeed_ui, &CrossfeedUi::on_new_output_level));

      effects_base->crossfeed->bypass = false;
    } else if (name == plugin_name::crystalizer) {
      auto* const crystalizer_ui = CrystalizerUi::add_to_stack(stack_plugins, path);

      crystalizer_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->crystalizer->bypass = crystalizer_ui->bypass->get_active(); });

      effects_base->crystalizer->input_level.connect(
          sigc::mem_fun(*crystalizer_ui, &CrystalizerUi::on_new_input_level));
      effects_base->crystalizer->output_level.connect(
          sigc::mem_fun(*crystalizer_ui, &CrystalizerUi::on_new_output_level));

      effects_base->crystalizer->bypass = false;
    } else if (name == plugin_name::deesser) {
      auto* const deesser_ui = DeesserUi::add_to_stack(stack_plugins, path);

      deesser_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->deesser->bypass = deesser_ui->bypass->get_active(); });

      effects_base->deesser->input_level.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_input_level));
      effects_base->deesser->output_level.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_output_level));
      effects_base->deesser->compression.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_compression));
      effects_base->deesser->detected.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_detected));

      effects_base->deesser->bypass = false;
    } else if (name == plugin_name::delay) {
      auto* const delay_ui = DelayUi::add_to_stack(stack_plugins, path);

      delay_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->delay->bypass = delay_ui->bypass->get_active(); });

      effects_base->delay->input_level.connect(sigc::mem_fun(*delay_ui, &DelayUi::on_new_input_level));
      effects_base->delay->output_level.connect(sigc::mem_fun(*delay_ui, &DelayUi::on_new_output_level));

      effects_base->delay->bypass = false;
    } else if (name == plugin_name::echo_canceller) {
      auto* const echo_canceller_ui = EchoCancellerUi::add_to_stack(stack_plugins, path);

      echo_canceller_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->echo_canceller->bypass = echo_canceller_ui->bypass->get_active(); });

      effects_base->echo_canceller->input_level.connect(
          sigc::mem_fun(*echo_canceller_ui, &EchoCancellerUi::on_new_input_level));
      effects_base->echo_canceller->output_level.connect(
          sigc::mem_fun(*echo_canceller_ui, &EchoCancellerUi::on_new_output_level));

      effects_base->echo_canceller->bypass = false;
    } else if (name == plugin_name::equalizer) {
      auto* const equalizer_ui = EqualizerUi::add_to_stack(stack_plugins, path);

      equalizer_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->equalizer->bypass = equalizer_ui->bypass->get_active(); });

      equalizer_ui->set_transient_window(transient_window);

      effects_base->equalizer->input_level.connect(sigc::mem_fun(*equalizer_ui, &EqualizerUi::on_new_input_level));
      effects_base->equalizer->output_level.connect(sigc::mem_fun(*equalizer_ui, &EqualizerUi::on_new_output_level));

      effects_base->equalizer->bypass = false;
    } else if (name == plugin_name::exciter) {
      auto* const exciter_ui = ExciterUi::add_to_stack(stack_plugins, path);

      exciter_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->exciter->bypass = exciter_ui->bypass->get_active(); });

      effects_base->exciter->input_level.connect(sigc::mem_fun(*exciter_ui, &ExciterUi::on_new_input_level));
      effects_base->exciter->output_level.connect(sigc::mem_fun(*exciter_ui, &ExciterUi::on_new_output_level));
      effects_base->exciter->harmonics.connect(sigc::mem_fun(*exciter_ui, &ExciterUi::on_new_harmonics_level));

      effects_base->exciter->bypass = false;
    } else if (name == plugin_name::filter) {
      auto* const filter_ui = FilterUi::add_to_stack(stack_plugins, path);

      filter_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->filter->bypass = filter_ui->bypass->get_active(); });

      effects_base->filter->input_level.connect(sigc::mem_fun(*filter_ui, &FilterUi::on_new_input_level));
      effects_base->filter->output_level.connect(sigc::mem_fun(*filter_ui, &FilterUi::on_new_output_level));

      effects_base->filter->bypass = false;
    } else if (name == plugin_name::gate) {
      auto* const gate_ui = GateUi::add_to_stack(stack_plugins, path);

      gate_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->gate->bypass = gate_ui->bypass->get_active(); });

      effects_base->gate->input_level.connect(sigc::mem_fun(*gate_ui, &GateUi::on_new_input_level));
      effects_base->gate->output_level.connect(sigc::mem_fun(*gate_ui, &GateUi::on_new_output_level));
      effects_base->gate->gating.connect(sigc::mem_fun(*gate_ui, &GateUi::on_new_gating));

      effects_base->gate->bypass = false;
    } else if (name == plugin_name::limiter) {
      auto* const limiter_ui = LimiterUi::add_to_stack(stack_plugins, path);

      limiter_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->limiter->bypass = limiter_ui->bypass->get_active(); });

      effects_base->limiter->input_level.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_input_level));
      effects_base->limiter->output_level.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_output_level));
      effects_base->limiter->gain_left.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_left_gain));
      effects_base->limiter->gain_right.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_right_gain));
      effects_base->limiter->sidechain_left.connect(
          sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_left_sidechain));
      effects_base->limiter->sidechain_right.connect(
          sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_right_sidechain));

      effects_base->limiter->bypass = false;
    } else if (name == plugin_name::loudness) {
      auto* const loudness_ui = LoudnessUi::add_to_stack(stack_plugins, path);

      loudness_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->loudness->bypass = loudness_ui->bypass->get_active(); });

      effects_base->loudness->input_level.connect(sigc::mem_fun(*loudness_ui, &LoudnessUi::on_new_input_level));
      effects_base->loudness->output_level.connect(sigc::mem_fun(*loudness_ui, &LoudnessUi::on_new_output_level));

      effects_base->loudness->bypass = false;
    } else if (name == plugin_name::maximizer) {
      auto* const maximizer_ui = MaximizerUi::add_to_stack(stack_plugins, path);

      maximizer_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->maximizer->bypass = maximizer_ui->bypass->get_active(); });

      effects_base->maximizer->input_level.connect(sigc::mem_fun(*maximizer_ui, &MaximizerUi::on_new_input_level));
      effects_base->maximizer->output_level.connect(sigc::mem_fun(*maximizer_ui, &MaximizerUi::on_new_output_level));
      effects_base->maximizer->reduction.connect(sigc::mem_fun(*maximizer_ui, &MaximizerUi::on_new_reduction));

      effects_base->maximizer->bypass = false;
    } else if (name == plugin_name::multiband_compressor) {
      auto* const multiband_compressor_ui = MultibandCompressorUi::add_to_stack(stack_plugins, path);

      multiband_compressor_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->multiband_compressor->bypass = multiband_compressor_ui->bypass->get_active(); });

      effects_base->multiband_compressor->input_level.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_input_level));
      effects_base->multiband_compressor->output_level.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output_level));

      effects_base->multiband_compressor->frequency_range.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_frequency_range));
      effects_base->multiband_compressor->envelope.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_envelope));
      effects_base->multiband_compressor->curve.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_curve));
      effects_base->multiband_compressor->reduction.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_reduction));

      effects_base->multiband_compressor->bypass = false;
    } else if (name == plugin_name::multiband_gate) {
      auto* const multiband_gate_ui = MultibandGateUi::add_to_stack(stack_plugins, path);

      multiband_gate_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->multiband_gate->bypass = multiband_gate_ui->bypass->get_active(); });

      effects_base->multiband_gate->input_level.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_input_level));
      effects_base->multiband_gate->output_level.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output_level));

      effects_base->multiband_gate->output0.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output0));
      effects_base->multiband_gate->output1.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output1));
      effects_base->multiband_gate->output2.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output2));
      effects_base->multiband_gate->output3.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output3));

      effects_base->multiband_gate->gating0.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating0));
      effects_base->multiband_gate->gating1.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating1));
      effects_base->multiband_gate->gating2.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating2));
      effects_base->multiband_gate->gating3.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating3));

      effects_base->multiband_gate->bypass = false;
    } else if (name == plugin_name::pitch) {
      auto* const pitch_ui = PitchUi::add_to_stack(stack_plugins, path);

      pitch_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->pitch->bypass = pitch_ui->bypass->get_active(); });

      effects_base->pitch->input_level.connect(sigc::mem_fun(*pitch_ui, &PitchUi::on_new_input_level));
      effects_base->pitch->output_level.connect(sigc::mem_fun(*pitch_ui, &PitchUi::on_new_output_level));

      effects_base->pitch->bypass = false;
    } else if (name == plugin_name::reverb) {
      auto* const reverb_ui = ReverbUi::add_to_stack(stack_plugins, path);

      reverb_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->reverb->bypass = reverb_ui->bypass->get_active(); });

      effects_base->reverb->input_level.connect(sigc::mem_fun(*reverb_ui, &ReverbUi::on_new_input_level));
      effects_base->reverb->output_level.connect(sigc::mem_fun(*reverb_ui, &ReverbUi::on_new_output_level));

      effects_base->reverb->bypass = false;
    } else if (name == plugin_name::rnnoise) {
      auto* const rnnoise_ui = RNNoiseUi::add_to_stack(stack_plugins, path);

      rnnoise_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->rnnoise->bypass = rnnoise_ui->bypass->get_active(); });

      rnnoise_ui->set_transient_window(transient_window);

      effects_base->rnnoise->input_level.connect(sigc::mem_fun(*rnnoise_ui, &RNNoiseUi::on_new_input_level));
      effects_base->rnnoise->output_level.connect(sigc::mem_fun(*rnnoise_ui, &RNNoiseUi::on_new_output_level));

      effects_base->rnnoise->bypass = false;
    } else if (name == plugin_name::stereo_tools) {
      auto* const stereo_tools_ui = StereoToolsUi::add_to_stack(stack_plugins, path);

      stereo_tools_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->stereo_tools->bypass = stereo_tools_ui->bypass->get_active(); });

      effects_base->stereo_tools->input_level.connect(
          sigc::mem_fun(*stereo_tools_ui, &StereoToolsUi::on_new_input_level));
      effects_base->stereo_tools->output_level.connect(
          sigc::mem_fun(*stereo_tools_ui, &StereoToolsUi::on_new_output_level));
      effects_base->stereo_tools->new_correlation.connect(
          sigc::mem_fun(*stereo_tools_ui, &StereoToolsUi::on_new_phase_correlation));

      effects_base->stereo_tools->bypass = false;
    }
  }
}

void EffectsBaseUi::setup_listview_players() {
  // setting the listview model and factory

  listview_players->set_model(Gtk::NoSelection::create(players_model));

  auto factory = Gtk::SignalListItemFactory::create();

  listview_players->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    const auto& b = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/app_info.ui");

    auto* const top_box = b->get_widget<Gtk::Box>("top_box");

    list_item->set_data("enable", b->get_widget<Gtk::Switch>("enable"));
    list_item->set_data("app_icon", b->get_widget<Gtk::Image>("app_icon"));
    list_item->set_data("app_name", b->get_widget<Gtk::Label>("app_name"));
    list_item->set_data("media_name", b->get_widget<Gtk::Label>("media_name"));
    list_item->set_data("blocklist", b->get_widget<Gtk::CheckButton>("blocklist"));
    list_item->set_data("format", b->get_widget<Gtk::Label>("format"));
    list_item->set_data("rate", b->get_widget<Gtk::Label>("rate"));
    list_item->set_data("channels", b->get_widget<Gtk::Label>("channels"));
    list_item->set_data("latency", b->get_widget<Gtk::Label>("latency"));
    list_item->set_data("state", b->get_widget<Gtk::Label>("state"));
    list_item->set_data("mute", b->get_widget<Gtk::ToggleButton>("mute"));
    list_item->set_data("scale_volume", b->get_widget<Gtk::Scale>("scale_volume"));
    list_item->set_data("volume", b->get_object<Gtk::Adjustment>("volume").get());

    list_item->set_child(*top_box);
  });

  factory->signal_bind().connect([=, this](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const app_name = static_cast<Gtk::Label*>(list_item->get_data("app_name"));
    auto* const media_name = static_cast<Gtk::Label*>(list_item->get_data("media_name"));
    auto* const format = static_cast<Gtk::Label*>(list_item->get_data("format"));
    auto* const rate = static_cast<Gtk::Label*>(list_item->get_data("rate"));
    auto* const channels = static_cast<Gtk::Label*>(list_item->get_data("channels"));
    auto* const latency = static_cast<Gtk::Label*>(list_item->get_data("latency"));
    auto* const state = static_cast<Gtk::Label*>(list_item->get_data("state"));
    auto* const enable = static_cast<Gtk::Switch*>(list_item->get_data("enable"));
    auto* const app_icon = static_cast<Gtk::Image*>(list_item->get_data("app_icon"));
    auto* const scale_volume = static_cast<Gtk::Scale*>(list_item->get_data("scale_volume"));
    auto* const volume = static_cast<Gtk::Adjustment*>(list_item->get_data("volume"));
    auto* const mute = static_cast<Gtk::ToggleButton*>(list_item->get_data("mute"));
    auto* const blocklist = static_cast<Gtk::CheckButton*>(list_item->get_data("blocklist"));

    auto holder = std::dynamic_pointer_cast<NodeInfoHolder>(list_item->get_item());

    auto connection_enable = enable->signal_state_set().connect(
        [=, this](const auto& state) {
          if (!app_is_blocklisted(holder->info.name)) {
            (state) ? connect_stream(holder->info) : disconnect_stream(holder->info);

            enabled_app_list.insert_or_assign(holder->info.id, state);
          }

          holder->info_updated.emit(holder->info);

          return false;
        },
        false);

    auto connection_volume = volume->signal_value_changed().connect(
        [=]() { PipeManager::set_node_volume(holder->info, static_cast<float>(volume->get_value()) / 100.0F); });

    auto connection_mute = mute->signal_toggled().connect([=]() {
      const auto& state = mute->get_active();

      if (state) {
        mute->property_icon_name().set_value("audio-volume-muted-symbolic");

        scale_volume->set_sensitive(false);
      } else {
        mute->property_icon_name().set_value("audio-volume-high-symbolic");

        scale_volume->set_sensitive(true);
      }

      PipeManager::set_node_mute(holder->info, state);
    });

    auto connection_blocklist_checkbutton = blocklist->signal_toggled().connect([=, this]() {
      if (blocklist->get_active()) {
        enabled_app_list.insert_or_assign(holder->info.id, enable->get_active());

        add_new_blocklist_entry(holder->info.name);
      } else {
        remove_blocklist_entry(holder->info.name);
      }

      // Stream connection/disconnection will be done in blocklist items signal changed
    });

    auto* pointer_connection_enable = new sigc::connection(connection_enable);
    auto* pointer_connection_volume = new sigc::connection(connection_volume);
    auto* pointer_connection_mute = new sigc::connection(connection_mute);
    auto* pointer_connection_blocklist_checkbutton = new sigc::connection(connection_blocklist_checkbutton);

    auto connection_info = holder->info_updated.connect([=, this](NodeInfo i) {
      app_name->set_text(i.name);
      media_name->set_text(i.media_name);
      format->set_text(i.format);
      rate->set_text(Glib::ustring::format(i.rate) + " Hz");
      channels->set_text(Glib::ustring::format(i.n_volume_channels));
      latency->set_text(Glib::ustring::format(std::setprecision(2), std::fixed, i.latency) + " s");
      state->set_text(node_state_to_ustring(i.state));

      // set the enable switch

      pointer_connection_enable->block();

      const auto& is_enabled = pm->stream_is_connected(holder->info.media_class, holder->info.id);
      const auto& is_blocklisted = app_is_blocklisted(holder->info.name);

      enable->set_sensitive(is_enabled || !is_blocklisted);
      enable->set_active(is_enabled);

      pointer_connection_enable->unblock();

      // set the blocklist checkbutton

      pointer_connection_blocklist_checkbutton->block();

      blocklist->set_active(is_blocklisted);

      pointer_connection_blocklist_checkbutton->unblock();

      // save app enabled state only the first time when is not preset in the enabled_app_list map

      enabled_app_list.try_emplace(holder->info.id, is_enabled);

      // set the icon name and the volume slider

      if (i.state != PW_NODE_STATE_CREATING) {
        // PW_NODE_STATE_CREATING causes icon name to be set even if it's empty,
        // showing a broken image. Therefore we skip it's update on that state.

        if (!i.app_icon_name.empty()) {
          app_icon->set_from_icon_name(i.app_icon_name);

          app_icon->set_visible(true);
        } else if (!i.media_icon_name.empty()) {
          app_icon->set_from_icon_name(i.media_icon_name);

          app_icon->set_visible(true);
        } else {
          app_icon->set_from_icon_name(Glib::ustring(i.name).lowercase());

          app_icon->set_visible(true);
        }

        // Volume can't be set on PW_NODE_STATE_CREATING, so we leave it hidden on this state

        pointer_connection_volume->block();

        scale_volume->set_visible(true);

        scale_volume->set_sensitive(true);

        volume->set_value(100 * holder->info.volume);

        pointer_connection_volume->unblock();
      }

      // set the mute button

      pointer_connection_mute->block();

      if (holder->info.mute) {
        mute->property_icon_name().set_value("audio-volume-muted-symbolic");

        scale_volume->set_sensitive(false);
      } else {
        mute->property_icon_name().set_value("audio-volume-high-symbolic");

        scale_volume->set_sensitive(true);
      }

      mute->set_active(holder->info.mute);

      pointer_connection_mute->unblock();
    });

    scale_volume->set_format_value_func([=](const auto& v) {
      return Glib::ustring::format(static_cast<int>(v)) + " %";
    });

    list_item->set_data("connection_enable", pointer_connection_enable, Glib::destroy_notify_delete<sigc::connection>);

    list_item->set_data("connection_volume", pointer_connection_volume, Glib::destroy_notify_delete<sigc::connection>);

    list_item->set_data("connection_mute", pointer_connection_mute, Glib::destroy_notify_delete<sigc::connection>);

    list_item->set_data("connection_blocklist_checkbutton", pointer_connection_blocklist_checkbutton,
                        Glib::destroy_notify_delete<sigc::connection>);

    list_item->set_data("connection_info", new sigc::connection(connection_info),
                        Glib::destroy_notify_delete<sigc::connection>);
  });

  factory->signal_unbind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    const auto& connections_list = {
      "connection_enable", "connection_volume", "connection_mute",
      "connection_blocklist_checkbutton", "connection_info"
    };

    for (const auto* conn : connections_list) {
      if (auto* connection = static_cast<sigc::connection*>(list_item->get_data(conn))) {
        connection->disconnect();

        list_item->set_data(conn, nullptr);
      }
    }
  });
}

void EffectsBaseUi::connect_stream(const NodeInfo& node_info) {
  if (node_info.media_class == "Stream/Output/Audio") {
    pm->connect_stream_output(node_info);
  } else if (node_info.media_class == "Stream/Input/Audio") {
    pm->connect_stream_input(node_info);
  }
}

void EffectsBaseUi::disconnect_stream(const NodeInfo& node_info) {
  if (node_info.media_class == "Stream/Output/Audio") {
    pm->disconnect_stream_output(node_info);
  } else if (node_info.media_class == "Stream/Input/Audio") {
    pm->disconnect_stream_input(node_info);
  }
}

void EffectsBaseUi::setup_listview_blocklist() {
  blocklist->remove(0);

  for (const auto& name : settings->get_string_array("blocklist")) {
    blocklist->append(name);
  }

  settings->signal_changed("blocklist").connect([=, this](const auto& key) {
    const auto& list = settings->get_string_array(key);

    blocklist->splice(0, blocklist->get_n_items(), list);
  });

  blocklist->signal_items_changed().connect([=, this](const guint& position, const guint& removed, const guint& added) {
    util::debug("blocklist->signal_items_changed");

    if (removed > 0U) {
      // Some items removed from the blocklist, so the listview_players might show an item hidden before

      players_model->remove_all();

      listview_players->set_model(nullptr);

      for (guint n = 0U; n < all_players_model->get_n_items(); n++) {
        players_model->append(all_players_model->get_item(n));
      }

      listview_players->set_model(Gtk::NoSelection::create(players_model));
    }

    // Updating the enabled state of items added/removed to/from blocklist

    for (guint n = 0U; n < all_players_model->get_n_items(); n++) {
      auto holder = std::dynamic_pointer_cast<NodeInfoHolder>(all_players_model->get_item(n));

      const auto& app_is_enabled = pm->stream_is_connected(holder->info.media_class, holder->info.id);

      if (app_is_blocklisted(holder->info.name)) {
        if (app_is_enabled) {
          disconnect_stream(holder->info);
        }
      } else if (!app_is_enabled) {
        // Try to restore the previous enabled state, if needed

        try {
          if (enabled_app_list.at(holder->info.id)) {
            connect_stream(holder->info);
          }
        } catch (...) {
          connect_stream(holder->info);

          util::warning("Can't retrieve enabled state of node " + std::to_string(holder->info.id));

          enabled_app_list.insert_or_assign(holder->info.id, true);
        }
      }

      // enabled switch state will be updated in holder info_updated signal handler

      holder->info_updated.emit(holder->info);
    }
  });

  // sorter

  const auto& sorter =
      Gtk::StringSorter::create(Gtk::PropertyExpression<Glib::ustring>::create(GTK_TYPE_STRING_OBJECT, "string"));

  const auto& sort_list_model = Gtk::SortListModel::create(blocklist, sorter);

  // setting the listview model and factory

  listview_blocklist->set_model(Gtk::NoSelection::create(sort_list_model));

  auto factory = Gtk::SignalListItemFactory::create();

  listview_blocklist->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const box = Gtk::make_managed<Gtk::Box>();
    auto* const label = Gtk::make_managed<Gtk::Label>();
    auto* const btn = Gtk::make_managed<Gtk::Button>();

    label->set_hexpand(true);
    label->set_halign(Gtk::Align::START);

    btn->set_icon_name("user-trash-symbolic");

    box->append(*label);
    box->append(*btn);

    list_item->set_data("name", label);
    list_item->set_data("remove", btn);

    list_item->set_child(*box);
  });

  factory->signal_bind().connect([=, this](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const label = static_cast<Gtk::Label*>(list_item->get_data("name"));
    auto* const remove = static_cast<Gtk::Button*>(list_item->get_data("remove"));

    const auto& name = list_item->get_item()->get_property<Glib::ustring>("string");

    label->set_text(name);

    auto connection_remove = remove->signal_clicked().connect([=, this]() {
      auto list = settings->get_string_array("blocklist");

      list.erase(std::remove_if(list.begin(), list.end(), [=](const auto& player_name) { return player_name == name; }),
                 list.end());

      settings->set_string_array("blocklist", list);
    });

    list_item->set_data("connection_remove", new sigc::connection(connection_remove),
                        Glib::destroy_notify_delete<sigc::connection>);
  });

  factory->signal_unbind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    if (auto* connection = static_cast<sigc::connection*>(list_item->get_data("connection_remove"))) {
      connection->disconnect();

      list_item->set_data("connection_remove", nullptr);
    }
  });
}

void EffectsBaseUi::setup_listview_plugins() {
  plugins->remove(0);

  for (const auto& translated_name : std::views::values(plugins_names)) {
    plugins->append(translated_name);
  }

  // filter

  auto filter =
      Gtk::StringFilter::create(Gtk::PropertyExpression<Glib::ustring>::create(GTK_TYPE_STRING_OBJECT, "string"));

  auto filter_model = Gtk::FilterListModel::create(plugins, filter);

  filter_model->set_incremental(true);

  Glib::Binding::bind_property(entry_plugins_search->property_text(), filter->property_search());

  // sorter

  const auto& sorter =
      Gtk::StringSorter::create(Gtk::PropertyExpression<Glib::ustring>::create(GTK_TYPE_STRING_OBJECT, "string"));

  const auto& sort_list_model = Gtk::SortListModel::create(filter_model, sorter);

  // setting the listview model and factory

  listview_plugins->set_model(Gtk::NoSelection::create(sort_list_model));

  auto factory = Gtk::SignalListItemFactory::create();

  listview_plugins->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const box = Gtk::make_managed<Gtk::Box>();
    auto* const label = Gtk::make_managed<Gtk::Label>();
    auto* const btn = Gtk::make_managed<Gtk::Button>();

    label->set_hexpand(true);
    label->set_halign(Gtk::Align::START);

    btn->set_icon_name("list-add-symbolic");

    box->append(*label);
    box->append(*btn);

    list_item->set_data("name", label);
    list_item->set_data("add", btn);

    list_item->set_child(*box);
  });

  factory->signal_bind().connect([=, this](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const label = static_cast<Gtk::Label*>(list_item->get_data("name"));
    auto* const add = static_cast<Gtk::Button*>(list_item->get_data("add"));

    const auto& translated_name = list_item->get_item()->get_property<Glib::ustring>("string");

    Glib::ustring key_name;

    for (const auto& [key, value] : plugins_names) {
      if (translated_name == value.c_str()) {
        key_name = key;
      }
    }

    label->set_text(translated_name);

    auto connection_add = add->signal_clicked().connect([=, this]() {
      auto list = settings->get_string_array("plugins");

      if (std::ranges::find(list, key_name) == list.end()) {
        list.emplace_back(key_name);

        settings->set_string_array("plugins", list);
      }
    });

    list_item->set_data("connection_add", new sigc::connection(connection_add),
                        Glib::destroy_notify_delete<sigc::connection>);
  });

  factory->signal_unbind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    if (auto* connection = static_cast<sigc::connection*>(list_item->get_data("connection_add"))) {
      connection->disconnect();

      list_item->set_data("connection_add", nullptr);
    }
  });
}

void EffectsBaseUi::setup_listview_selected_plugins() {
  selected_plugins->remove(0);

  if (!settings->get_string_array("plugins").empty()) {
    for (const auto& name : settings->get_string_array("plugins")) {
      selected_plugins->append(name);
    }

    // showing the first plugin in the list by default

    const auto& selected_name = selected_plugins->get_string(0);

    for (auto* child = stack_plugins->get_first_child(); child != nullptr; child = child->get_next_sibling()) {
      if (stack_plugins->get_page(*child)->get_name() == selected_name) {
        stack_plugins->set_visible_child(*child);

        break;
      }
    }
  }

  settings->signal_changed("plugins").connect([=, this](const auto& key) {
    const auto& list = settings->get_string_array(key);

    selected_plugins->splice(0, selected_plugins->get_n_items(), list);

    if (!list.empty()) {
      auto* visible_child = stack_plugins->get_visible_child();

      if (visible_child == nullptr) {
        return;
      }

      const auto& visible_page_name = stack_plugins->get_page(*visible_child)->get_name();

      if (std::ranges::find(list, visible_page_name) == list.end()) {
        listview_selected_plugins->get_model()->select_item(0, true);

        for (auto* child = stack_plugins->get_first_child(); child != nullptr; child = child->get_next_sibling()) {
          if (stack_plugins->get_page(*child)->get_name() == list[0]) {
            stack_plugins->set_visible_child(*child);
          }
        }
      } else {
        for (size_t m = 0U, list_size = list.size(); m < list_size; m++) {
          if (list[m] == visible_page_name) {
            listview_selected_plugins->get_model()->select_item(m, true);

            break;
          }
        }
      }
    }
  });

  // setting the listview model and factory

  listview_selected_plugins->set_model(Gtk::SingleSelection::create(selected_plugins));

  auto factory = Gtk::SignalListItemFactory::create();

  listview_selected_plugins->set_factory(factory);

  // setting the item selection callback

  listview_selected_plugins->get_model()->signal_selection_changed().connect([&](guint position, guint n_items) {
    auto single = std::dynamic_pointer_cast<Gtk::SingleSelection>(listview_selected_plugins->get_model());

    const auto& selected_name = single->get_selected_item()->get_property<Glib::ustring>("string");

    for (auto* child = stack_plugins->get_first_child(); child != nullptr; child = child->get_next_sibling()) {
      if (stack_plugins->get_page(*child)->get_name() == selected_name) {
        stack_plugins->set_visible_child(*child);

        return;
      }
    }
  });

  // setting the factory callbacks

  factory->signal_setup().connect([=, this](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const box = Gtk::make_managed<Gtk::Box>();
    auto* const label = Gtk::make_managed<Gtk::Label>();
    auto* const remove = Gtk::make_managed<Gtk::Button>();
    auto* const drag_handle = Gtk::make_managed<Gtk::Image>();
    auto* const plugin_icon = Gtk::make_managed<Gtk::Image>();

    label->set_hexpand(true);
    label->set_halign(Gtk::Align::START);

    remove->set_icon_name("user-trash-symbolic");
    remove->set_css_classes({"flat"});

    drag_handle->set_from_icon_name("list-drag-handle-symbolic");

    plugin_icon->set_from_icon_name("emblem-system-symbolic");
    plugin_icon->set_margin_start(6);
    plugin_icon->set_margin_end(6);

    box->set_spacing(6);
    box->append(*plugin_icon);
    box->append(*label);
    box->append(*remove);
    box->append(*drag_handle);

    remove->set_opacity(0);
    drag_handle->set_opacity(0);

    auto controller = Gtk::EventControllerMotion::create();

    controller->signal_enter().connect([=](const double& x, const double& y) {
      remove->set_opacity(1);
      drag_handle->set_opacity(1);
    });

    controller->signal_leave().connect([=]() {
      remove->set_opacity(0);
      drag_handle->set_opacity(0);
    });

    box->add_controller(controller);

    // drag and drop

    auto drag_source = Gtk::DragSource::create();
    auto drop_target = Gtk::DropTarget::create(Glib::Value<Glib::ustring>::value_type(), Gdk::DragAction::MOVE);

    drag_source->set_actions(Gdk::DragAction::MOVE);

    drag_source->signal_prepare().connect(
        [=](const double& x, const double& y) {
          auto* const controller_widget = drag_source->get_widget();

          auto* const item = controller_widget->get_ancestor(Gtk::Box::get_type());

          controller_widget->set_data("dragged-item", item);

          Glib::Value<Glib::RefPtr<const Gtk::Label>> texture_value;

          Glib::Value<Glib::ustring> name_value;

          name_value.init(Glib::Value<Glib::ustring>::value_type());

          name_value.set(label->get_name());

          return Gdk::ContentProvider::create(name_value);
        },
        false);

    drag_source->signal_drag_begin().connect([=](const Glib::RefPtr<Gdk::Drag>& drag) {
      auto* controller_widget = drag_source->get_widget();

      auto* row_box = static_cast<Gtk::Box*>(controller_widget->get_data("dragged-item"));

      const auto& paintable = Gtk::WidgetPaintable::create(*row_box);

      drag_source->set_icon(paintable, row_box->get_allocated_width() - controller_widget->get_allocated_width() / 2,
                            row_box->get_allocated_height() / 2);

      controller_widget->set_data("dragged-item", nullptr);
    });

    drop_target->signal_drop().connect(
        [=, this](const Glib::ValueBase& v, const double& x, const double& y) {
          Glib::Value<Glib::ustring> name_value;

          name_value.init(v.gobj());

          const auto& src = name_value.get();
          const auto& dst = label->get_name();

          if (src != dst) {
            auto list = settings->get_string_array("plugins");

            const auto& iter_src = std::ranges::find(list, src);
            auto iter_dst = std::ranges::find(list, dst);

            const auto& insert_after = (iter_src - list.begin() < iter_dst - list.begin()) ? true : false;

            list.erase(iter_src);

            iter_dst = std::ranges::find(list, dst);

            list.insert(((insert_after) ? (iter_dst + 1) : iter_dst), src);

            settings->set_string_array("plugins", list);

            return true;
          }

          return false;
        },
        false);

    drag_handle->add_controller(drag_source);
    box->add_controller(drop_target);

    // setting list_item data

    list_item->set_data("name", label);
    list_item->set_data("remove", remove);

    list_item->set_child(*box);
  });

  factory->signal_bind().connect([=, this](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const label = static_cast<Gtk::Label*>(list_item->get_data("name"));
    auto* const remove = static_cast<Gtk::Button*>(list_item->get_data("remove"));

    const auto& name = list_item->get_item()->get_property<Glib::ustring>("string");

    label->set_name(name);
    label->set_text(plugins_names[name]);

    auto connection_remove = remove->signal_clicked().connect([=, this]() {
      auto list = settings->get_string_array("plugins");

      list.erase(std::remove_if(list.begin(), list.end(), [=](const auto& plugin_name) { return plugin_name == name; }),
                 list.end());

      settings->set_string_array("plugins", list);
    });

    // setting list_item data

    list_item->set_data("connection_remove", new sigc::connection(connection_remove),
                        Glib::destroy_notify_delete<sigc::connection>);
  });

  factory->signal_unbind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    for (const auto* conn : {"connection_remove"}) {
      if (auto* connection = static_cast<sigc::connection*>(list_item->get_data(conn))) {
        connection->disconnect();

        list_item->set_data(conn, nullptr);
      }
    }
  });
}

void EffectsBaseUi::on_app_added(NodeInfo node_info) {
  // do not add the same stream twice

  for (guint n = 0U; n < all_players_model->get_n_items(); n++) {
    if (all_players_model->get_item(n)->info.id == node_info.id) {
      return;
    }
  }

  all_players_model->append(NodeInfoHolder::create(node_info));

  // Blocklist check

  if (app_is_blocklisted(node_info.name) && !settings->get_boolean("show-blocklisted-apps")) {
    return;
  }

  players_model->append(NodeInfoHolder::create(node_info));
}

void EffectsBaseUi::on_app_changed(NodeInfo node_info) {
  for (guint n = 0U; n < players_model->get_n_items(); n++) {
    if (auto* item = players_model->get_item(n).get(); item->info.id == node_info.id) {
      item->info = node_info;

      item->info_updated.emit(node_info);

      break;
    }
  }
}

void EffectsBaseUi::on_app_removed(NodeInfo node_info) {
  for (guint n = 0U; n < players_model->get_n_items(); n++) {
    if (players_model->get_item(n)->info.id == node_info.id) {
      players_model->remove(n);

      break;
    }
  }

  for (guint n = 0U; n < all_players_model->get_n_items(); n++) {
    if (all_players_model->get_item(n)->info.id == node_info.id) {
      all_players_model->remove(n);

      break;
    }
  }
}

void EffectsBaseUi::on_new_output_level_db(const float& left, const float& right) {
  global_output_level_left->set_text(((left > 0.0) ? "+" : "") +
      Glib::ustring::format(std::setprecision(0), std::fixed, left));

  global_output_level_right->set_text(((right > 0.0) ? "+" : "") +
      Glib::ustring::format(std::setprecision(0), std::fixed, right));

  // saturation icon notification

  saturation_icon->set_opacity((left > 0.0 || right > 0.0) ? 1.0 : 0.0);
}

auto EffectsBaseUi::node_state_to_ustring(const pw_node_state& state) -> Glib::ustring {
  switch (state) {
    case PW_NODE_STATE_RUNNING:
      return _("running");
    case PW_NODE_STATE_SUSPENDED:
      return _("suspended");
    case PW_NODE_STATE_IDLE:
      return _("idle");
    case PW_NODE_STATE_CREATING:
      return _("creating");
    case PW_NODE_STATE_ERROR:
      return _("error");
    default:
      return _("unknown");
  }
}

auto EffectsBaseUi::app_is_blocklisted(const Glib::ustring& name) -> bool {
  const auto& bl = settings->get_string_array("blocklist");

  return std::ranges::find(bl, name) != bl.end();
}

auto EffectsBaseUi::add_new_blocklist_entry(const Glib::ustring& name) -> bool {
  if (name.empty()) {
    return false;
  }

  auto bl = settings->get_string_array("blocklist");

  if (std::any_of(bl.cbegin(), bl.cend(), [&](const auto& str) { return str == name; })) {
    util::debug("blocklist_settings_ui: entry already present in the list");

    return false;
  }

  bl.emplace_back(name);

  settings->set_string_array("blocklist", bl);

  util::debug("blocklist_settings_ui: new entry has been added to the blocklist");

  return true;
}

void EffectsBaseUi::remove_blocklist_entry(const Glib::ustring& name) {
  auto bl = settings->get_string_array("blocklist");

  bl.erase(std::remove_if(bl.begin(), bl.end(), [=](const auto& a) { return a == name; }), bl.end());

  settings->set_string_array("blocklist", bl);

  util::debug("blocklist_settings_ui: an entry has been removed from the blocklist");
}

void EffectsBaseUi::set_transient_window(Gtk::Window* transient_window) {
  this->transient_window = transient_window;

  for (auto* child = stack_plugins->get_first_child(); child != nullptr; child = child->get_next_sibling()) {
    const auto& page = stack_plugins->get_page(*child);

    if (page->get_name() == plugin_name::equalizer) {
      dynamic_cast<EqualizerUi*>(child)->set_transient_window(transient_window);
    } else if (page->get_name() == plugin_name::rnnoise) {
      dynamic_cast<RNNoiseUi*>(child)->set_transient_window(transient_window);
    } else if (page->get_name() == plugin_name::convolver) {
      dynamic_cast<ConvolverUi*>(child)->set_transient_window(transient_window);
    }
  }
}
