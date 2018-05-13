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
    // add_plugins();
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
    auto v_array = Glib::Variant<std::vector<std::string>>();

    settings->get_value("plugins", v_array);

    auto plugins_order = v_array.get();

    for (auto name : plugins_order) {
        if (name == "limiter") {
            add_to_listbox(move(limiter_ui));
            stack->add(*limiter_ui, std::string("limiter"));
        }
    }
}
