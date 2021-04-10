/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "convolver_ui.hpp"

ConvolverUi::ConvolverUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& schema,
                         const std::string& schema_path)
    : Gtk::Box(cobject),
      PluginUiBase(builder, schema, schema_path),
      irs_dir(Glib::get_user_config_dir() + "/PulseEffects/irs"),
      string_list(Gtk::StringList::create({"initial_value"})),
      spectrum_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.spectrum")) {
  name = plugin_name::convolver;

  // loading builder widgets

  input_gain = builder->get_widget<Gtk::Scale>("input_gain");
  output_gain = builder->get_widget<Gtk::Scale>("output_gain");

  ir_width = builder->get_widget<Gtk::SpinButton>("ir_width");

  listview = builder->get_widget<Gtk::ListView>("listview");

  scrolled_window = builder->get_widget<Gtk::ScrolledWindow>("scrolled_window");

  import = builder->get_widget<Gtk::Button>("import");

  popover_menu = builder->get_widget<Gtk::Popover>("popover_menu");

  show_fft = builder->get_widget<Gtk::ToggleButton>("show_fft");
  check_left = builder->get_widget<Gtk::CheckButton>("check_left");
  check_right = builder->get_widget<Gtk::CheckButton>("check_right");

  label_sampling_rate = builder->get_widget<Gtk::Label>("label_sampling_rate");
  label_samples = builder->get_widget<Gtk::Label>("label_samples");
  label_duration = builder->get_widget<Gtk::Label>("label_duration");
  label_file_name = builder->get_widget<Gtk::Label>("label_file_name");

  drawing_area = builder->get_widget<Gtk::DrawingArea>("drawing_area");

  entry_search = builder->get_widget<Gtk::SearchEntry>("entry_search");

  setup_listview();

  plot = std::make_unique<Plot>(drawing_area);

  // builder->get_widget("irs_menu_button", irs_menu_button);

  // impulse response import and selection callbacks

  popover_menu->signal_show().connect([=, this]() {
    int height = static_cast<int>(0.5F * static_cast<float>(get_allocated_height()));

    scrolled_window->set_max_content_height(height);
  });

  import->signal_clicked().connect(sigc::mem_fun(*this, &ConvolverUi::on_import_irs_clicked));

  check_left->signal_toggled().connect([&, this]() {
    if (check_left->get_active()) {
      if (show_fft->get_active()) {
        plot_fft();
      } else {
        plot_waveform();
      }
    }
  });

  check_right->signal_toggled().connect([&, this]() {
    if (check_right->get_active()) {
      if (show_fft->get_active()) {
        plot_fft();
      } else {
        plot_waveform();
      }
    }
  });

  show_fft->signal_toggled().connect([=, this]() {
    if (show_fft->get_active()) {
      plot_fft();
    } else {
      plot_waveform();
    }
  });

  // gsettings bindings

  settings->bind("input-gain", input_gain->get_adjustment().get(), "value");
  settings->bind("output-gain", output_gain->get_adjustment().get(), "value");
  settings->bind("ir-width", ir_width->get_adjustment().get(), "value");

  // irs dir

  auto dir_exists = std::filesystem::is_directory(irs_dir);

  if (!dir_exists) {
    if (std::filesystem::create_directories(irs_dir)) {
      util::debug(log_tag + "irs directory created: " + irs_dir.string());
    } else {
      util::warning(log_tag + "failed to create irs directory: " + irs_dir.string());
    }
  } else {
    util::debug(log_tag + "irs directory already exists: " + irs_dir.string());
  }

  // reading the current configured irs file

  auto f = [=, this]() {
    std::lock_guard<std::mutex> lock(lock_guard_irs_info);

    get_irs_info();
  };

  auto future = std::async(std::launch::async, f);

  futures.emplace_back(std::move(future));

  /* this is necessary to update the interface with the irs info when a preset
     is loaded
  */

  connections.emplace_back(settings->signal_changed("kernel-path").connect([=, this](auto key) {
    auto f = [=, this]() {
      std::lock_guard<std::mutex> lock(lock_guard_irs_info);

      get_irs_info();
    };

    auto future = std::async(std::launch::async, f);

    futures.emplace_back(std::move(future));
  }));

  folder_monitor = Gio::File::create_for_path(irs_dir.string())->monitor_directory();

  folder_monitor->signal_changed().connect([=, this](const Glib::RefPtr<Gio::File>& file, auto other_f, auto event) {
    switch (event) {
      case Gio::FileMonitor::Event::CREATED: {
        string_list->append(util::remove_filename_extension(file->get_basename()));

        break;
      }
      case Gio::FileMonitor::Event::DELETED: {
        Glib::ustring name_removed = util::remove_filename_extension(file->get_basename());

        int count = 0;

        auto name = string_list->get_string(count);

        while (name.c_str() != nullptr) {
          if (name_removed == name) {
            string_list->remove(count);

            break;
          }

          count++;

          name = string_list->get_string(count);
        }

        break;
      }
      default:
        break;
    }
  });
}

