#include <algorithm>
#include "sink_input_effects_ui.hpp"

SinkInputEffectsUi::SinkInputEffectsUi(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder,
    const Glib::RefPtr<Gio::Settings>& refSettings,
    std::shared_ptr<SinkInputEffects> sie_ptr)
    : EffectsBaseUi(cobject, refBuilder, refSettings, sie_ptr->pm),
      sie(sie_ptr),
      limiter_ui(
          LimiterUi::create("com.github.wwmm.pulseeffects.sinkinputs.limiter")),
      compressor_ui(CompressorUi::create(
          "com.github.wwmm.pulseeffects.sinkinputs.compressor")),
      filter_ui(
          FilterUi::create("com.github.wwmm.pulseeffects.sinkinputs.filter")),
      equalizer_ui(EqualizerUi::create(
          "com.github.wwmm.pulseeffects.sinkinputs.equalizer")),
      reverb_ui(
          ReverbUi::create("com.github.wwmm.pulseeffects.sinkinputs.reverb")) {
    level_meters_connections();
    populate_listbox();
    populate_stack();
    up_down_connections();
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

    auto settings =
        Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs");

    SinkInputEffectsUi* sie_ui = nullptr;

    builder->get_widget_derived("widgets_box", sie_ui, settings, sie);

    return std::unique_ptr<SinkInputEffectsUi>(sie_ui);
}

void SinkInputEffectsUi::level_meters_connections() {
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

    // filter level meters connections

    connections.push_back(sie->filter->input_level.connect(
        sigc::mem_fun(*filter_ui, &FilterUi::on_new_input_level)));
    connections.push_back(sie->filter->output_level.connect(
        sigc::mem_fun(*filter_ui, &FilterUi::on_new_output_level)));

    // equalizer level meters connections

    connections.push_back(sie->equalizer_input_level.connect(
        sigc::mem_fun(*equalizer_ui, &EqualizerUi::on_new_input_level_db)));
    connections.push_back(sie->equalizer_output_level.connect(
        sigc::mem_fun(*equalizer_ui, &EqualizerUi::on_new_output_level_db)));

    // reverb level meters connections

    connections.push_back(sie->reverb->input_level.connect(
        sigc::mem_fun(*reverb_ui, &ReverbUi::on_new_input_level)));
    connections.push_back(sie->reverb->output_level.connect(
        sigc::mem_fun(*reverb_ui, &ReverbUi::on_new_output_level)));
}

void SinkInputEffectsUi::populate_listbox() {
    add_to_listbox(limiter_ui);
    add_to_listbox(compressor_ui);
    add_to_listbox(filter_ui);
    add_to_listbox(equalizer_ui);
    add_to_listbox(reverb_ui);
}

void SinkInputEffectsUi::populate_stack() {
    stack->add(*limiter_ui, std::string("limiter"));
    stack->add(*compressor_ui, std::string("compressor"));
    stack->add(*filter_ui, std::string("filter"));
    stack->add(*equalizer_ui, std::string("equalizer"));
    stack->add(*reverb_ui, std::string("reverb"));
}

void SinkInputEffectsUi::up_down_connections() {
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
}

void SinkInputEffectsUi::reset() {
    limiter_ui->reset();
    compressor_ui->reset();
    filter_ui->reset();
    equalizer_ui->reset();
    reverb_ui->reset();
}
