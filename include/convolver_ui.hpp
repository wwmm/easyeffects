#ifndef CONVOLVER_UI_HPP
#define CONVOLVER_UI_HPP

#include <gtkmm/button.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/listbox.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/togglebutton.h>
#include <boost/filesystem.hpp>
#include <future>
#include <mutex>
#include "plugin_ui_base.hpp"

class ConvolverUi : public Gtk::Grid, public PluginUiBase {
 public:
  ConvolverUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name);
  ConvolverUi(const ConvolverUi&) = delete;
  auto operator=(const ConvolverUi&) -> ConvolverUi& = delete;
  ConvolverUi(const ConvolverUi&&) = delete;
  auto operator=(const ConvolverUi &&) -> ConvolverUi& = delete;
  ~ConvolverUi() override;

 private:
  std::string log_tag = "convolver_ui: ";

  Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, ir_width;
  Gtk::ListBox* irs_listbox;
  Gtk::MenuButton* irs_menu_button;
  Gtk::ScrolledWindow* irs_scrolled_window;
  Gtk::Button* import_irs;
  Gtk::DrawingArea *left_plot, *right_plot;
  Gtk::Label *label_file_name, *label_sampling_rate, *label_samples, *label_duration;
  Gtk::ToggleButton* show_fft;

  Pango::FontDescription font;

  boost::filesystem::path irs_dir;

  bool mouse_inside = false, show_fft_spectrum = false;
  unsigned int max_plot_points = 200;
  float mouse_intensity = 0.0F, mouse_time = 0.0F, mouse_freq = 0.0F;
  float min_left = 0.0F, max_left = 0.0F, min_right = 0.0F, max_right = 0.0F;
  float max_time = 0.0F;
  float fft_min_left = 0.0F, fft_max_left = 0.0F, fft_min_right = 0.0F, fft_max_right = 0.0F;
  float fft_max_freq = 0.0F, fft_min_freq = 0.0F;
  std::vector<float> left_mag, right_mag, time_axis;
  std::vector<float> left_spectrum, right_spectrum, freq_axis;

  Glib::RefPtr<Gio::Settings> spectrum_settings;

  std::mutex lock_guard_irs_info;

  std::vector<std::future<void>> futures;

  auto get_irs_names() -> std::vector<std::string>;

  void import_irs_file(const std::string& file_path);

  void remove_irs_file(const std::string& name);

  void populate_irs_listbox();

  auto on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) -> int;

  void on_irs_menu_button_clicked();

  void on_import_irs_clicked();

  void get_irs_info();

  void get_irs_spectrum(const int& rate);

  void draw_channel(Gtk::DrawingArea* da,
                    const Cairo::RefPtr<Cairo::Context>& ctx,
                    const std::vector<float>& magnitudes);

  void update_mouse_info_L(GdkEventMotion* event);

  void update_mouse_info_R(GdkEventMotion* event);

  auto on_left_draw(const Cairo::RefPtr<Cairo::Context>& ctx) -> bool;

  auto on_left_motion_notify_event(GdkEventMotion* event) -> bool;

  auto on_right_draw(const Cairo::RefPtr<Cairo::Context>& ctx) -> bool;

  auto on_right_motion_notify_event(GdkEventMotion* event) -> bool;

  auto on_mouse_enter_notify_event(GdkEventCrossing* event) -> bool;

  auto on_mouse_leave_notify_event(GdkEventCrossing* event) -> bool;
};

#endif
