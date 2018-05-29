#include "compressor_ui.hpp"

namespace {

gboolean detection_enum_to_int(GValue* value,
                               GVariant* variant,
                               gpointer user_data) {
    auto v = g_variant_get_string(variant, nullptr);

    if (v == std::string("RMS")) {
        g_value_set_int(value, 0);
    } else if (v == std::string("Peak")) {
        g_value_set_int(value, 1);
    }

    return true;
}

GVariant* int_to_detection_enum(const GValue* value,
                                const GVariantType* expected_type,
                                gpointer user_data) {
    int v = g_value_get_int(value);

    if (v == 0) {
        return g_variant_new_string("RMS");
    } else {
        return g_variant_new_string("Peak");
    }
}

gboolean stereo_link_enum_to_int(GValue* value,
                                 GVariant* variant,
                                 gpointer user_data) {
    auto v = g_variant_get_string(variant, nullptr);

    if (v == std::string("Average")) {
        g_value_set_int(value, 0);
    } else if (v == std::string("Maximum")) {
        g_value_set_int(value, 1);
    }

    return true;
}

GVariant* int_to_stereo_link_enum(const GValue* value,
                                  const GVariantType* expected_type,
                                  gpointer user_data) {
    int v = g_value_get_int(value);

    if (v == 0) {
        return g_variant_new_string("Average");
    } else {
        return g_variant_new_string("Maximum");
    }
}

}  // namespace

CompressorUi::CompressorUi(BaseObjectType* cobject,
                           const Glib::RefPtr<Gtk::Builder>& refBuilder,
                           const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(refBuilder, settings_name) {
    name = "compressor";

    // loading glade widgets

    builder->get_widget("detection", detection);
    builder->get_widget("stereo_link", stereo_link);
    builder->get_widget("compression", compression);
    builder->get_widget("compression_label", compression_label);
    builder->get_widget("preset_vocal_leveller1", preset_vocal_leveller1);
    builder->get_widget("preset_vocal_leveller2", preset_vocal_leveller2);
    builder->get_widget("preset_default", preset_default);

    get_object("attack", attack);
    get_object("knee", knee);
    get_object("makeup", makeup);
    get_object("mix", mix);
    get_object("ratio", ratio);
    get_object("release", release);
    get_object("threshold", threshold);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("attack", attack, "value", flag);
    settings->bind("knee", knee, "value", flag);
    settings->bind("makeup", makeup, "value", flag);
    settings->bind("mix", mix, "value", flag);
    settings->bind("ratio", ratio, "value", flag);
    settings->bind("release", release, "value", flag);
    settings->bind("threshold", threshold, "value", flag);

    g_settings_bind_with_mapping(settings->gobj(), "detection",
                                 detection->gobj(), "active",
                                 G_SETTINGS_BIND_DEFAULT, detection_enum_to_int,
                                 int_to_detection_enum, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings->gobj(), "stereo-link", stereo_link->gobj(), "active",
        G_SETTINGS_BIND_DEFAULT, stereo_link_enum_to_int,
        int_to_stereo_link_enum, nullptr, nullptr);

    init_presets_buttons();

    settings->set_boolean("post-messages", true);
}

CompressorUi::~CompressorUi() {
    settings->set_boolean("post-messages", false);
}

std::shared_ptr<CompressorUi> CompressorUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/compressor.glade");

    CompressorUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    return std::shared_ptr<CompressorUi>(grid);
}

void CompressorUi::on_new_compression(double value) {
    compression->set_value(1 - value);

    compression_label->set_text(level_to_str(util::linear_to_db(value)));
}

void CompressorUi::init_presets_buttons() {
    preset_vocal_leveller1->signal_clicked().connect([=]() {
        threshold->set_value(util::linear_to_db(0.0883884));
        ratio->set_value(4.25008);
        attack->set_value(3.10087);
        release->set_value(25.0012);
        makeup->set_value(util::linear_to_db(4.85678));
        knee->set_value(util::linear_to_db(8));
        detection->set_active(0);
        stereo_link->set_active(0);
        settings->reset("mix");
    });

    preset_vocal_leveller2->signal_clicked().connect([=]() {
        threshold->set_value(util::linear_to_db(0.0883884));
        ratio->set_value(4.25008);
        attack->set_value(10.5096);
        release->set_value(106.852);
        makeup->set_value(util::linear_to_db(4.85678));
        knee->set_value(util::linear_to_db(8));
        detection->set_active(0);
        stereo_link->set_active(0);
        settings->reset("mix");
    });

    preset_default->signal_clicked().connect([=]() {
        settings->reset("detection");
        settings->reset("stereo-link");
        settings->reset("mix");
        settings->reset("attack");
        settings->reset("release");
        settings->reset("threshold");
        settings->reset("ratio");
        settings->reset("knee");
        settings->reset("makeup");
    });
}

void CompressorUi::reset() {
    settings->reset("state");
    settings->reset("detection");
    settings->reset("stereo-link");
    settings->reset("mix");
    settings->reset("attack");
    settings->reset("release");
    settings->reset("threshold");
    settings->reset("ratio");
    settings->reset("knee");
    settings->reset("makeup");
}
