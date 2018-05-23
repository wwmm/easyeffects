#include <algorithm>
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
          "com.github.wwmm.pulseeffects.sinkinputs.compressor")),
      filter_ui(
          FilterUi::create("com.github.wwmm.pulseeffects.sinkinputs.filter")),
      equalizer_ui(EqualizerUi::create(
          "com.github.wwmm.pulseeffects.sinkinputs.equalizer")),
      reverb_ui(
          ReverbUi::create("com.github.wwmm.pulseeffects.sinkinputs.reverb")) {
    level_meters_connections();

    add_plugins();

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

void SinkInputEffectsUi::up_down_connections() {
    auto on_up = [=](auto p) {
        auto order = Glib::Variant<std::vector<std::string>>();

        settings->get_value("plugins", order);

        auto vorder = order.get();

        auto r = std::find(std::begin(vorder), std::end(vorder), p->name);

        if (r != std::begin(vorder)) {
            std::iter_swap(r, r - 1);

            for (auto v : vorder) {
                std::cout << v << std::endl;
            }
        }
    };

    auto on_down = [=](auto p) {
        auto order = Glib::Variant<std::vector<std::string>>();

        settings->get_value("plugins", order);

        auto vorder = order.get();

        auto r = std::find(std::begin(vorder), std::end(vorder), p->name);

        if (r != std::end(vorder) - 1) {
            std::iter_swap(r, r + 1);

            for (auto v : vorder) {
                std::cout << v << std::endl;
            }
        }
    };

    equalizer_ui->plugin_up->signal_clicked().connect(
        [=]() { on_up(equalizer_ui); });
    equalizer_ui->plugin_down->signal_clicked().connect(
        [=]() { on_down(equalizer_ui); });

    reverb_ui->plugin_up->signal_clicked().connect([=]() { on_up(reverb_ui); });
    reverb_ui->plugin_down->signal_clicked().connect(
        [=]() { on_down(reverb_ui); });
}

void SinkInputEffectsUi::reset() {
    limiter_ui->reset();
    compressor_ui->reset();
    filter_ui->reset();
    equalizer_ui->reset();
    reverb_ui->reset();
}