ConvolverUi::~ConvolverUi() {
  futures.clear();

  util::debug(name + " ui destroyed");
}

auto ConvolverUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> ConvolverUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/convolver.ui");

  auto* ui = Gtk::Builder::get_widget_derived<ConvolverUi>(builder, "top_box", "com.github.wwmm.pulseeffects.convolver",
                                                           schema_path + "convolver/");

  auto stack_page = stack->add(*ui, plugin_name::convolver);

  return ui;
}

void ConvolverUi::setup_listview() {
  string_list->remove(0);

  auto names = get_irs_names();

  for (const auto& name : names) {
    string_list->append(name);
  }

  // filter

  auto filter =
      Gtk::StringFilter::create(Gtk::PropertyExpression<Glib::ustring>::create(GTK_TYPE_STRING_OBJECT, "string"));

  auto filter_model = Gtk::FilterListModel::create(string_list, filter);

  filter_model->set_incremental(true);

  Glib::Binding::bind_property(entry_search->property_text(), filter->property_search());

  // sorter

  auto sorter =
      Gtk::StringSorter::create(Gtk::PropertyExpression<Glib::ustring>::create(GTK_TYPE_STRING_OBJECT, "string"));

  auto sort_list_model = Gtk::SortListModel::create(filter_model, sorter);

  // setting the listview model and factory

  listview->set_model(Gtk::SingleSelection::create(sort_list_model));

  auto factory = Gtk::SignalListItemFactory::create();

  listview->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* box = Gtk::make_managed<Gtk::Box>();
    auto* label = Gtk::make_managed<Gtk::Label>();
    auto* remove = Gtk::make_managed<Gtk::Button>();

    label->set_hexpand(true);
    label->set_halign(Gtk::Align::START);

    remove->set_icon_name("user-trash-symbolic");

    box->set_spacing(6);
    box->append(*label);
    box->append(*remove);

    list_item->set_data("name", label);
    list_item->set_data("remove", remove);

    list_item->set_child(*box);
  });

  factory->signal_bind().connect([=, this](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* label = static_cast<Gtk::Label*>(list_item->get_data("name"));
    auto* remove = static_cast<Gtk::Button*>(list_item->get_data("remove"));

    auto name = list_item->get_item()->get_property<Glib::ustring>("string");

    label->set_text(name);

    auto connection_remove = remove->signal_clicked().connect([=, this]() { remove_irs_file(name); });

    //     connections.emplace_back(apply_btn->signal_clicked().connect([=, this]() {
    //       auto irs_file = irs_dir / std::filesystem::path{row->get_name() + ".irs"};

    //       settings->set_string("kernel-path", irs_file.string());
    //     }));

    list_item->set_data("connection_remove", new sigc::connection(connection_remove),
                        Glib::destroy_notify_delete<sigc::connection>);
  });

  factory->signal_unbind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    if (auto* connection = static_cast<sigc::connection*>(list_item->get_data("connection_remove"))) {
      connection->disconnect();

      list_item->set_data("connection_remove", nullptr);
    }
  });

  // selection callback

  listview->get_model()->signal_selection_changed().connect([&, this](guint position, guint n_items) {
    auto single = std::dynamic_pointer_cast<Gtk::SingleSelection>(listview->get_model());

    auto selected_name = single->get_selected_item()->get_property<Glib::ustring>("string");

    auto irs_file = irs_dir / std::filesystem::path{selected_name + ".irs"};

    settings->set_string("kernel-path", irs_file.string());
  });

  // initializing selecting the row that corresponds to the saved model

  Glib::ustring saved_name = std::filesystem::path{settings->get_string("kernel-path")}.stem().string();

  auto single = std::dynamic_pointer_cast<Gtk::SingleSelection>(listview->get_model());

  for (guint n = 0; n < single->get_n_items(); n++) {
    auto name = single->get_object(n)->get_property<Glib::ustring>("string");

    if (name == saved_name) {
      single->select_item(n, true);
    }
  }
}

