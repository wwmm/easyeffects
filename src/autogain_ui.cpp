#include "autogain_ui.hpp"

AutoGainUi::AutoGainUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
    name = "autogain";

    // loading glade widgets

    builder->get_widget("m_level", m_level);
    builder->get_widget("s_level", s_level);
    builder->get_widget("i_level", i_level);
    builder->get_widget("r_level", r_level);
    builder->get_widget("g_level", g_level);
    builder->get_widget("m_label", m_label);
    builder->get_widget("s_label", s_label);
    builder->get_widget("i_label", i_label);
    builder->get_widget("r_label", r_label);
    builder->get_widget("g_label", g_label);

    get_object(builder, "input_gain", input_gain);
    get_object(builder, "output_gain", output_gain);
    get_object(builder, "target", target);
    get_object(builder, "weight_m", weight_m);
    get_object(builder, "weight_s", weight_s);
    get_object(builder, "weight_i", weight_i);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("installed", this, "sensitive", flag);
    settings->bind("input-gain", input_gain.get(), "value", flag);
    settings->bind("output-gain", output_gain.get(), "value", flag);
    settings->bind("target", target.get(), "value", flag);
    settings->bind("weight-m", weight_m.get(), "value", flag);
    settings->bind("weight-s", weight_s.get(), "value", flag);
    settings->bind("weight-i", weight_i.get(), "value", flag);

    settings->set_boolean("post-messages", true);
}

AutoGainUi::~AutoGainUi() {
    settings->set_boolean("post-messages", false);

    util::debug(name + " ui destroyed");
}

void AutoGainUi::reset() {
    settings->reset("state");
    settings->reset("input-gain");
    settings->reset("output-gain");
    settings->reset("target");
    settings->reset("weight_m");
    settings->reset("weight_s");
    settings->reset("weight_i");
}
