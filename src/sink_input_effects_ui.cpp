#include <algorithm>
#include "sink_input_effects_ui.hpp"

SinkInputEffectsUi::SinkInputEffectsUi(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder,
    const Glib::RefPtr<Gio::Settings>& refSettings,
    const std::shared_ptr<SinkInputEffects>& sie_ptr)
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
          ReverbUi::create("com.github.wwmm.pulseeffects.sinkinputs.reverb")),
      bass_enhancer_ui(BassEnhancerUi::create(
          "com.github.wwmm.pulseeffects.sinkinputs.bassenhancer")),
      exciter_ui(
          ExciterUi::create("com.github.wwmm.pulseeffects.sinkinputs.exciter")),
      stereo_enhancer_ui(StereoEnhancerUi::create(
          "com.github.wwmm.pulseeffects.sinkinputs.stereoenhancer")),
      panorama_ui(PanoramaUi::create(
          "com.github.wwmm.pulseeffects.sinkinputs.panorama")),
      crossfeed_ui(CrossfeedUi::create(
          "com.github.wwmm.pulseeffects.sinkinputs.crossfeed")),
      maximizer_ui(MaximizerUi::create(
          "com.github.wwmm.pulseeffects.sinkinputs.maximizer")),
      delay_ui(
          DelayUi::create("com.github.wwmm.pulseeffects.sinkinputs.delay")) {
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

    // bass_enhancer level meters connections

    connections.push_back(sie->bass_enhancer_input_level.connect(sigc::mem_fun(
        *bass_enhancer_ui, &BassEnhancerUi::on_new_input_level_db)));
    connections.push_back(sie->bass_enhancer_output_level.connect(sigc::mem_fun(
        *bass_enhancer_ui, &BassEnhancerUi::on_new_output_level_db)));
    connections.push_back(sie->bass_enhancer->harmonics.connect(sigc::mem_fun(
        *bass_enhancer_ui, &BassEnhancerUi::on_new_harmonics_level)));

    // exciter level meters connections

    connections.push_back(sie->exciter_input_level.connect(
        sigc::mem_fun(*exciter_ui, &ExciterUi::on_new_input_level_db)));
    connections.push_back(sie->exciter_output_level.connect(
        sigc::mem_fun(*exciter_ui, &ExciterUi::on_new_output_level_db)));
    connections.push_back(sie->exciter->harmonics.connect(
        sigc::mem_fun(*exciter_ui, &ExciterUi::on_new_harmonics_level)));

    // stereo_enhancer level meters connections

    connections.push_back(
        sie->stereo_enhancer->input_level.connect(sigc::mem_fun(
            *stereo_enhancer_ui, &StereoEnhancerUi::on_new_input_level)));
    connections.push_back(
        sie->stereo_enhancer->output_level.connect(sigc::mem_fun(
            *stereo_enhancer_ui, &StereoEnhancerUi::on_new_output_level)));
    connections.push_back(
        sie->stereo_enhancer->side_level.connect(sigc::mem_fun(
            *stereo_enhancer_ui, &StereoEnhancerUi::on_new_side_level)));

    // panorama level meters connections

    connections.push_back(sie->panorama_input_level.connect(
        sigc::mem_fun(*panorama_ui, &PanoramaUi::on_new_input_level_db)));
    connections.push_back(sie->panorama_output_level.connect(
        sigc::mem_fun(*panorama_ui, &PanoramaUi::on_new_output_level_db)));

    // crossfeed level meters connections

    connections.push_back(sie->crossfeed_input_level.connect(
        sigc::mem_fun(*crossfeed_ui, &CrossfeedUi::on_new_input_level_db)));
    connections.push_back(sie->crossfeed_output_level.connect(
        sigc::mem_fun(*crossfeed_ui, &CrossfeedUi::on_new_output_level_db)));

    // maximizer level meters connections

    connections.push_back(sie->maximizer_input_level.connect(
        sigc::mem_fun(*maximizer_ui, &MaximizerUi::on_new_input_level_db)));
    connections.push_back(sie->maximizer_output_level.connect(
        sigc::mem_fun(*maximizer_ui, &MaximizerUi::on_new_output_level_db)));
    connections.push_back(sie->maximizer->reduction.connect(
        sigc::mem_fun(*maximizer_ui, &MaximizerUi::on_new_reduction)));

    // delay level meters connections

    connections.push_back(sie->delay_input_level.connect(
        sigc::mem_fun(*delay_ui, &DelayUi::on_new_input_level_db)));
    connections.push_back(sie->delay_output_level.connect(
        sigc::mem_fun(*delay_ui, &DelayUi::on_new_output_level_db)));
    connections.push_back(sie->delay->tempo.connect(
        sigc::mem_fun(*delay_ui, &DelayUi::on_new_tempo)));
}