void ConvolverUi::reset() {
  settings->reset("state");

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("kernel-path");

  settings->reset("ir-width");
}

auto ConvolverUi::get_irs_names() -> std::vector<std::string> {
  std::filesystem::directory_iterator it{irs_dir};
  std::vector<std::string> names;

  while (it != std::filesystem::directory_iterator{}) {
    if (std::filesystem::is_regular_file(it->status())) {
      if (it->path().extension().string() == ".irs") {
        names.emplace_back(it->path().stem().string());
      }
    }

    it++;
  }

  return names;
}

void ConvolverUi::import_irs_file(const std::string& file_path) {
  std::filesystem::path p{file_path};

  if (std::filesystem::is_regular_file(p)) {
    SndfileHandle file = SndfileHandle(file_path);

    if (file.channels() != 2 || file.frames() == 0) {
      util::warning(log_tag + " Only stereo impulse files are supported!");
      util::warning(log_tag + file_path + " loading failed");

      return;
    }

    auto out_path = irs_dir / p.filename();

    out_path.replace_extension(".irs");

    std::filesystem::copy_file(p, out_path, std::filesystem::copy_options::overwrite_existing);

    util::debug(log_tag + "imported irs file to: " + out_path.string());
  } else {
    util::warning(log_tag + p.string() + " is not a file!");
  }
}

void ConvolverUi::remove_irs_file(const std::string& name) {
  auto irs_file = irs_dir / std::filesystem::path{name + ".irs"};

  if (std::filesystem::exists(irs_file)) {
    std::filesystem::remove(irs_file);

    util::debug(log_tag + "removed irs file: " + irs_file.string());
  }
}

void ConvolverUi::on_import_irs_clicked() {
  Glib::RefPtr<Gtk::FileChooserNative> dialog;

  if (transient_window != nullptr) {
    dialog = Gtk::FileChooserNative::create(_("Import Impulse File"), *transient_window, Gtk::FileChooser::Action::OPEN,
                                            _("Open"), _("Cancel"));
  } else {
    dialog = Gtk::FileChooserNative::create(_("Import Impulse File"), Gtk::FileChooser::Action::OPEN, _("Open"),
                                            _("Cancel"));
  }

  auto dialog_filter = Gtk::FileFilter::create();

  dialog_filter->set_name(_("Impulse Response"));
  dialog_filter->add_pattern("*.irs");
  dialog_filter->add_pattern("*.wav");

  dialog->add_filter(dialog_filter);

  dialog->signal_response().connect([=, this](auto response_id) {
    switch (response_id) {
      case Gtk::ResponseType::ACCEPT: {
        import_irs_file(dialog->get_file()->get_path());

        // populate_irs_listbox();

        break;
      }
      default:
        break;
    }
  });

  dialog->set_modal(true);
  dialog->show();
}

