#include "source_output_effects_ui.hpp"

SourceOutputEffectsUi::SourceOutputEffectsUi(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder,
    const Glib::RefPtr<Gio::Settings>& refSettings,
    const std::shared_ptr<SourceOutputEffects>& soe_ptr)
    : Gtk::Box(cobject),
      EffectsBaseUi(refBuilder, refSettings, soe_ptr->pm),
      soe(soe_ptr) {
    // populate stack

    auto b_limiter = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/limiter.glade");
    auto b_compressor = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/compressor.glade");
    auto b_filter = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/filter.glade");
    auto b_equalizer = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/equalizer.glade");
    auto b_reverb = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/reverb.glade");
    auto b_gate = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/gate.glade");
    auto b_deesser = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/deesser.glade");
    auto b_pitch = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/pitch.glade");
    auto b_webrtc = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/webrtc.glade");

    b_limiter->get_widget_derived(
        "widgets_grid", limiter_ui,
        "com.github.wwmm.pulseeffects.sourceoutputs.limiter");
    b_compressor->get_widget_derived(
        "widgets_grid", compressor_ui,
        "com.github.wwmm.pulseeffects.sourceoutputs.compressor");
    b_filter->get_widget_derived(
        "widgets_grid", filter_ui,
        "com.github.wwmm.pulseeffects.sourceoutputs.filter");
    b_equalizer->get_widget_derived(
        "widgets_grid", equalizer_ui,
        "com.github.wwmm.pulseeffects.sourceoutputs.equalizer");
    b_reverb->get_widget_derived(
        "widgets_grid", reverb_ui,
        "com.github.wwmm.pulseeffects.sourceoutputs.reverb");
    b_gate->get_widget_derived(
        "widgets_grid", gate_ui,
        "com.github.wwmm.pulseeffects.sourceoutputs.gate");
    b_deesser->get_widget_derived(
        "widgets_grid", deesser_ui,
        "com.github.wwmm.pulseeffects.sourceoutputs.deesser");
    b_pitch->get_widget_derived(
        "widgets_grid", pitch_ui,
        "com.github.wwmm.pulseeffects.sourceoutputs.pitch");
    b_webrtc->get_widget_derived(
        "widgets_grid", webrtc_ui,
        "com.github.wwmm.pulseeffects.sourceoutputs.webrtc");

    stack->add(*limiter_ui, limiter_ui->name);
    stack->add(*compressor_ui, compressor_ui->name);
    stack->add(*filter_ui, filter_ui->name);
    stack->add(*equalizer_ui, equalizer_ui->name);
    stack->add(*reverb_ui, reverb_ui->name);
    stack->add(*gate_ui, gate_ui->name);
    stack->add(*deesser_ui, deesser_ui->name);
    stack->add(*pitch_ui, pitch_ui->name);
    stack->add(*webrtc_ui, webrtc_ui->name);

    // populate listbox

    add_to_listbox(limiter_ui);
    add_to_listbox(compressor_ui);
    add_to_listbox(filter_ui);
    add_to_listbox(equalizer_ui);
    add_to_listbox(reverb_ui);
    add_to_listbox(gate_ui);
    add_to_listbox(deesser_ui);
    add_to_listbox(pitch_ui);
    add_to_listbox(webrtc_ui);

    level_meters_connections();
    up_down_connections();
}

SourceOutputEffectsUi::~SourceOutputEffectsUi() {
    util::debug(log_tag + "destroyed");
}

void SourceOutputEffectsUi::level_meters_connections() {
    // limiter level meters connections

    connections.push_back(soe->limiter->input_level.connect(
        sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_input_level)));
    connections.push_back(soe->limiter->output_level.connect(
        sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_output_level)));
    connections.push_back(soe->limiter->attenuation.connect(
        sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_attenuation)));

    // compressor level meters connections

    connections.push_back(soe->compressor_input_level.connect(
        sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_input_level_db)));
    connections.push_back(soe->compressor_output_level.connect(
        sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_output_level_db)));
    connections.push_back(soe->compressor->compression.connect(
        sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_compression)));

    // filter level meters connections

    connections.push_back(soe->filter->input_level.connect(
        sigc::mem_fun(*filter_ui, &FilterUi::on_new_input_level)));
    connections.push_back(soe->filter->output_level.connect(
        sigc::mem_fun(*filter_ui, &FilterUi::on_new_output_level)));

    // equalizer level meters connections

    connections.push_back(soe->equalizer_input_level.connect(
        sigc::mem_fun(*equalizer_ui, &EqualizerUi::on_new_input_level_db)));
    connections.push_back(soe->equalizer_output_level.connect(
        sigc::mem_fun(*equalizer_ui, &EqualizerUi::on_new_output_level_db)));

    // reverb level meters connections

    connections.push_back(soe->reverb->input_level.connect(
        sigc::mem_fun(*reverb_ui, &ReverbUi::on_new_input_level)));
    connections.push_back(soe->reverb->output_level.connect(
        sigc::mem_fun(*reverb_ui, &ReverbUi::on_new_output_level)));

    // gate level meters connections

    connections.push_back(soe->gate_input_level.connect(
        sigc::mem_fun(*gate_ui, &GateUi::on_new_input_level_db)));
    connections.push_back(soe->gate_output_level.connect(
        sigc::mem_fun(*gate_ui, &GateUi::on_new_output_level_db)));
    connections.push_back(soe->gate->gating.connect(
        sigc::mem_fun(*gate_ui, &GateUi::on_new_gating)));

    // deesser level meters connections

    connections.push_back(soe->deesser_input_level.connect(
        sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_input_level_db)));
    connections.push_back(soe->deesser_output_level.connect(
        sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_output_level_db)));
    connections.push_back(soe->deesser->compression.connect(
        sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_compression)));
    connections.push_back(soe->deesser->detected.connect(
        sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_detected)));

    // pitch level meters connections

    connections.push_back(soe->pitch_input_level.connect(
        sigc::mem_fun(*pitch_ui, &PitchUi::on_new_input_level_db)));
    connections.push_back(soe->pitch_output_level.connect(
        sigc::mem_fun(*pitch_ui, &PitchUi::on_new_output_level_db)));

    // webrtc level meters connections

    connections.push_back(soe->webrtc_input_level.connect(
        sigc::mem_fun(*webrtc_ui, &WebrtcUi::on_new_input_level_db)));
    connections.push_back(soe->webrtc_output_level.connect(
        sigc::mem_fun(*webrtc_ui, &WebrtcUi::on_new_output_level_db)));
}

