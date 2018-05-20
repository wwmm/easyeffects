#include "equalizer_ui.hpp"

EqualizerUi::EqualizerUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& refBuilder,
                         std::string settings_name)
    : Gtk::Grid(cobject), PluginUiBase(refBuilder, settings_name) {
    name = "equalizer";

    // loading glade widgets

    builder->get_widget("bands_grid", bands_grid);

    get_object("nbands", nbands);

    nbands->signal_value_changed().connect(
        sigc::mem_fun(*this, &EqualizerUi::on_nbands_changed));

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("num-bands", nbands, "value", flag);

    // g_settings_bind_with_mapping(settings->gobj(), "mode", mode->gobj(),
    //                              "active", G_SETTINGS_BIND_DEFAULT,
    //                              equalizer_enum_to_int,
    //                              int_to_equalizer_enum, nullptr, nullptr);

    // connections.push_back(
    //     settings->signal_changed("num-bands").connect([&](auto key) {
    //         auto nbands = settings->get_int("num-bands");
    //     }));

    settings->set_boolean("post-messages", true);
}

EqualizerUi::~EqualizerUi() {
    settings->set_boolean("post-messages", false);
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

    for (auto c : bands_grid->get_children()) {
        bands_grid->remove(*c);
    }

    for (int n = 0; n < N; n++) {
        auto B = Gtk::Builder::create_from_resource(
            "/com/github/wwmm/pulseeffects/equalizer_band.glade");

        Gtk::Grid* band_grid;

        B->get_widget("band_grid", band_grid);

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
