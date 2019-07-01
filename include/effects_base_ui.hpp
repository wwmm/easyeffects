#ifndef EFFECTS_BASE_UI_HPP
#define EFFECTS_BASE_UI_HPP

#include <giomm/settings.h>
#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/listbox.h>
#include <gtkmm/stack.h>
#include <memory>
#include <vector>
#include "app_info_ui.hpp"
#include "pulse_manager.hpp"
#include "spectrum_ui.hpp"

class EffectsBaseUi {
 public:
  EffectsBaseUi(const Glib::RefPtr<Gtk::Builder>& builder,
                const Glib::RefPtr<Gio::Settings>& refSettings,
                PulseManager* pulse_manager);

  virtual ~EffectsBaseUi();

  void on_app_added(std::shared_ptr<AppInfo> app_info);
  void on_app_changed(std::shared_ptr<AppInfo> app_info);
  void on_app_removed(uint idx);

 protected:
  Glib::RefPtr<Gio::Settings> settings;
  Gtk::ListBox* listbox;
  Gtk::Stack* stack;

  SpectrumUi* spectrum_ui;

  std::vector<sigc::connection> connections;

  template <typename T>
  void add_to_listbox(T p) {
    auto row = Gtk::manage(new Gtk::ListBoxRow());
    auto eventBox = Gtk::manage(new Gtk::EventBox());

    eventBox->add(*p->listbox_control);

    row->add(*eventBox);
    row->set_name(p->name);
    row->set_margin_bottom(6);
    row->set_margin_right(6);
    row->set_margin_left(6);

    std::vector<Gtk::TargetEntry> listTargets;

    auto entry = Gtk::TargetEntry("Gtk::ListBoxRow", Gtk::TARGET_SAME_APP, 0);

    listTargets.push_back(entry);

    eventBox->drag_source_set(listTargets, Gdk::MODIFIER_MASK, Gdk::ACTION_MOVE);

    eventBox->drag_dest_set(listTargets, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);

    eventBox->signal_drag_data_get().connect(
        [=](const Glib::RefPtr<Gdk::DragContext>& context, Gtk::SelectionData& selection_data, guint info, guint time) {
          selection_data.set(selection_data.get_target(), p->name);
        });

    eventBox->signal_drag_data_received().connect([=](const Glib::RefPtr<Gdk::DragContext>& context, int x, int y,
                                                      const Gtk::SelectionData& selection_data, guint info,
                                                      guint time) {
      const int length = selection_data.get_length();

      if ((length >= 0) && (selection_data.get_format() == 8)) {
        auto src = selection_data.get_data_as_string();
        auto dst = p->name;

        if (src != dst) {
          auto order = Glib::Variant<std::vector<std::string>>();

          settings->get_value("plugins", order);

          auto vorder = order.get();

          auto r1 = std::find(std::begin(vorder), std::end(vorder), src);

          if (r1 != std::end(vorder)) {
            // for (auto v : vorder) {
            //   std::cout << v << std::endl;
            // }

            vorder.erase(r1);

            auto r2 = std::find(std::begin(vorder), std::end(vorder), dst);

            vorder.insert(r2, src);

            settings->set_string_array("plugins", vorder);

            // std::cout << "" << std::endl;

            // for (auto v : vorder) {
            //   std::cout << v << std::endl;
            // }
          }
        }
      }

      context->drag_finish(false, false, time);
    });

    eventBox->signal_drag_begin().connect([=](const Glib::RefPtr<Gdk::DragContext>& context) {
      auto w = row->get_allocated_width();
      auto h = row->get_allocated_height();

      auto surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, w, h);

      auto ctx = Cairo::Context::create(surface);

      auto styleContext = row->get_style_context();

      styleContext->add_class("drag-listboxrow-icon");

      gtk_widget_draw(GTK_WIDGET(row->gobj()), ctx->cobj());

      styleContext->remove_class("drag-listboxrow-icon");

      context->set_icon(surface);
    });

    listbox->add(*row);
  }

 private:
  Gtk::Box* apps_box;

  PulseManager* pm;

  Gtk::Box* placeholder_spectrum;

  std::vector<AppInfoUi*> apps_list;

  int on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2);
};

#endif
