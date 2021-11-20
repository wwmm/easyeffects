/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "plugins_menu.hpp"

namespace ui::plugins_menu {

using namespace std::string_literals;

auto constexpr log_tag = "plugins_menu: ";

struct _PluginsMenu {
  GtkPopover parent_instance;

  GtkScrolledWindow* scrolled_window;

  GtkListView* listview;

  GtkSearchEntry* plugins_search;

  GtkStringList* string_list;

  GSettings* settings;

  app::Application* application;

  std::unordered_map<std::string, std::string> plugins_names;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

G_DEFINE_TYPE(PluginsMenu, plugins_menu, GTK_TYPE_POPOVER)

void setup_listview(PluginsMenu* self) {
  for (const auto& translated_name : std::views::values(self->plugins_names)) {
    gtk_string_list_append(self->string_list, translated_name.c_str());
  }

  // filter

  auto* filter = gtk_string_filter_new(gtk_property_expression_new(GTK_TYPE_STRING_OBJECT, nullptr, "string"));

  auto* filter_model = gtk_filter_list_model_new(G_LIST_MODEL(self->string_list), GTK_FILTER(filter));

  gtk_filter_list_model_set_incremental(filter_model, 1);

  g_object_bind_property(self->plugins_search, "text", filter, "search", G_BINDING_DEFAULT);

  // sorter

  auto* sorter = gtk_string_sorter_new(gtk_property_expression_new(GTK_TYPE_STRING_OBJECT, nullptr, "string"));

  auto* sorter_model = gtk_sort_list_model_new(G_LIST_MODEL(filter_model), GTK_SORTER(sorter));

  // setting the listview model and factory

  auto* selection = gtk_no_selection_new(G_LIST_MODEL(sorter_model));

  gtk_list_view_set_model(self->listview, GTK_SELECTION_MODEL(selection));

  g_object_unref(selection);

  auto* factory = gtk_signal_list_item_factory_new();

  // setting the factory callbacks

  g_signal_connect(
      factory, "setup", G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PluginsMenu* self) {
        auto* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        auto* label = gtk_label_new(nullptr);
        auto* button = gtk_button_new_from_icon_name("list-add-symbolic");

        gtk_widget_set_halign(GTK_WIDGET(label), GTK_ALIGN_START);
        gtk_widget_set_hexpand(GTK_WIDGET(label), 1);

        gtk_box_append(GTK_BOX(box), GTK_WIDGET(label));
        gtk_box_append(GTK_BOX(box), GTK_WIDGET(button));

        gtk_list_item_set_activatable(item, 0);
        gtk_list_item_set_child(item, GTK_WIDGET(box));

        g_object_set_data(G_OBJECT(item), "name", label);
        g_object_set_data(G_OBJECT(item), "add", button);

        g_signal_connect(
            button, "clicked", G_CALLBACK(+[](GtkButton* btn, PluginsMenu* self) {
              if (auto* string_object = GTK_STRING_OBJECT(g_object_get_data(G_OBJECT(btn), "string-object"));
                  string_object != nullptr) {
                auto* translated_name = gtk_string_object_get_string(string_object);

                std::string key_name;

                for (const auto& [key, value] : self->plugins_names) {
                  if (translated_name == value) {
                    key_name = key;

                    break;
                  }
                }

                auto list = util::gchar_array_to_vector(g_settings_get_strv(self->settings, "plugins"));

                if (std::ranges::find(list, key_name) != list.end()) {
                  return;
                }

                static const auto limiter_plugins = std::to_array({plugin_name::limiter, plugin_name::maximizer});

                if (!list.empty() && std::any_of(limiter_plugins.begin(), limiter_plugins.end(),
                                                 [&](const auto& str) { return str == list.at(list.size() - 1U); })) {
                  // If the user is careful protecting his/her device with a plugin of
                  // type limiter at the last position of the filter chain, we follow
                  // this behaviour inserting the new plugin at the second last position

                  list.insert(list.cend() - 1U, key_name);
                } else {
                  list.push_back(key_name);
                }

                g_settings_set_strv(self->settings, "plugins", util::make_gchar_pointer_vector(list).data());
              }
            }),
            self);
      }),
      self);

