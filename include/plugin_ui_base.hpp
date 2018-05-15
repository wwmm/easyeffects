#ifndef PLUGIN_UI_BASE_HPP
#define PLUGIN_UI_BASE_HPP

#include <giomm/settings.h>
#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/levelbar.h>
#include <gtkmm/switch.h>

class PluginUiBase {
   public:
    PluginUiBase(const Glib::RefPtr<Gtk::Builder>& refBuilder,
                 std::string settings_name);
    virtual ~PluginUiBase();

    Gtk::Box* listbox_control;

    void on_new_input_level(const std::array<double, 2>& peak);
    void on_new_output_level(const std::array<double, 2>& peak);

   protected:
    Glib::RefPtr<Gtk::Builder> builder;
    Glib::RefPtr<Gio::Settings> settings;

    Gtk::Switch* enable;
    Gtk::Box* controls;
    Gtk::Image* img_state;

    Gtk::LevelBar *input_level_left, *input_level_right;
    Gtk::LevelBar *output_level_left, *output_level_right;
    Gtk::Label *input_level_left_label, *input_level_right_label;
    Gtk::Label *output_level_left_label, *output_level_right_label;

    template <typename T>
    void get_object(std::string name, T& object) {
        object = (T)builder->get_object(name).get();
    }

   private:
    std::string level_to_str(double value);

    template <typename T1, typename T2, typename T3, typename T4>
    void update_level(const T1& w_left,
                      const T2& w_left_label,
                      const T3& w_right,
                      const T4& w_right_label,
                      const std::array<double, 2>& peak) {
        auto left = peak[0];
        auto right = peak[1];

        if (left >= -99) {
            w_left->set_value(pow(10, left / 10));
            w_left_label->set_text(level_to_str(left));
        } else {
            w_left->set_value(0);
            w_left_label->set_text("-99");
        }

        if (right >= -99) {
            w_right->set_value(pow(10, right / 10));
            w_right_label->set_text(level_to_str(right));
        } else {
            w_right->set_value(0);
            w_right_label->set_text("-99");
        }
    }
};

#endif
