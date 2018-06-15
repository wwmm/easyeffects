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

gboolean sidechain_mode_enum_to_int(GValue* value,
                                    GVariant* variant,
                                    gpointer user_data) {
    auto v = g_variant_get_string(variant, nullptr);

    if (v == std::string("Peak")) {
        g_value_set_int(value, 0);
    } else if (v == std::string("RMS")) {
        g_value_set_int(value, 1);
    } else if (v == std::string("Low-Pass")) {
        g_value_set_int(value, 2);
    } else if (v == std::string("Uniform")) {
        g_value_set_int(value, 3);
    }

    return true;
}

GVariant* int_to_sidechain_mode_enum(const GValue* value,
                                     const GVariantType* expected_type,
                                     gpointer user_data) {
    int v = g_value_get_int(value);

    if (v == 0) {
        return g_variant_new_string("Peak");
    } else if (v == 1) {
        return g_variant_new_string("RMS");
    } else if (v == 2) {
        return g_variant_new_string("Low-Pass");
    } else {
        return g_variant_new_string("Uniform");
    }
}

gboolean sidechain_source_enum_to_int(GValue* value,
                                      GVariant* variant,
                                      gpointer user_data) {
    auto v = g_variant_get_string(variant, nullptr);

    if (v == std::string("Middle")) {
        g_value_set_int(value, 0);
    } else if (v == std::string("Side")) {
        g_value_set_int(value, 1);
    } else if (v == std::string("Left")) {
        g_value_set_int(value, 2);
    } else if (v == std::string("Right")) {
        g_value_set_int(value, 3);
    }

    return true;
}

GVariant* int_to_sidechain_source_enum(const GValue* value,
                                       const GVariantType* expected_type,
                                       gpointer user_data) {
    int v = g_value_get_int(value);

    if (v == 0) {
        return g_variant_new_string("Middle");
    } else if (v == 1) {
        return g_variant_new_string("Side");
    } else if (v == 2) {
        return g_variant_new_string("Left");
    } else {
        return g_variant_new_string("Right");
    }
}

}  // namespace

ExpanderUi::ExpanderUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
    name = "expander";

    // loading glade widgets

    builder->get_widget("em", em);
    builder->get_widget("scm", scm);
    builder->get_widget("scs", scs);
    builder->get_widget("scl", scl);

    get_object(builder, "scr", scr);
    get_object(builder, "scp", scp);
    get_object(builder, "sla", sla);
    get_object(builder, "cr", cr);
    get_object(builder, "kn", kn);
    get_object(builder, "mk", mk);
    get_object(builder, "al", al);
    get_object(builder, "at", at);
    get_object(builder, "rrl", rrl);
    get_object(builder, "rt", rt);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("scl", scl, "active", flag);
    settings->bind("scr", scr.get(), "value", flag);
    settings->bind("scp", scp.get(), "value", flag);
    settings->bind("sla", sla.get(), "value", flag);
    settings->bind("cr", cr.get(), "value", flag);
    settings->bind("kn", kn.get(), "value", flag);
    settings->bind("mk", mk.get(), "value", flag);
    settings->bind("al", al.get(), "value", flag);
    settings->bind("at", at.get(), "value", flag);
    settings->bind("rrl", rrl.get(), "value", flag);
    settings->bind("rt", rt.get(), "value", flag);

    g_settings_bind_with_mapping(
        settings->gobj(), "em", em->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
        expander_mode_enum_to_int, int_to_expander_mode_enum, nullptr, nullptr);

    g_settings_bind_with_mapping(settings->gobj(), "scm", scm->gobj(), "active",
                                 G_SETTINGS_BIND_DEFAULT,
                                 sidechain_mode_enum_to_int,
                                 int_to_sidechain_mode_enum, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings->gobj(), "scs", scs->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
        sidechain_source_enum_to_int, int_to_sidechain_source_enum, nullptr,
        nullptr);

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
