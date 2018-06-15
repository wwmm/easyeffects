#include "stereo_enhancer_ui.hpp"

namespace {

gboolean stereo_enhancer_enum_to_int(GValue* value,
                                     GVariant* variant,
                                     gpointer user_data) {
    auto v = g_variant_get_string(variant, nullptr);

    if (v == std::string("Left")) {
        g_value_set_int(value, 0);
    } else if (v == std::string("Right")) {
        g_value_set_int(value, 1);
    } else if (v == std::string("Mid (L+R)")) {
        g_value_set_int(value, 2);
    } else if (v == std::string("Side (L-R)")) {
        g_value_set_int(value, 3);
    } else if (v == std::string("Mute")) {
        g_value_set_int(value, 4);
    }

    return true;
}

GVariant* int_to_stereo_enhancer_enum(const GValue* value,
                                      const GVariantType* expected_type,
                                      gpointer user_data) {
    int v = g_value_get_int(value);

    if (v == 0) {
        return g_variant_new_string("Left");
    } else if (v == 1) {
        return g_variant_new_string("Right");
    } else if (v == 2) {
        return g_variant_new_string("Mid (L+R)");
    } else if (v == 3) {
        return g_variant_new_string("Side (L-R)");
    } else {
        return g_variant_new_string("Mute");
    }
}

}  // namespace

StereoEnhancerUi::StereoEnhancerUi(BaseObjectType* cobject,
                                   const Glib::RefPtr<Gtk::Builder>& builder,
                                   const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
    name = "stereo_enhancer";

    // loading glade widgets

    builder->get_widget("middle_source", middle_source);
    builder->get_widget("middle_phase", middle_phase);
    builder->get_widget("left_phase", left_phase);
    builder->get_widget("right_phase", right_phase);
    builder->get_widget("side_l_levelbar", side_l_levelbar);
    builder->get_widget("side_r_levelbar", side_r_levelbar);

    get_object(builder, "input_gain", input_gain);
    get_object(builder, "output_gain", output_gain);
    get_object(builder, "side_gain", side_gain);
    get_object(builder, "left_balance", left_balance);
    get_object(builder, "left_delay", left_delay);
    get_object(builder, "left_gain", left_gain);
    get_object(builder, "right_balance", right_balance);
    get_object(builder, "right_delay", right_delay);
    get_object(builder, "right_gain", right_gain);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("installed", this, "sensitive", flag);
    settings->bind("input-gain", input_gain.get(), "value", flag);
    settings->bind("output-gain", output_gain.get(), "value", flag);
    settings->bind("side-gain", side_gain.get(), "value", flag);
    settings->bind("middle-phase", middle_phase, "active", flag);
    settings->bind("left-phase", left_phase, "active", flag);
    settings->bind("right-phase", right_phase, "active", flag);
    settings->bind("left-balance", left_balance.get(), "value", flag);
    settings->bind("left-delay", left_delay.get(), "value", flag);
    settings->bind("left-gain", left_gain.get(), "value", flag);
    settings->bind("right-balance", right_balance.get(), "value", flag);
    settings->bind("right-delay", right_delay.get(), "value", flag);
    settings->bind("right-gain", right_gain.get(), "value", flag);

    g_settings_bind_with_mapping(
        settings->gobj(), "middle-source", middle_source->gobj(), "active",
        G_SETTINGS_BIND_DEFAULT, stereo_enhancer_enum_to_int,
        int_to_stereo_enhancer_enum, nullptr, nullptr);

    settings->set_boolean("post-messages", true);
}

StereoEnhancerUi::~StereoEnhancerUi() {
    settings->set_boolean("post-messages", false);

    util::debug(name + " ui destroyed");
}

void StereoEnhancerUi::on_new_side_level(const std::array<double, 2>& peak) {
    side_l_levelbar->set_value(peak[0]);
    side_r_levelbar->set_value(peak[1]);
}

void StereoEnhancerUi::reset() {
    settings->reset("state");
    settings->reset("input-gain");
    settings->reset("output-gain");
    settings->reset("middle-source");
    settings->reset("side-gain");
    settings->reset("middle-phase");
    settings->reset("left-phase");
    settings->reset("left-balance");
    settings->reset("left-delay");
    settings->reset("left-gain");
    settings->reset("right-phase");
    settings->reset("right-balance");
    settings->reset("right-delay");
    settings->reset("right-gain");
}