void ConvolverUi::get_irs_info() {
  auto path = settings->get_string("kernel-path");

  if (path.c_str() == nullptr) {
    util::warning(log_tag + name + ": irs file path is null.");

    return;
  }

  util::debug(log_tag + "reading the impulse file: " + path);

  SndfileHandle file = SndfileHandle(path);

  if (file.channels() != 2 || file.frames() == 0) {
    // warning user that there is a problem

    Glib::signal_idle().connect_once([=, this]() {
      label_sampling_rate->set_text(_("Failed"));
      label_samples->set_text(_("Failed"));

      label_duration->set_text(_("Failed"));

      label_file_name->set_text(_("Could Not Load The Impulse File"));
    });

    return;
  }

  std::vector<float> kernel(file.channels() * file.frames());

  file.readf(kernel.data(), file.frames());

  float dt = 1.0F / static_cast<float>(file.samplerate());

  float duration = (static_cast<float>(file.frames()) - 1.0F) * dt;

  time_axis.resize(file.frames());
  left_mag.resize(file.frames());
  right_mag.resize(file.frames());

  for (uint n = 0; n < file.frames(); n++) {
    time_axis[n] = n * dt;

    left_mag[n] = kernel[2U * n];

    right_mag[n] = kernel[2U * n + 1];
  }

  get_irs_spectrum(file.samplerate());

  if (file.frames() > spectrum_settings->get_int("n-points")) {
    // decimating the data so we can draw it

    std::vector<float> t;
    std::vector<float> l;
    std::vector<float> r;
    std::vector<float> bin_x;
    std::vector<float> bin_l_y;
    std::vector<float> bin_r_y;

    size_t bin_size = std::ceil(file.frames() / spectrum_settings->get_int("n-points"));

    for (int n = 0; n < file.frames(); n++) {
      bin_x.emplace_back(time_axis[n]);

      bin_l_y.emplace_back(left_mag[n]);
      bin_r_y.emplace_back(right_mag[n]);

      if (bin_x.size() == bin_size) {
        const auto [min, max] = std::ranges::minmax_element(bin_l_y);

        t.emplace_back(bin_x[min - bin_l_y.begin()]);
        t.emplace_back(bin_x[max - bin_l_y.begin()]);

        l.emplace_back(*min);
        l.emplace_back(*max);

        const auto [minr, maxr] = std::ranges::minmax_element(bin_r_y);

        r.emplace_back(*minr);
        r.emplace_back(*maxr);

        bin_x.resize(0);
        bin_l_y.resize(0);
        bin_r_y.resize(0);
      }
    }

    std::ranges::for_each(t, [](auto& v) { v *= 1000.0F; });  // converting to milliseconds

    time_axis = t;
    left_mag = l;
    right_mag = r;
  }

  // ensure that the fft can be computed

  if (time_axis.size() % 2 != 0) {
    time_axis.emplace_back(static_cast<float>(time_axis.size() - 1) * dt);
  }

  if (left_mag.size() % 2 != 0) {
    left_mag.emplace_back(0.0F);
  }

  if (right_mag.size() % 2 != 0) {
    right_mag.emplace_back(0.0F);
  }

  time_axis.shrink_to_fit();
  left_mag.shrink_to_fit();
  right_mag.shrink_to_fit();

  // find min and max values

  auto min_left = std::ranges::min(left_mag);
  auto max_left = std::ranges::max(left_mag);

  auto min_right = std::ranges::min(right_mag);
  auto max_right = std::ranges::max(right_mag);

  // rescaling between 0 and 1

  for (size_t n = 0; n < left_mag.size(); n++) {
    left_mag[n] = (left_mag[n] - min_left) / (max_left - min_left);
    right_mag[n] = (right_mag[n] - min_right) / (max_right - min_right);
  }

  // updating interface with ir file info

  Glib::signal_idle().connect_once([=, this]() {
    label_sampling_rate->set_text(std::to_string(file.samplerate()) + " Hz");
    label_samples->set_text(std::to_string(file.frames()));

    label_duration->set_text(level_to_localized_string(duration, 3) + " s");

    auto fpath = std::filesystem::path{path};

    label_file_name->set_text(fpath.stem().string());

    plot_waveform();
  });
}

