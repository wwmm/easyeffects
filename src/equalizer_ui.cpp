#include <gtkmm/comboboxtext.h>
#include <gtkmm/label.h>
#include "equalizer_ui.hpp"

namespace {

gboolean bandtype_enum_to_int(GValue* value,
                              GVariant* variant,
                              gpointer user_data) {
    auto v = g_variant_get_string(variant, nullptr);

    if (v == std::string("peak")) {
        g_value_set_int(value, 0);
    } else if (v == std::string("low-shelf")) {
        g_value_set_int(value, 1);
    } else if (v == std::string("high-shelf")) {
        g_value_set_int(value, 2);
    }

    return true;
}

GVariant* int_to_bandtype_enum(const GValue* value,
                               const GVariantType* expected_type,
                               gpointer user_data) {
    int v = g_value_get_int(value);

    if (v == 0) {
        return g_variant_new_string("peak");
    } else if (v == 1) {
        return g_variant_new_string("low-shelf");
    } else {
        return g_variant_new_string("high-shelf");
    }
}

}  // namespace

EqualizerUi::EqualizerUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
    name = "equalizer";

    // loading glade widgets

    builder->get_widget("bands_grid", bands_grid);
    builder->get_widget("reset_eq", reset_eq);
    builder->get_widget("flat_response", flat_response);
    builder->get_widget("calculate_freqs", calculate_freqs);

    get_object(builder, "nbands", nbands);
    get_object(builder, "input_gain", input_gain);
    get_object(builder, "output_gain", output_gain);

    nbands->signal_value_changed().connect(
        sigc::mem_fun(*this, &EqualizerUi::on_nbands_changed));

    reset_eq->signal_clicked().connect(
        sigc::mem_fun(*this, &EqualizerUi::reset));

    flat_response->signal_clicked().connect(
        sigc::mem_fun(*this, &EqualizerUi::on_flat_response));

    calculate_freqs->signal_clicked().connect(
        sigc::mem_fun(*this, &EqualizerUi::on_calculate_frequencies));

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("installed", this, "sensitive", flag);
    settings->bind("num-bands", nbands.get(), "value", flag);
    settings->bind("input-gain", input_gain.get(), "value", flag);
    settings->bind("output-gain", output_gain.get(), "value", flag);

    settings->set_boolean("post-messages", true);
}

EqualizerUi::~EqualizerUi() {
    settings->set_boolean("post-messages", false);

    for (auto c : connections) {
        c.disconnect();
    }

    util::debug(name + " ui destroyed");
}

void EqualizerUi::on_nbands_changed() {
    int N = nbands->get_value();

    for (auto c : connections) {
        c.disconnect();
    }

    connections.clear();

    for (auto c : bands_grid->get_children()) {
        bands_grid->remove(*c);

        delete c;
    }

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    for (int n = 0; n < N; n++) {
        auto B = Gtk::Builder::create_from_resource(
            "/com/github/wwmm/pulseeffects/equalizer_band.glade");

        Gtk::Grid* band_grid;
        Gtk::ComboBoxText* band_t;
        Gtk::Label *band_q, *band_label;
        Gtk::Button *reset_f, *reset_w;

        B->get_widget("band_grid", band_grid);
        B->get_widget("band_t", band_t);
        B->get_widget("band_q", band_q);
        B->get_widget("band_label", band_label);
        B->get_widget("reset_f", reset_f);
        B->get_widget("reset_w", reset_w);

        auto band_g = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(
            B->get_object("band_g"));
        auto band_f = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(
            B->get_object("band_f"));
        auto band_w = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(
            B->get_object("band_w"));

        auto update_q = [=]() {
            auto w = band_w->get_value();

            if (w > 0) {
                auto f = band_f->get_value();

                std::ostringstream msg;

                msg.precision(2);
                msg << std::fixed << f / w;

                band_q->set_text(msg.str());
            }
        };

        auto update_band_label = [=]() {
            auto f = band_f->get_value();

            std::ostringstream msg;

            if (f > 1000) {
                msg.precision(1);
                msg << std::fixed << f / 1000 << "kHz";
            } else {
                msg.precision(0);
                msg << std::fixed << f << "Hz";
            }

            band_label->set_text(msg.str());
        };

        connections.push_back(band_f->signal_value_changed().connect(update_q));
        connections.push_back(
            band_f->signal_value_changed().connect(update_band_label));
        connections.push_back(band_w->signal_value_changed().connect(update_q));

        settings->bind(std::string("band" + std::to_string(n) + "-gain"),
                       band_g.get(), "value", flag);
        settings->bind(std::string("band" + std::to_string(n) + "-frequency"),
                       band_f.get(), "value", flag);
        settings->bind(std::string("band" + std::to_string(n) + "-width"),
                       band_w.get(), "value", flag);

        g_settings_bind_with_mapping(
            settings->gobj(),
            std::string("band" + std::to_string(n) + "-type").c_str(),
            band_t->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
            bandtype_enum_to_int, int_to_bandtype_enum, nullptr, nullptr);

        connections.push_back(reset_f->signal_clicked().connect([=]() {
            settings->reset(
                std::string("band" + std::to_string(n) + "-frequency"));
        }));

        connections.push_back(reset_w->signal_clicked().connect([=]() {
            settings->reset(std::string("band" + std::to_string(n) + "-width"));
        }));

        bands_grid->add(*band_grid);
    }

    bands_grid->show_all();
}

void EqualizerUi::on_flat_response() {
    for (int n = 0; n < 30; n++) {
        settings->reset(std::string("band" + std::to_string(n) + "-gain"));
    }
}

void EqualizerUi::on_calculate_frequencies() {
    const double min_freq = 20.0;
    const double max_freq = 20000.0;
    double freq0, freq1, step;

    int nbands = settings->get_int("num-bands");

    // code taken from gstreamer equalizer sources: gstiirequalizer.c
    // function: gst_iir_equalizer_compute_frequencies

    step = pow(max_freq / min_freq, 1.0 / nbands);
    freq0 = min_freq;

    for (int n = 0; n < nbands; n++) {
        freq1 = freq0 * step;

        double freq = freq0 + ((freq1 - freq0) / 2.0);
        double width = freq1 - freq0;

        // std::cout << n << "\t" << freq << "\t" << width << std::endl;

        settings->set_double(
            std::string("band" + std::to_string(n) + "-frequency"), freq);

        settings->set_double(std::string("band" + std::to_string(n) + "-width"),
                             width);

        freq0 = freq1;
    }
}

void EqualizerUi::reset() {
    settings->reset("state");
    settings->reset("num-bands");

    for (int n = 0; n < 30; n++) {
        settings->reset(std::string("band" + std::to_string(n) + "-gain"));
        settings->reset(std::string("band" + std::to_string(n) + "-frequency"));
        settings->reset(std::string("band" + std::to_string(n) + "-width"));
        settings->reset(std::string("band" + std::to_string(n) + "-type"));
    }
}