  g_signal_connect(factory, "bind",
                   G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PluginsMenu* self) {
                     auto* label = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "name"));
                     auto* btn_add = static_cast<GtkButton*>(g_object_get_data(G_OBJECT(item), "add"));

                     auto* child_item = gtk_list_item_get_item(item);
                     auto* string_object = GTK_STRING_OBJECT(child_item);

                     g_object_set_data(G_OBJECT(btn_add), "string-object", string_object);

                     auto* translated_name = gtk_string_object_get_string(GTK_STRING_OBJECT(child_item));

                     gtk_label_set_text(label, translated_name);

                     gtk_accessible_update_property(GTK_ACCESSIBLE(btn_add), GTK_ACCESSIBLE_PROPERTY_LABEL,
                                                    (_("Add") + " "s + translated_name).c_str(), -1);
                   }),
                   self);

  gtk_list_view_set_factory(self->listview, factory);

  g_object_unref(factory);
}

void setup(PluginsMenu* self, app::Application* application, PipelineType pipeline_type) {
  self->application = application;

  switch (pipeline_type) {
    case PipelineType::input: {
      self->settings = g_settings_new("com.github.wwmm.easyeffects.streaminputs");

      break;
    }
    case PipelineType::output: {
      self->settings = g_settings_new("com.github.wwmm.easyeffects.streamoutputs");

      break;
    }
  }

  setup_listview(self);
}

void show(GtkWidget* widget) {
  auto* self = EE_PLUGINS_MENU(widget);

  auto* active_window = gtk_application_get_active_window(GTK_APPLICATION(self->application));

  auto active_window_height = gtk_widget_get_allocated_height(GTK_WIDGET(active_window));

  const int menu_height = static_cast<int>(0.5F * static_cast<float>(active_window_height));

  gtk_scrolled_window_set_max_content_height(self->scrolled_window, menu_height);

  GTK_WIDGET_CLASS(plugins_menu_parent_class)->show(widget);
}

void dispose(GObject* object) {
  auto* self = EE_PLUGINS_MENU(object);

  for (auto& c : self->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  self->connections.clear();
  self->gconnections.clear();

  g_object_unref(self->settings);

  util::debug(log_tag + "disposed"s);

  G_OBJECT_CLASS(plugins_menu_parent_class)->dispose(object);
}

void plugins_menu_class_init(PluginsMenuClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  widget_class->show = show;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/plugins_menu.ui");

  gtk_widget_class_bind_template_child(widget_class, PluginsMenu, scrolled_window);
  gtk_widget_class_bind_template_child(widget_class, PluginsMenu, listview);
  gtk_widget_class_bind_template_child(widget_class, PluginsMenu, plugins_search);

  //   gtk_widget_class_bind_template_callback(widget_class, on_show_pluginsed_apps);
  //   gtk_widget_class_bind_template_callback(widget_class, on_add_to_plugins);
}

void plugins_menu_init(PluginsMenu* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->string_list = gtk_string_list_new(nullptr);

  self->plugins_names =
      std::unordered_map<std::string, std::string>({{plugin_name::autogain, _("Autogain")},
                                                    {plugin_name::bass_enhancer, _("Bass Enhancer")},
                                                    {plugin_name::bass_loudness, _("Bass Loudness")},
                                                    {plugin_name::compressor, _("Compressor")},
                                                    {plugin_name::convolver, _("Convolver")},
                                                    {plugin_name::crossfeed, _("Crossfeed")},
                                                    {plugin_name::crystalizer, _("Crystalizer")},
                                                    {plugin_name::deesser, _("Deesser")},
                                                    {plugin_name::delay, _("Delay")},
                                                    {plugin_name::echo_canceller, _("Echo Canceller")},
                                                    {plugin_name::equalizer, _("Equalizer")},
                                                    {plugin_name::exciter, _("Exciter")},
                                                    {plugin_name::filter, _("Filter")},
                                                    {plugin_name::gate, _("Gate")},
                                                    {plugin_name::limiter, _("Limiter")},
                                                    {plugin_name::loudness, _("Loudness")},
                                                    {plugin_name::maximizer, _("Maximizer")},
                                                    {plugin_name::multiband_compressor, _("Multiband Compressor")},
                                                    {plugin_name::multiband_gate, _("Multiband Gate")},
                                                    {plugin_name::pitch, _("Pitch")},
                                                    {plugin_name::reverb, _("Reverberation")},
                                                    {plugin_name::rnnoise, _("Noise Reduction")},
                                                    {plugin_name::stereo_tools, _("Stereo Tools")}});
}

auto create() -> PluginsMenu* {
  return static_cast<PluginsMenu*>(g_object_new(EE_TYPE_PLUGINS_MENU, nullptr));
}

}  // namespace ui::plugins_menu