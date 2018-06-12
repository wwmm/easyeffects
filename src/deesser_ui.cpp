#include "deesser_ui.hpp"

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

    if (v == std::string("Wide")) {
        g_value_set_int(value, 0);
    } else if (v == std::string("Split")) {
        g_value_set_int(value, 1);
    }

    return true;
}

GVariant* int_to_mode_enum(const GValue* value,
                           const GVariantType* expected_type,
                           gpointer user_data) {
    int v = g_value_get_int(value);

    if (v == 0) {
        return g_variant_new_string("Wide");
    } else {
        return g_variant_new_string("Split");
    }
}

}  // namespace

DeesserUi::DeesserUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
    name = "deesser";

    // loading glade widgets

    builder->get_widget("detection", detection);
    builder->get_widget("mode", mode);
    builder->get_widget("compression", compression);
    builder->get_widget("compression_label", compression_label);
    builder->get_widget("detected", detected);
    builder->get_widget("detected_label", detected_label);
    builder->get_widget("sc_listen", sc_listen);

    get_object(builder, "makeup", makeup);
    get_object(builder, "ratio", ratio);
    get_object(builder, "threshold", threshold);
    get_object(builder, "f1_freq", f1_freq);
    get_object(builder, "f2_freq", f2_freq);
    get_object(builder, "f1_level", f1_level);
    get_object(builder, "f2_level", f2_level);
    get_object(builder, "f2_q", f2_q);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("installed", this, "sensitive", flag);
    settings->bind("makeup", makeup.get(), "value", flag);
    settings->bind("ratio", ratio.get(), "value", flag);
    settings->bind("threshold", threshold.get(), "value", flag);
    settings->bind("f1-freq", f1_freq.get(), "value", flag);
    settings->bind("f2-freq", f2_freq.get(), "value", flag);
    settings->bind("f1-level", f1_level.get(), "value", flag);
    settings->bind("f2-level", f2_level.get(), "value", flag);
    settings->bind("f2-q", f2_q.get(), "value", flag);

    g_settings_bind_with_mapping(settings->gobj(), "detection",
                                 detection->gobj(), "active",
                                 G_SETTINGS_BIND_DEFAULT, detection_enum_to_int,
                                 int_to_detection_enum, nullptr, nullptr);

    g_settings_bind_with_mapping(settings->gobj(), "mode", mode->gobj(),
                                 "active", G_SETTINGS_BIND_DEFAULT,
                                 mode_enum_to_int, int_to_mode_enum, nullptr,
                                 nullptr);

    settings->set_boolean("post-messages", true);
}

DeesserUi::~DeesserUi() {
    settings->set_boolean("post-messages", false);
}

std::shared_ptr<DeesserUi> DeesserUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/deesser.glade");

    DeesserUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    grid->reference();

    return std::shared_ptr<DeesserUi>(grid);
}

void DeesserUi::on_new_compression(double value) {
    compression->set_value(1 - value);

    compression_label->set_text(level_to_str(util::linear_to_db(value)));
}

void DeesserUi::on_new_detected(double value) {
    detected->set_value(value);

    detected_label->set_text(level_to_str(util::linear_to_db(value)));
}

void DeesserUi::reset() {
    settings->reset("state");
    settings->reset("detection");
    settings->reset("mode");
    settings->reset("threshold");
    settings->reset("ratio");
    settings->reset("makeup");
    settings->reset("f1-freq");
    settings->reset("f2-freq");
    settings->reset("f1-level");
    settings->reset("f2-level");
    settings->reset("f2-q");
    settings->reset("sc-listen");
}
