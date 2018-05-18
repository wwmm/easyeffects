#include "sink_input_effects_ui.hpp"

SinkInputEffectsUi::SinkInputEffectsUi(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder,
    std::shared_ptr<SinkInputEffects> sie_ptr)
    : EffectsBaseUi(cobject, refBuilder, sie_ptr->pm),
      sie(sie_ptr),
      settings(
          Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs")),
      limiter_ui(
          LimiterUi::create("com.github.wwmm.pulseeffects.sinkinputs.limiter")),
      compressor_ui(CompressorUi::create(
          "com.github.wwmm.pulseeffects.sinkinputs.compressor")) {
    // limiter level meters connections

    connections.push_back(sie->limiter->input_level.connect(
        sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_input_level)));
    connections.push_back(sie->limiter->output_level.connect(
        sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_output_level)));
    connections.push_back(sie->limiter->attenuation.connect(
        sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_attenuation)));

    // compressor level meters connections

    connections.push_back(sie->compressor_input_level.connect(
        sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_input_level_db)));
    connections.push_back(sie->compressor_output_level.connect(
        sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_output_level_db)));
    connections.push_back(sie->compressor->compression.connect(
        sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_compression)));

    add_plugins();
}

SinkInputEffectsUi::~SinkInputEffectsUi() {
    for (auto c : connections) {
        c.disconnect();
    }
}

std::unique_ptr<SinkInputEffectsUi> SinkInputEffectsUi::create(
    std::shared_ptr<SinkInputEffects> sie) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/effects_base.glade");

    SinkInputEffectsUi* sie_ui = nullptr;

    builder->get_widget_derived("widgets_box", sie_ui, sie);

    return std::unique_ptr<SinkInputEffectsUi>(sie_ui);
}

void SinkInputEffectsUi::add_plugins() {
    auto plugins_order = Glib::Variant<std::vector<std::string>>();

    settings->get_value("plugins", plugins_order);

    for (auto name : plugins_order.get()) {
        if (name == std::string("limiter")) {
            add_to_listbox(limiter_ui);
            stack->add(*limiter_ui, std::string("limiter"));
        } else if (name == std::string("compressor")) {
            add_to_listbox(compressor_ui);
            stack->add(*compressor_ui, std::string("compressor"));
        }
    }
}

void SinkInputEffectsUi::reset() {
    limiter_ui->reset();
    compressor_ui->reset();
}
