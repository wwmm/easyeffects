#include "gate_ui.hpp"

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

GateUi::GateUi(BaseObjectType* cobject,
               const Glib::RefPtr<Gtk::Builder>& refBuilder,
               const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(refBuilder, settings_name) {
    name = "gate";

    // loading glade widgets

    builder->get_widget("detection", detection);
    builder->get_widget("stereo_link", stereo_link);
    builder->get_widget("gating", gating);
    builder->get_widget("gating_label", gating_label);

    get_object("attack", attack);
    get_object("knee", knee);
    get_object("makeup", makeup);
    get_object("range", range);
    get_object("ratio", ratio);
    get_object("release", release);
    get_object("threshold", threshold);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("attack", attack, "value", flag);
    settings->bind("knee", knee, "value", flag);
    settings->bind("makeup", makeup, "value", flag);
    settings->bind("range", range, "value", flag);
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

    settings->set_boolean("post-messages", true);
}

GateUi::~GateUi() {
    settings->set_boolean("post-messages", false);
}

std::shared_ptr<GateUi> GateUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/gate.glade");

    GateUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    return std::shared_ptr<GateUi>(grid);
}

void GateUi::on_new_gating(double value) {
    gating->set_value(1 - value);

    gating_label->set_text(level_to_str(util::linear_to_db(value)));
}

void GateUi::reset() {
    settings->reset("state");
    settings->reset("detection");
    settings->reset("stereo-link");
    settings->reset("range");
    settings->reset("attack");
    settings->reset("release");
    settings->reset("threshold");
    settings->reset("ratio");
    settings->reset("knee");
    settings->reset("makeup");
}
