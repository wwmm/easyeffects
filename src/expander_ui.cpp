#include "expander_ui.hpp"

namespace {

gboolean expander_mode_enum_to_int(GValue* value,
                                   GVariant* variant,
                                   gpointer user_data) {
    auto v = g_variant_get_string(variant, nullptr);

    if (v == std::string("Down")) {
        g_value_set_int(value, 0);
    } else if (v == std::string("Up")) {
        g_value_set_int(value, 1);
    }

    return true;
}

GVariant* int_to_expander_mode_enum(const GValue* value,
                                    const GVariantType* expected_type,
                                    gpointer user_data) {
    int v = g_value_get_int(value);

    if (v == 0) {
        return g_variant_new_string("Down");
    } else {
        return g_variant_new_string("Up");
    }
}

}  // namespace

ExpanderUi::ExpanderUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
    name = "expander";

    // loading glade widgets

    builder->get_widget("expander_mode", expander_mode);

    // get_object(builder, "lookahead", lookahead);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;
    auto flag_get = Gio::SettingsBindFlags::SETTINGS_BIND_GET;
    auto flag_invert_boolean =
        Gio::SettingsBindFlags::SETTINGS_BIND_INVERT_BOOLEAN;

    g_settings_bind_with_mapping(settings->gobj(), "em", expander_mode->gobj(),
                                 "active", G_SETTINGS_BIND_DEFAULT,
                                 expander_mode_enum_to_int,
                                 int_to_expander_mode_enum, nullptr, nullptr);

    settings->bind("installed", this, "sensitive", flag);

    // settings->bind("lookahead", lookahead.get(), "value", flag);

    settings->set_boolean("post-messages", true);
}

ExpanderUi::~ExpanderUi() {
    settings->set_boolean("post-messages", false);

    for (auto c : connections) {
        c.disconnect();
    }

    util::debug(name + " ui destroyed");
}

void ExpanderUi::reset() {
    settings->reset("state");
    settings->reset("scm");
    settings->reset("sla");
    settings->reset("scl");
    settings->reset("scs");
    settings->reset("scr");
    settings->reset("scp");
    settings->reset("em");
    settings->reset("al");
    settings->reset("at");
    settings->reset("rrl");
    settings->reset("rt");
    settings->reset("cr");
    settings->reset("kn");
    settings->reset("mk");
}
