#include "multiband_compressor_ui.hpp"

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

gboolean mode_enum_to_int(GValue* value,
                          GVariant* variant,
                          gpointer user_data) {
    auto v = g_variant_get_string(variant, nullptr);

    if (v == std::string("LR4")) {
        g_value_set_int(value, 0);
    } else if (v == std::string("LR8")) {
        g_value_set_int(value, 1);
    }

    return true;
}

GVariant* int_to_mode_enum(const GValue* value,
                           const GVariantType* expected_type,
                           gpointer user_data) {
    int v = g_value_get_int(value);

    if (v == 0) {
        return g_variant_new_string("LR4");
    } else {
        return g_variant_new_string("LR8");
    }
}

}  // namespace

MultibandCompressorUi::MultibandCompressorUi(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& builder,
    const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
    name = "multiband_compressor";

    // loading glade widgets

    builder->get_widget("detection", detection);
    builder->get_widget("stereo_link", stereo_link);
    builder->get_widget("compression", compression);
    builder->get_widget("compression_label", compression_label);
    builder->get_widget("preset_vocal_leveller1", preset_vocal_leveller1);
    builder->get_widget("preset_vocal_leveller2", preset_vocal_leveller2);
    builder->get_widget("preset_default", preset_default);

    get_object(builder, "attack", attack);
    get_object(builder, "knee", knee);
    get_object(builder, "makeup", makeup);
    get_object(builder, "mix", mix);
    get_object(builder, "ratio", ratio);
    get_object(builder, "release", release);
    get_object(builder, "threshold", threshold);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("installed", this, "sensitive", flag);
    settings->bind("attack", attack.get(), "value", flag);
    settings->bind("knee", knee.get(), "value", flag);
    settings->bind("makeup", makeup.get(), "value", flag);
    settings->bind("mix", mix.get(), "value", flag);
    settings->bind("ratio", ratio.get(), "value", flag);
    settings->bind("release", release.get(), "value", flag);
    settings->bind("threshold", threshold.get(), "value", flag);

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

MultibandCompressorUi::~MultibandCompressorUi() {
    settings->set_boolean("post-messages", false);

    util::debug(name + " ui destroyed");
}

void MultibandCompressorUi::on_new_compression(double value) {
    compression->set_value(1 - value);

    compression_label->set_text(level_to_str(util::linear_to_db(value)));
}

void MultibandCompressorUi::init_presets_buttons() {
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

void MultibandCompressorUi::reset() {
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
