#include "sink_input_effects_ui.hpp"

SinkInputEffectsUi::SinkInputEffectsUi(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder,
    std::shared_ptr<SinkInputEffects> sie_ptr)
    : EffectsBaseUi(cobject, refBuilder, sie_ptr->pm),
      sie(sie_ptr),
      settings(
          Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs")),
      limiter_ui(LimiterUi::create(
          "com.github.wwmm.pulseeffects.sinkinputs.limiter")) {
    // level meters connections

    sie->limiter_input_level.connect(
        sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_input_level));
    sie->limiter_output_level.connect(
        sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_output_level));
    sie->limiter_attenuation.connect(
        sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_attenuation));

    add_plugins();
}

SinkInputEffectsUi::~SinkInputEffectsUi() {}

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
        if (name == "limiter") {
            add_to_listbox(limiter_ui);
            stack->add(*limiter_ui, std::string("limiter"));
        }
    }
}
