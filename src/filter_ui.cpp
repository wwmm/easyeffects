#include "filter_ui.hpp"

namespace {

gboolean filter_enum_to_int(GValue* value,
                            GVariant* variant,
                            gpointer user_data) {
    auto v = g_variant_get_string(variant, nullptr);

    if (v == std::string("12dB/oct Lowpass")) {
        g_value_set_int(value, 0);
    } else if (v == std::string("24dB/oct Lowpass")) {
        g_value_set_int(value, 1);
    } else if (v == std::string("36dB/oct Lowpass")) {
        g_value_set_int(value, 2);
    } else if (v == std::string("12dB/oct Highpass")) {
        g_value_set_int(value, 3);
    } else if (v == std::string("24dB/oct Highpass")) {
        g_value_set_int(value, 4);
    } else if (v == std::string("36dB/oct Highpass")) {
        g_value_set_int(value, 5);
    } else if (v == std::string("6dB/oct Bandpass")) {
        g_value_set_int(value, 6);
    } else if (v == std::string("12dB/oct Bandpass")) {
        g_value_set_int(value, 7);
    } else if (v == std::string("18dB/oct Bandpass")) {
        g_value_set_int(value, 8);
    } else if (v == std::string("6dB/oct Bandreject")) {
        g_value_set_int(value, 9);
    } else if (v == std::string("12dB/oct Bandreject")) {
        g_value_set_int(value, 10);
    } else if (v == std::string("18dB/oct Bandreject")) {
        g_value_set_int(value, 11);
    }

    return true;
}

GVariant* int_to_filter_enum(const GValue* value,
                             const GVariantType* expected_type,
                             gpointer user_data) {
    int v = g_value_get_int(value);

    if (v == 0) {
        return g_variant_new_string("12dB/oct Lowpass");
    } else if (v == 1) {
        return g_variant_new_string("24dB/oct Lowpass");
    } else if (v == 2) {
        return g_variant_new_string("36dB/oct Lowpass");
    } else if (v == 3) {
        return g_variant_new_string("12dB/oct Highpass");
    } else if (v == 4) {
        return g_variant_new_string("24dB/oct Highpass");
    } else if (v == 5) {
        return g_variant_new_string("36dB/oct Highpass");
    } else if (v == 6) {
        return g_variant_new_string("6dB/oct Bandpass");
    } else if (v == 7) {
        return g_variant_new_string("12dB/oct Bandpass");
    } else if (v == 8) {
        return g_variant_new_string("18dB/oct Bandpass");
    } else if (v == 9) {
        return g_variant_new_string("6dB/oct Bandreject");
    } else if (v == 10) {
        return g_variant_new_string("12dB/oct Bandreject");
    } else {
        return g_variant_new_string("18dB/oct Bandreject");
    }
}

}  // namespace

FilterUi::FilterUi(BaseObjectType* cobject,
                   const Glib::RefPtr<Gtk::Builder>& refBuilder,
                   std::string settings_name)
    : Gtk::Grid(cobject), PluginUiBase(refBuilder, settings_name) {
    name = "filter";

    // loading glade widgets

    builder->get_widget("mode", mode);
    builder->get_widget("preset_muted", preset_muted);
    builder->get_widget("preset_disco", preset_disco);
    builder->get_widget("preset_distant_headphones", preset_distant_headphones);
    builder->get_widget("preset_default", preset_default);

    get_object("input_gain", input_gain);
    get_object("output_gain", output_gain);
    get_object("frequency", frequency);
    get_object("resonance", resonance);
    get_object("inertia", inertia);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("input-gain", input_gain, "value", flag);
    settings->bind("output-gain", output_gain, "value", flag);
    settings->bind("frequency", frequency, "value", flag);
    settings->bind("resonance", resonance, "value", flag);
    settings->bind("inertia", inertia, "value", flag);

    g_settings_bind_with_mapping(settings->gobj(), "mode", mode->gobj(),
                                 "active", G_SETTINGS_BIND_DEFAULT,
                                 filter_enum_to_int, int_to_filter_enum,
                                 nullptr, nullptr);

    init_presets_buttons();

    settings->set_boolean("post-messages", true);
}

FilterUi::~FilterUi() {
    settings->set_boolean("post-messages", false);
}

std::shared_ptr<FilterUi> FilterUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/filter.glade");

    FilterUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    return std::shared_ptr<FilterUi>(grid);
}

void FilterUi::init_presets_buttons() {
    preset_muted->signal_clicked().connect([=]() {
        frequency->set_value(10);
        resonance->set_value(0.707);
        mode->set_active(2);
        inertia->set_value(20);
    });

    preset_disco->signal_clicked().connect([=]() {
        frequency->set_value(193.821);
        resonance->set_value(1.37956);
        mode->set_active(0);
        inertia->set_value(74);
    });

    preset_distant_headphones->signal_clicked().connect([=]() {
        frequency->set_value(305.818);
        resonance->set_value(0.707);
        mode->set_active(3);
        inertia->set_value(74);
    });

    preset_default->signal_clicked().connect([=]() {
        settings->reset("frequency");
        settings->reset("resonance");
        settings->reset("mode");
        settings->reset("inertia");
    });
}

void FilterUi::reset() {
    settings->reset("state");
    settings->reset("input-gain");
    settings->reset("output-gain");
    settings->reset("frequency");
    settings->reset("resonance");
    settings->reset("mode");
    settings->reset("inertia");
}
