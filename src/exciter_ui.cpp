#include "exciter_ui.hpp"

ExciterUi::ExciterUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& refBuilder,
                     const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(refBuilder, settings_name) {
    name = "exciter";

    // loading glade widgets

    builder->get_widget("harmonics_levelbar", harmonics_levelbar);
    builder->get_widget("harmonics_levelbar_label", harmonics_levelbar_label);
    builder->get_widget("ceil_active", ceil_active);
    builder->get_widget("listen", listen);

    get_object("amount", amount);
    get_object("blend", blend);
    get_object("ceil", ceilv);
    get_object("harmonics", harmonics);
    get_object("scope", scope);
    get_object("input_gain", input_gain);
    get_object("output_gain", output_gain);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("installed", this, "sensitive", flag);
    settings->bind("amount", amount.get(), "value", flag);
    settings->bind("harmonics", harmonics.get(), "value", flag);
    settings->bind("scope", scope.get(), "value", flag);
    settings->bind("ceil", ceilv.get(), "value", flag);
    settings->bind("blend", blend.get(), "value", flag);
    settings->bind("input-gain", input_gain.get(), "value", flag);
    settings->bind("output-gain", output_gain.get(), "value", flag);
    settings->bind("listen", listen, "active", flag);
    settings->bind("ceil-active", ceil_active, "active", flag);

    settings->set_boolean("post-messages", true);
}

ExciterUi::~ExciterUi() {
    settings->set_boolean("post-messages", false);
}

std::shared_ptr<ExciterUi> ExciterUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/exciter.glade");

    ExciterUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    return std::shared_ptr<ExciterUi>(grid);
}

void ExciterUi::on_new_harmonics_level(double value) {
    harmonics_levelbar->set_value(value);

    harmonics_levelbar_label->set_text(level_to_str(util::linear_to_db(value)));
}

void ExciterUi::reset() {
    settings->reset("state");
    settings->reset("input-gain");
    settings->reset("output-gain");
    settings->reset("amount");
    settings->reset("harmonics");
    settings->reset("scope");
    settings->reset("ceil");
    settings->reset("blend");
    settings->reset("ceil-active");
    settings->reset("listen");
}