void SourceOutputEffectsUi::up_down_connections() {
    auto on_up = [=](auto p) {
        auto order = Glib::Variant<std::vector<std::string>>();

        settings->get_value("plugins", order);

        auto vorder = order.get();

        auto r = std::find(std::begin(vorder), std::end(vorder), p->name);

        if (r != std::begin(vorder)) {
            std::iter_swap(r, r - 1);

            settings->set_string_array("plugins", vorder);
        }
    };

    auto on_down = [=](auto p) {
        auto order = Glib::Variant<std::vector<std::string>>();

        settings->get_value("plugins", order);

        auto vorder = order.get();

        auto r = std::find(std::begin(vorder), std::end(vorder), p->name);

        if (r != std::end(vorder) - 1) {
            std::iter_swap(r, r + 1);

            settings->set_string_array("plugins", vorder);
        }
    };

    connections.push_back(limiter_ui->plugin_up->signal_clicked().connect(
        [=]() { on_up(limiter_ui); }));
    connections.push_back(limiter_ui->plugin_down->signal_clicked().connect(
        [=]() { on_down(limiter_ui); }));

    connections.push_back(compressor_ui->plugin_up->signal_clicked().connect(
        [=]() { on_up(compressor_ui); }));
    connections.push_back(compressor_ui->plugin_down->signal_clicked().connect(
        [=]() { on_down(compressor_ui); }));

    connections.push_back(filter_ui->plugin_up->signal_clicked().connect(
        [=]() { on_up(filter_ui); }));
    connections.push_back(filter_ui->plugin_down->signal_clicked().connect(
        [=]() { on_down(filter_ui); }));

    connections.push_back(equalizer_ui->plugin_up->signal_clicked().connect(
        [=]() { on_up(equalizer_ui); }));
    connections.push_back(equalizer_ui->plugin_down->signal_clicked().connect(
        [=]() { on_down(equalizer_ui); }));

    connections.push_back(reverb_ui->plugin_up->signal_clicked().connect(
        [=]() { on_up(reverb_ui); }));
    connections.push_back(reverb_ui->plugin_down->signal_clicked().connect(
        [=]() { on_down(reverb_ui); }));

    connections.push_back(gate_ui->plugin_up->signal_clicked().connect(
        [=]() { on_up(gate_ui); }));
    connections.push_back(gate_ui->plugin_down->signal_clicked().connect(
        [=]() { on_down(gate_ui); }));

    connections.push_back(deesser_ui->plugin_up->signal_clicked().connect(
        [=]() { on_up(deesser_ui); }));
    connections.push_back(deesser_ui->plugin_down->signal_clicked().connect(
        [=]() { on_down(deesser_ui); }));

    connections.push_back(pitch_ui->plugin_up->signal_clicked().connect(
        [=]() { on_up(pitch_ui); }));
    connections.push_back(pitch_ui->plugin_down->signal_clicked().connect(
        [=]() { on_down(pitch_ui); }));

    connections.push_back(webrtc_ui->plugin_up->signal_clicked().connect(
        [=]() { on_up(webrtc_ui); }));
    connections.push_back(webrtc_ui->plugin_down->signal_clicked().connect(
        [=]() { on_down(webrtc_ui); }));
}

void SourceOutputEffectsUi::reset() {
    settings->reset("plugins");

    limiter_ui->reset();
    compressor_ui->reset();
    filter_ui->reset();
    equalizer_ui->reset();
    reverb_ui->reset();
    gate_ui->reset();
    deesser_ui->reset();
    pitch_ui->reset();
    webrtc_ui->reset();
}