void SinkInputEffectsUi::populate_listbox() {
    add_to_listbox(limiter_ui);
    add_to_listbox(compressor_ui);
    add_to_listbox(filter_ui);
    add_to_listbox(equalizer_ui);
    add_to_listbox(reverb_ui);
    add_to_listbox(bass_enhancer_ui);
    add_to_listbox(exciter_ui);
    add_to_listbox(stereo_enhancer_ui);
    add_to_listbox(panorama_ui);
    add_to_listbox(crossfeed_ui);
    add_to_listbox(maximizer_ui);
    add_to_listbox(delay_ui);
}

void SinkInputEffectsUi::populate_stack() {
    stack->add(*limiter_ui, std::string("limiter"));
    stack->add(*compressor_ui, std::string("compressor"));
    stack->add(*filter_ui, std::string("filter"));
    stack->add(*equalizer_ui, std::string("equalizer"));
    stack->add(*reverb_ui, std::string("reverb"));
    stack->add(*bass_enhancer_ui, std::string("bass_enhancer"));
    stack->add(*exciter_ui, std::string("exciter"));
    stack->add(*stereo_enhancer_ui, std::string("stereo_enhancer"));
    stack->add(*panorama_ui, std::string("panorama"));
    stack->add(*crossfeed_ui, std::string("crossfeed"));
    stack->add(*maximizer_ui, std::string("maximizer"));
    stack->add(*delay_ui, std::string("delay"));
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

    connections.push_back(bass_enhancer_ui->plugin_up->signal_clicked().connect(
        [=]() { on_up(bass_enhancer_ui); }));
    connections.push_back(
        bass_enhancer_ui->plugin_down->signal_clicked().connect(
            [=]() { on_down(bass_enhancer_ui); }));

    connections.push_back(exciter_ui->plugin_up->signal_clicked().connect(
        [=]() { on_up(exciter_ui); }));
    connections.push_back(exciter_ui->plugin_down->signal_clicked().connect(
        [=]() { on_down(exciter_ui); }));

    connections.push_back(
        stereo_enhancer_ui->plugin_up->signal_clicked().connect(
            [=]() { on_up(stereo_enhancer_ui); }));
    connections.push_back(
        stereo_enhancer_ui->plugin_down->signal_clicked().connect(
            [=]() { on_down(stereo_enhancer_ui); }));

    connections.push_back(panorama_ui->plugin_up->signal_clicked().connect(
        [=]() { on_up(panorama_ui); }));
    connections.push_back(panorama_ui->plugin_down->signal_clicked().connect(
        [=]() { on_down(panorama_ui); }));

    connections.push_back(crossfeed_ui->plugin_up->signal_clicked().connect(
        [=]() { on_up(crossfeed_ui); }));
    connections.push_back(crossfeed_ui->plugin_down->signal_clicked().connect(
        [=]() { on_down(crossfeed_ui); }));

    connections.push_back(maximizer_ui->plugin_up->signal_clicked().connect(
        [=]() { on_up(maximizer_ui); }));
    connections.push_back(maximizer_ui->plugin_down->signal_clicked().connect(
        [=]() { on_down(maximizer_ui); }));

    connections.push_back(delay_ui->plugin_up->signal_clicked().connect(
        [=]() { on_up(delay_ui); }));
    connections.push_back(delay_ui->plugin_down->signal_clicked().connect(
        [=]() { on_down(delay_ui); }));
}

void SinkInputEffectsUi::reset() {
    settings->reset("plugins");

    limiter_ui->reset();
    compressor_ui->reset();
    filter_ui->reset();
    equalizer_ui->reset();
    reverb_ui->reset();
    bass_enhancer_ui->reset();
    exciter_ui->reset();
    stereo_enhancer_ui->reset();
    panorama_ui->reset();
    crossfeed_ui->reset();
    maximizer_ui->reset();
    delay_ui->reset();
}
