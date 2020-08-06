#ifndef EQUALIZER_UI_HPP
#define EQUALIZER_UI_HPP

#include <unordered_map>
#include "plugin_ui_base.hpp"

enum FilterType : const unsigned int {
  PEAKING = 1,
  LOW_PASS = 1 << 1,
  LOW_PASS_Q = 1 << 2,
  HIGH_PASS = 1 << 3,
  HIGH_PASS_Q = 1 << 4,
  BAND_PASS = 1 << 5,
  LOW_SHELF = 1 << 6,
  LOW_SHELF_xdB = 1 << 7,
  HIGH_SHELF = 1 << 8,
  HIGH_SHELF_xdB = 1 << 9,
  NOTCH = 1 << 10,
  ALL_PASS = 1 << 11
};

struct ImportedBand {
  unsigned int type;
  float freq;
  float gain;
  float quality_factor;
  float slope_dB;
};

class EqualizerUi : public Gtk::Grid, public PluginUiBase {
 public:
  EqualizerUi(BaseObjectType* cobject,
              const Glib::RefPtr<Gtk::Builder>& builder,
              const std::string& schema,
              const std::string& schema_path,
              const std::string& schema_channel,
              const std::string& schema_channel_left_path,
              const std::string& schema_channel_right_path);
  EqualizerUi(const EqualizerUi&) = delete;
  auto operator=(const EqualizerUi&) -> EqualizerUi& = delete;
  EqualizerUi(const EqualizerUi&&) = delete;
  auto operator=(const EqualizerUi &&) -> EqualizerUi& = delete;
  ~EqualizerUi() override;

  void reset() override;

 private:
  int max_bands = 30;

  Glib::RefPtr<Gio::Settings> settings_left, settings_right;

  Glib::RefPtr<Gtk::Adjustment> nbands, input_gain, output_gain;
  Gtk::Grid *bands_grid_left = nullptr, *bands_grid_right = nullptr;
  Gtk::Button *flat_response = nullptr, *calculate_freqs = nullptr, *import_apo = nullptr;
  Gtk::ListBox* presets_listbox = nullptr;
  Gtk::Switch* split_channels = nullptr;
  Gtk::Stack* stack = nullptr;
  Gtk::StackSwitcher* stack_switcher = nullptr;
  Gtk::ComboBoxText* mode = nullptr;

  std::vector<sigc::connection> connections_bands;

  std::string presets_path = "/com/github/wwmm/pulseeffects/presets/";

  void load_preset(const std::string& file_name);

  void on_nbands_changed();

  void build_bands(Gtk::Grid* bands_grid, const Glib::RefPtr<Gio::Settings>& cfg, const int& nbands);

  void build_unified_bands(const int& nbands);

  void on_flat_response();

  void on_calculate_frequencies();

  static auto on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) -> int;

  void populate_presets_listbox();

  void on_import_apo_preset_clicked();

  bool parse_apo_filter(const std::string& line, struct ImportedBand& filter);

  void import_apo_preset(const std::string& file_path);

  std::unordered_map<std::string, FilterType> const FilterTypeMap = {
      {"PK", FilterType::PEAKING},         {"LP", FilterType::LOW_PASS},       {"LPQ", FilterType::LOW_PASS_Q},
      {"HP", FilterType::HIGH_PASS},       {"HPQ", FilterType::HIGH_PASS_Q},   {"BP", FilterType::BAND_PASS},
      {"LS", FilterType::LOW_SHELF},       {"LSC", FilterType::LOW_SHELF_xdB}, {"HS", FilterType::HIGH_SHELF},
      {"HSC", FilterType::HIGH_SHELF_xdB}, {"NO", FilterType::NOTCH},          {"AP", FilterType::ALL_PASS}};
};

#endif
