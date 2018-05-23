#include "source_output_effects_ui.hpp"

SourceOutputEffectsUi::SourceOutputEffectsUi(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder,
    std::shared_ptr<SourceOutputEffects> soe_ptr)
    : EffectsBaseUi(cobject, refBuilder, soe_ptr->pm),
      soe(soe_ptr),
      settings(
          Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs")),
      limiter_ui(LimiterUi::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.limiter")),
      compressor_ui(CompressorUi::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.compressor")),
      filter_ui(FilterUi::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.filter")),
      equalizer_ui(EqualizerUi::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.equalizer")),
      reverb_ui(ReverbUi::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.reverb")) {
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

    add_plugins();
}

SourceOutputEffectsUi::~SourceOutputEffectsUi() {
    for (auto c : connections) {
        c.disconnect();
    }
}

std::unique_ptr<SourceOutputEffectsUi> SourceOutputEffectsUi::create(
    std::shared_ptr<SourceOutputEffects> soe) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/effects_base.glade");

    SourceOutputEffectsUi* soe_ui = nullptr;

    builder->get_widget_derived("widgets_box", soe_ui, soe);

    return std::unique_ptr<SourceOutputEffectsUi>(soe_ui);
}

void SourceOutputEffectsUi::add_plugins() {
    auto plugins_order = Glib::Variant<std::vector<std::string>>();

    settings->get_value("plugins", plugins_order);

    for (auto name : plugins_order.get()) {
        if (name == std::string("limiter")) {
            add_to_listbox(limiter_ui);
            stack->add(*limiter_ui, std::string("limiter"));
        } else if (name == std::string("compressor")) {
            add_to_listbox(compressor_ui);
            stack->add(*compressor_ui, std::string("compressor"));
        } else if (name == std::string("filter")) {
            add_to_listbox(filter_ui);
            stack->add(*filter_ui, std::string("filter"));
        } else if (name == std::string("equalizer")) {
            add_to_listbox(equalizer_ui);
            stack->add(*equalizer_ui, std::string("equalizer"));
        } else if (name == std::string("reverb")) {
            add_to_listbox(reverb_ui);
            stack->add(*reverb_ui, std::string("reverb"));
        }
    }
}

void SourceOutputEffectsUi::reset() {
    limiter_ui->reset();
    compressor_ui->reset();
    filter_ui->reset();
    equalizer_ui->reset();
    reverb_ui->reset();
}
