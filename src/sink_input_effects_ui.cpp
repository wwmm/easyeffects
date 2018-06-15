#include <algorithm>
#include "sink_input_effects_ui.hpp"

SinkInputEffectsUi::SinkInputEffectsUi(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder,
    const Glib::RefPtr<Gio::Settings>& refSettings,
    const std::shared_ptr<SinkInputEffects>& sie_ptr)
    : Gtk::Box(cobject),
      EffectsBaseUi(refBuilder, refSettings, sie_ptr->pm),
      sie(sie_ptr) {
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
    auto b_bass_enhancer = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/bass_enhancer.glade");
    auto b_exciter = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/exciter.glade");
    auto b_stereo_enhancer = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/stereo_enhancer.glade");
    auto b_panorama = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/panorama.glade");
    auto b_crossfeed = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/crossfeed.glade");
    auto b_maximizer = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/maximizer.glade");
    auto b_delay = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/delay.glade");
    auto b_expander = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/expander.glade");

    b_limiter->get_widget_derived(
        "widgets_grid", limiter_ui,
        "com.github.wwmm.pulseeffects.sinkinputs.limiter");
    b_compressor->get_widget_derived(
        "widgets_grid", compressor_ui,
        "com.github.wwmm.pulseeffects.sinkinputs.compressor");
    b_filter->get_widget_derived(
        "widgets_grid", filter_ui,
        "com.github.wwmm.pulseeffects.sinkinputs.filter");
    b_equalizer->get_widget_derived(
        "widgets_grid", equalizer_ui,
        "com.github.wwmm.pulseeffects.sinkinputs.equalizer");
    b_reverb->get_widget_derived(
        "widgets_grid", reverb_ui,
        "com.github.wwmm.pulseeffects.sinkinputs.reverb");
    b_bass_enhancer->get_widget_derived(
        "widgets_grid", bass_enhancer_ui,
        "com.github.wwmm.pulseeffects.sinkinputs.bassenhancer");
    b_exciter->get_widget_derived(
        "widgets_grid", exciter_ui,
        "com.github.wwmm.pulseeffects.sinkinputs.exciter");
    b_stereo_enhancer->get_widget_derived(
        "widgets_grid", stereo_enhancer_ui,
        "com.github.wwmm.pulseeffects.sinkinputs.stereoenhancer");
    b_panorama->get_widget_derived(
        "widgets_grid", panorama_ui,
        "com.github.wwmm.pulseeffects.sinkinputs.panorama");
    b_crossfeed->get_widget_derived(
        "widgets_grid", crossfeed_ui,
        "com.github.wwmm.pulseeffects.sinkinputs.crossfeed");
    b_maximizer->get_widget_derived(
        "widgets_grid", maximizer_ui,
        "com.github.wwmm.pulseeffects.sinkinputs.maximizer");
    b_delay->get_widget_derived(
        "widgets_grid", delay_ui,
        "com.github.wwmm.pulseeffects.sinkinputs.delay");
    b_expander->get_widget_derived(
        "widgets_grid", expander_ui,
        "com.github.wwmm.pulseeffects.sinkinputs.expander");

    stack->add(*limiter_ui, limiter_ui->name);
    stack->add(*compressor_ui, compressor_ui->name);
    stack->add(*filter_ui, filter_ui->name);
    stack->add(*equalizer_ui, equalizer_ui->name);
    stack->add(*reverb_ui, reverb_ui->name);
    stack->add(*bass_enhancer_ui, bass_enhancer_ui->name);
    stack->add(*exciter_ui, exciter_ui->name);
    stack->add(*stereo_enhancer_ui, stereo_enhancer_ui->name);
    stack->add(*panorama_ui, panorama_ui->name);
    stack->add(*crossfeed_ui, crossfeed_ui->name);
    stack->add(*maximizer_ui, maximizer_ui->name);
    stack->add(*delay_ui, delay_ui->name);
    stack->add(*expander_ui, expander_ui->name);

    // populate_listbox

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
    add_to_listbox(expander_ui);

    level_meters_connections();
    up_down_connections();
}

SinkInputEffectsUi::~SinkInputEffectsUi() {
    util::debug(log_tag + "destroyed");
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

    // expander level meters connections

    connections.push_back(sie->expander->input_level.connect(
        sigc::mem_fun(*expander_ui, &ExpanderUi::on_new_input_level)));
    connections.push_back(sie->expander->output_level.connect(
        sigc::mem_fun(*expander_ui, &ExpanderUi::on_new_output_level)));
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

    connections.push_back(expander_ui->plugin_up->signal_clicked().connect(
        [=]() { on_up(expander_ui); }));
    connections.push_back(expander_ui->plugin_down->signal_clicked().connect(
        [=]() { on_down(expander_ui); }));
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
    expander_ui->reset();
}
