#include "reverb_ui.hpp"
#include <cstring>
#include "util.hpp"

namespace {

auto room_size_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  auto v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "Small") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Medium") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "Large") == 0) {
    g_value_set_int(value, 2);
  } else if (std::strcmp(v, "Tunnel-like") == 0) {
    g_value_set_int(value, 3);
  } else if (std::strcmp(v, "Large/smooth") == 0) {
    g_value_set_int(value, 4);
  } else if (std::strcmp(v, "Experimental") == 0) {
    g_value_set_int(value, 5);
  }

  return 1;
}

auto int_to_room_size_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("Small");
  }

  if (v == 1) {
    return g_variant_new_string("Medium");
  }

  if (v == 2) {
    return g_variant_new_string("Large");
  }

  if (v == 3) {
    return g_variant_new_string("Tunnel-like");
  }

  if (v == 4) {
    return g_variant_new_string("Large/smooth");
  }

  return g_variant_new_string("Experimental");
}

}  // namespace

ReverbUi::ReverbUi(BaseObjectType* cobject,
                   const Glib::RefPtr<Gtk::Builder>& builder,
                   const std::string& schema,
                   const std::string& schema_path)
    : Gtk::Grid(cobject), PluginUiBase(builder, schema, schema_path) {
  name = "reverb";

  // loading glade widgets

  builder->get_widget("room_size", room_size);
  builder->get_widget("preset_room", preset_room);
  builder->get_widget("preset_empty_walls", preset_empty_walls);
  builder->get_widget("preset_ambience", preset_ambience);
  builder->get_widget("preset_large_empty_hall", preset_large_empty_hall);
  builder->get_widget("preset_disco", preset_disco);
  builder->get_widget("preset_large_occupied_hall", preset_large_occupied_hall);
  builder->get_widget("preset_default", preset_default);
  builder->get_widget("plugin_reset", reset_button);

  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);
  get_object(builder, "predelay", predelay);
  get_object(builder, "decay_time", decay_time);
  get_object(builder, "diffusion", diffusion);
  get_object(builder, "amount", amount);
  get_object(builder, "dry", dry);
  get_object(builder, "hf_damp", hf_damp);
  get_object(builder, "bass_cut", bass_cut);
  get_object(builder, "treble_cut", treble_cut);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);
  settings->bind("predelay", predelay.get(), "value", flag);
  settings->bind("decay-time", decay_time.get(), "value", flag);
  settings->bind("diffusion", diffusion.get(), "value", flag);
  settings->bind("amount", amount.get(), "value", flag);
  settings->bind("dry", dry.get(), "value", flag);
  settings->bind("hf-damp", hf_damp.get(), "value", flag);
  settings->bind("bass-cut", bass_cut.get(), "value", flag);
  settings->bind("treble-cut", treble_cut.get(), "value", flag);

  g_settings_bind_with_mapping(settings->gobj(), "room-size", room_size->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               room_size_enum_to_int, int_to_room_size_enum, nullptr, nullptr);

  init_presets_buttons();

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

ReverbUi::~ReverbUi() {
  util::debug(name + " ui destroyed");
}

void ReverbUi::reset() {
  try {
    std::string section = (preset_type == PresetType::output) ? "output" : "input";

    update_default_key<double>(settings, "input-gain", section + ".reverb.input-gain");

    update_default_key<double>(settings, "output-gain", section + ".reverb.output-gain");

    update_default_string_key(settings, "room-size", section + ".reverb.room-size");

    update_default_key<double>(settings, "decay-time", section + ".reverb.decay-time");

    update_default_key<double>(settings, "hf-damp", section + ".reverb.hf-damp");

    update_default_key<double>(settings, "diffusion", section + ".reverb.diffusion");

    update_default_key<double>(settings, "amount", section + ".reverb.amount");

    update_default_key<double>(settings, "dry", section + ".reverb.dry");

    update_default_key<double>(settings, "predelay", section + ".reverb.predelay");

    update_default_key<double>(settings, "bass-cut", section + ".reverb.bass-cut");

    update_default_key<double>(settings, "treble-cut", section + ".reverb.treble-cut");

    util::debug(name + " plugin: successfully reset");
  } catch (std::exception& e) {
    util::debug(name + " plugin: an error occurred during reset process");
  }
}

void ReverbUi::init_presets_buttons() {
  preset_room->signal_clicked().connect([=]() {
    decay_time->set_value(0.445945);
    hf_damp->set_value(5508.46);
    room_size->set_active(4);
    diffusion->set_value(0.54);
    amount->set_value(util::linear_to_db(0.469761));
    dry->set_value(util::linear_to_db(1));
    predelay->set_value(25);
    bass_cut->set_value(257.65);
    treble_cut->set_value(20000);
  });

  preset_empty_walls->signal_clicked().connect([=]() {
    decay_time->set_value(0.505687);
    hf_damp->set_value(3971.64);
    room_size->set_active(4);
    diffusion->set_value(0.17);
    amount->set_value(util::linear_to_db(0.198884));
    dry->set_value(util::linear_to_db(1));
    predelay->set_value(13);
    bass_cut->set_value(240.453);
    treble_cut->set_value(3303.47);
  });

  preset_ambience->signal_clicked().connect([=]() {
    decay_time->set_value(1.10354);
    hf_damp->set_value(2182.58);
    room_size->set_active(4);
    diffusion->set_value(0.69);
    amount->set_value(util::linear_to_db(0.291183));
    dry->set_value(util::linear_to_db(1));
    predelay->set_value(6.5);
    bass_cut->set_value(514.079);
    treble_cut->set_value(4064.15);
  });

  preset_large_empty_hall->signal_clicked().connect([=]() {
    decay_time->set_value(2.00689);
    hf_damp->set_value(20000);
    amount->set_value(util::linear_to_db(0.366022));
    settings->reset("room-size");
    settings->reset("diffusion");
    settings->reset("dry");
    settings->reset("predelay");
    settings->reset("bass-cut");
    settings->reset("treble-cut");
  });

  preset_disco->signal_clicked().connect([=]() {
    decay_time->set_value(1);
    hf_damp->set_value(3396.49);
    amount->set_value(util::linear_to_db(0.269807));
    settings->reset("room-size");
    settings->reset("diffusion");
    settings->reset("dry");
    settings->reset("predelay");
    settings->reset("bass-cut");
    settings->reset("treble-cut");
  });

  preset_large_occupied_hall->signal_clicked().connect([=]() {
    decay_time->set_value(1.45397);
    hf_damp->set_value(9795.58);
    amount->set_value(util::linear_to_db(0.184284));
    settings->reset("room-size");
    settings->reset("diffusion");
    settings->reset("dry");
    settings->reset("predelay");
    settings->reset("bass-cut");
    settings->reset("treble-cut");
  });

  preset_default->signal_clicked().connect([=]() {
    settings->reset("decay-time");
    settings->reset("hf-damp");
    settings->reset("amount");
    settings->reset("room-size");
    settings->reset("diffusion");
    settings->reset("dry");
    settings->reset("predelay");
    settings->reset("bass-cut");
    settings->reset("treble-cut");
  });
}
