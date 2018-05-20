#include <gtkmm/comboboxtext.h>
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
                         const Glib::RefPtr<Gtk::Builder>& refBuilder,
                         std::string settings_name)
    : Gtk::Grid(cobject), PluginUiBase(refBuilder, settings_name) {
    name = "equalizer";

    // loading glade widgets

    builder->get_widget("bands_grid", bands_grid);
    builder->get_widget("reset_eq", reset_eq);

    get_object("nbands", nbands);

    nbands->signal_value_changed().connect(
        sigc::mem_fun(*this, &EqualizerUi::on_nbands_changed));

    reset_eq->signal_clicked().connect(
        sigc::mem_fun(*this, &EqualizerUi::reset));

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("num-bands", nbands, "value", flag);

    settings->set_boolean("post-messages", true);
}

EqualizerUi::~EqualizerUi() {
    settings->set_boolean("post-messages", false);

    for (auto c : connections) {
        c.disconnect();
    }
}

std::shared_ptr<EqualizerUi> EqualizerUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/equalizer.glade");

    EqualizerUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    return std::shared_ptr<EqualizerUi>(grid);
}

void EqualizerUi::on_nbands_changed() {
    int N = nbands->get_value();

    for (auto c : connections) {
        c.disconnect();
    }

    for (auto c : bands_grid->get_children()) {
        bands_grid->remove(*c);
    }

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    for (int n = 0; n < N; n++) {
        auto B = Gtk::Builder::create_from_resource(
            "/com/github/wwmm/pulseeffects/equalizer_band.glade");

        Gtk::Grid* band_grid;
        Gtk::ComboBoxText* band_t;
        Gtk::Button *reset_f, *reset_w;

        B->get_widget("band_grid", band_grid);
        B->get_widget("band_t", band_t);
        B->get_widget("reset_f", reset_f);
        B->get_widget("reset_w", reset_w);

        auto band_g = (Gtk::Adjustment*)B->get_object("band_g").get();
        auto band_f = (Gtk::Adjustment*)B->get_object("band_f").get();
        auto band_w = (Gtk::Adjustment*)B->get_object("band_w").get();

        settings->bind(std::string("band" + std::to_string(n) + "-gain"),
                       band_g, "value", flag);
        settings->bind(std::string("band" + std::to_string(n) + "-frequency"),
                       band_f, "value", flag);
        settings->bind(std::string("band" + std::to_string(n) + "-width"),
                       band_w, "value", flag);

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

void EqualizerUi::reset() {
    settings->reset("state");
    settings->reset("num-bands");

    for (int n = 0; n < 30; n++) {
        settings->reset(std::string("band" + std::to_string(n) + "-gain"));
        settings->reset(std::string("band" + std::to_string(n) + "-frequency"));
        settings->reset(std::string("band" + std::to_string(n) + "-width"));
    }
}