void ConvolverUi::get_irs_spectrum(const int& rate) {
  if (left_mag.empty() || right_mag.empty()) {
    return;
  }

  util::debug(log_tag + "calculating the impulse fft...");

  left_spectrum.resize(left_mag.size() / 2 + 1);
  right_spectrum.resize(right_mag.size() / 2 + 1);

  auto real_input = left_mag;

  for (uint n = 0; n < real_input.size(); n++) {
    // https://en.wikipedia.org/wiki/Hann_function

    auto w = 0.5F * (1.0F - cosf(2.0F * std::numbers::pi_v<float> * n / static_cast<float>(real_input.size() - 1)));

    real_input[n] *= w;
  }

  auto* complex_output = fftwf_alloc_complex(real_input.size());

  auto* plan = fftwf_plan_dft_r2c_1d(real_input.size(), real_input.data(), complex_output, FFTW_ESTIMATE);

  fftwf_execute(plan);

  for (uint i = 0; i < left_spectrum.size(); i++) {
    float sqr = complex_output[i][0] * complex_output[i][0] + complex_output[i][1] * complex_output[i][1];

    sqr /= static_cast<float>(left_spectrum.size() * left_spectrum.size());

    left_spectrum[i] = sqr;
  }

  // right channel fft

  real_input = right_mag;

  for (uint n = 0; n < real_input.size(); n++) {
    // https://en.wikipedia.org/wiki/Hann_function

    auto w = 0.5F * (1.0F - cosf(2.0F * std::numbers::pi_v<float> * n / static_cast<float>(real_input.size() - 1)));

    real_input[n] *= w;
  }

  fftwf_execute(plan);

  for (uint i = 0; i < right_spectrum.size(); i++) {
    float sqr = complex_output[i][0] * complex_output[i][0] + complex_output[i][1] * complex_output[i][1];

    sqr /= static_cast<float>(right_spectrum.size() * right_spectrum.size());

    right_spectrum[i] = sqr;
  }

  // cleaning

  fftwf_destroy_plan(plan);

  if (complex_output != nullptr) {
    fftwf_free(complex_output);
  }

  // initializing the frequency axis

  freq_axis.resize(left_spectrum.size());

  for (uint n = 0; n < left_spectrum.size(); n++) {
    freq_axis[n] = 0.5F * static_cast<float>(rate) * n / left_spectrum.size();
  }

  // initializing the logarithmic frequency axis

  auto log_axis = util::logspace(log10f(20.0F), log10f(22000.0F), spectrum_settings->get_int("n-points"));
  // auto log_axis = util::linspace(20.0F, 22000.0F, spectrum_settings->get_int("n-points"));

  std::vector<int> bin_count(log_axis.size());

  std::vector<float> l(log_axis.size());
  std::vector<float> r(log_axis.size());

  std::ranges::fill(l, 0.0F);
  std::ranges::fill(r, 0.0F);
  std::ranges::fill(bin_count, 0);

  // reducing the amount of data we have to plot and converting the frequency axis to the logarithimic scale

  for (size_t j = 0; j < freq_axis.size(); j++) {
    for (size_t n = 0; n < log_axis.size(); n++) {
      if (n > 0) {
        if (freq_axis[j] <= log_axis[n] && freq_axis[j] > log_axis[n - 1]) {
          l[n] += left_spectrum[j];
          r[n] += right_spectrum[j];

          bin_count[n]++;
        }
      } else {
        if (freq_axis[j] <= log_axis[n]) {
          l[n] += left_spectrum[j];
          r[n] += right_spectrum[j];

          bin_count[n]++;
        }
      }
    }
  }

  // fillint empty bins with their neighbors value

  for (size_t n = 0; n < bin_count.size(); n++) {
    if (bin_count[n] == 0 && n > 0) {
      l[n] = l[n - 1];
      r[n] = r[n - 1];
    }
  }

  freq_axis = log_axis;
  left_spectrum = l;
  right_spectrum = r;

  // find min and max values

  auto fft_min_left = std::ranges::min(left_spectrum);
  auto fft_max_left = std::ranges::max(left_spectrum);

  auto fft_min_right = std::ranges::min(right_spectrum);
  auto fft_max_right = std::ranges::max(right_spectrum);

  // rescaling between 0 and 1

  for (unsigned int n = 0; n < left_spectrum.size(); n++) {
    left_spectrum[n] = (left_spectrum[n] - fft_min_left) / (fft_max_left - fft_min_left);
    right_spectrum[n] = (right_spectrum[n] - fft_min_right) / (fft_max_right - fft_min_right);
  }

  Glib::signal_idle().connect_once([=, this]() { plot_fft(); });
}

void ConvolverUi::plot_waveform() {
  plot->set_plot_type(PlotType::line);

  plot->set_plot_scale(PlotScale::linear);

  plot->set_fill_bars(false);

  plot->set_line_width(static_cast<float>(spectrum_settings->get_double("line-width")));

  plot->set_x_unit("ms");

  if (check_left->get_active()) {
    plot->set_data(time_axis, left_mag);
  } else if (check_right->get_active()) {
    plot->set_data(time_axis, right_mag);
  }
}

void ConvolverUi::plot_fft() {
  plot->set_plot_type(PlotType::line);

  plot->set_plot_scale(PlotScale::logarithmic);

  plot->set_fill_bars(false);

  plot->set_line_width(static_cast<float>(spectrum_settings->get_double("line-width")));

  plot->set_x_unit("Hz");

  if (check_left->get_active()) {
    plot->set_data(freq_axis, left_spectrum);
  } else if (check_right->get_active()) {
    plot->set_data(freq_axis, right_spectrum);
  }
}
