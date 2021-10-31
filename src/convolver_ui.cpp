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

#include "convolver_ui.hpp"

ConvolverUi::ConvolverUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& schema,
                         const std::string& schema_path)
    : Gtk::Box(cobject),
      PluginUiBase(builder, schema, schema_path),
      irs_dir(Glib::get_user_config_dir() + "/easyeffects/irs"),
      string_list(Gtk::StringList::create({"initial_value"})),
      spectrum_settings(Gio::Settings::create("com.github.wwmm.easyeffects.spectrum")) {
  name = plugin_name::convolver;

  // irs dir

  if (!std::filesystem::is_directory(irs_dir)) {
    if (std::filesystem::create_directories(irs_dir)) {
      util::debug(log_tag + "irs directory created: " + irs_dir.string());
    } else {
      util::warning(log_tag + "failed to create irs directory: " + irs_dir.string());
    }
  } else {
    util::debug(log_tag + "irs directory already exists: " + irs_dir.string());
  }

  // loading builder widgets

  ir_width = builder->get_widget<Gtk::SpinButton>("ir_width");

  listview = builder->get_widget<Gtk::ListView>("listview");

  scrolled_window = builder->get_widget<Gtk::ScrolledWindow>("scrolled_window");

  import = builder->get_widget<Gtk::Button>("import");
  button_combine_kernels = builder->get_widget<Gtk::Button>("button_combine_kernels");

  popover_import = builder->get_widget<Gtk::Popover>("popover_import");
  popover_combine = builder->get_widget<Gtk::Popover>("popover_combine");

  dropdown_kernel_1 = builder->get_widget<Gtk::DropDown>("dropdown_kernel_1");
  dropdown_kernel_2 = builder->get_widget<Gtk::DropDown>("dropdown_kernel_2");

  show_fft = builder->get_widget<Gtk::ToggleButton>("show_fft");
  check_left = builder->get_widget<Gtk::CheckButton>("check_left");
  check_right = builder->get_widget<Gtk::CheckButton>("check_right");

  label_sampling_rate = builder->get_widget<Gtk::Label>("label_sampling_rate");
  label_samples = builder->get_widget<Gtk::Label>("label_samples");
  label_duration = builder->get_widget<Gtk::Label>("label_duration");
  label_file_name = builder->get_widget<Gtk::Label>("label_file_name");

  drawing_area = builder->get_widget<Gtk::DrawingArea>("drawing_area");

  entry_search = builder->get_widget<Gtk::SearchEntry>("entry_search");

  combined_kernel_name = builder->get_widget<Gtk::Entry>("combined_kernel_name");

  spinner_combine_kernel = builder->get_widget<Gtk::Spinner>("spinner_combine_kernel");

  setup_listview();

  setup_dropdown_kernels(dropdown_kernel_1, string_list);
  setup_dropdown_kernels(dropdown_kernel_2, string_list);

  plot = std::make_unique<Plot>(drawing_area);

  popover_import->signal_show().connect([=, this]() {
    int height = static_cast<int>(0.5F * static_cast<float>(get_allocated_height()));

    scrolled_window->set_max_content_height(height);
  });

  import->signal_clicked().connect(sigc::mem_fun(*this, &ConvolverUi::on_import_irs_clicked));

  button_combine_kernels->signal_clicked().connect([=, this]() {
    if (string_list->get_n_items() == 0) {
      return;
    }

    spinner_combine_kernel->start();

    const auto kernel_1_name = dropdown_kernel_1->get_selected_item()->get_property<Glib::ustring>("string").raw();
    const auto kernel_2_name = dropdown_kernel_2->get_selected_item()->get_property<Glib::ustring>("string").raw();

    auto output_file_name = combined_kernel_name->get_text();

    if (output_file_name.empty()) {
      combined_kernel_name->set_css_classes({"error"});

      combined_kernel_name->grab_focus();

      spinner_combine_kernel->stop();
    } else {
      combined_kernel_name->remove_css_class("error");

      /*
        The current code convolving the impulse responses is doing direct convolution. It can very slow depending on the
        size of each kernel. So we do not want to do it in the main thread
      */

      mythreads.emplace_back(  // Using emplace_back here makes sense
          [=, this]() { combine_kernels(kernel_1_name, kernel_2_name, output_file_name); });
    }
  });

  check_left->signal_toggled().connect([&, this]() {
    if (check_left->get_active()) {
      (show_fft->get_active()) ? plot_fft() : plot_waveform();
    }
  });

  check_right->signal_toggled().connect([&, this]() {
    if (check_right->get_active()) {
      (show_fft->get_active()) ? plot_fft() : plot_waveform();
    }
  });

  show_fft->signal_toggled().connect([=, this]() { (show_fft->get_active()) ? plot_fft() : plot_waveform(); });

  // gsettings bindings

  settings->bind("ir-width", ir_width->get_adjustment().get(), "value");

  setup_input_output_gain(builder);

  // Reading the current configured irs file

  mythreads.emplace_back([this]() {  // Using emplace_back here makes sense
    std::scoped_lock<std::mutex> lock(lock_guard_irs_info);

    get_irs_info();
  });

  /* this is necessary to update the interface with the irs info when a preset
     is loaded
  */

  connections.push_back(settings->signal_changed("kernel-path").connect([=, this](const auto& key) {
    mythreads.emplace_back([this]() {
      std::scoped_lock<std::mutex> lock(lock_guard_irs_info);

      get_irs_info();
    });
  }));

  folder_monitor = Gio::File::create_for_path(irs_dir.string())->monitor_directory();

  folder_monitor->signal_changed().connect(
      [=, this](const Glib::RefPtr<Gio::File>& file, const auto& other_f, const auto& event) {
        const auto irs_filename = util::remove_filename_extension(file->get_basename());

        if (irs_filename.empty()) {
          util::warning(log_tag + "can't retrieve information about irs file");

          return;
        }

        switch (event) {
          case Gio::FileMonitor::Event::CREATED: {
            for (guint n = 0; n < string_list->get_n_items(); n++) {
              if (string_list->get_string(n) == irs_filename) {
                return;
              }
            }

            string_list->append(irs_filename);

            break;
          }
          case Gio::FileMonitor::Event::DELETED: {
            for (guint n = 0; n < string_list->get_n_items(); n++) {
              if (string_list->get_string(n) == irs_filename) {
                string_list->remove(n);

                break;
              }
            }

            break;
          }
          default:
            break;
        }
      });
}

ConvolverUi::~ConvolverUi() {
  for (auto& t : mythreads) {
    t.join();
  }

  mythreads.clear();

  util::debug(name + " ui destroyed");
}

auto ConvolverUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> ConvolverUi* {
  const auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/convolver.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<ConvolverUi>(
      builder, "top_box", "com.github.wwmm.easyeffects.convolver", schema_path + "convolver/");

  stack->add(*ui, plugin_name::convolver);

  return ui;
}

void ConvolverUi::setup_listview() {
  string_list->remove(0);

  for (const auto& name : get_irs_names()) {
    string_list->append(name);
  }

  // filter

  auto filter =
      Gtk::StringFilter::create(Gtk::PropertyExpression<Glib::ustring>::create(GTK_TYPE_STRING_OBJECT, "string"));

  auto filter_model = Gtk::FilterListModel::create(string_list, filter);

  filter_model->set_incremental(true);

  Glib::Binding::bind_property(entry_search->property_text(), filter->property_search());

  // sorter

  const auto sorter =
      Gtk::StringSorter::create(Gtk::PropertyExpression<Glib::ustring>::create(GTK_TYPE_STRING_OBJECT, "string"));

  const auto sort_list_model = Gtk::SortListModel::create(filter_model, sorter);

  // setting the listview model and factory

  listview->set_model(Gtk::NoSelection::create(sort_list_model));

  auto factory = Gtk::SignalListItemFactory::create();

  listview->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const box = Gtk::make_managed<Gtk::Box>();
    auto* const label = Gtk::make_managed<Gtk::Label>();
    auto* const load = Gtk::make_managed<Gtk::Button>();
    auto* const remove = Gtk::make_managed<Gtk::Button>();

    label->set_hexpand(true);
    label->set_halign(Gtk::Align::START);

    load->set_label(_("Load"));

    remove->set_icon_name("user-trash-symbolic");

    box->set_spacing(6);
    box->append(*label);
    box->append(*load);
    box->append(*remove);

    list_item->set_data("name", label);
    list_item->set_data("load", load);
    list_item->set_data("remove", remove);

    list_item->set_child(*box);
  });

  factory->signal_bind().connect([=, this](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const label = static_cast<Gtk::Label*>(list_item->get_data("name"));
    auto* const load = static_cast<Gtk::Button*>(list_item->get_data("load"));
    auto* const remove = static_cast<Gtk::Button*>(list_item->get_data("remove"));

    const auto name = list_item->get_item()->get_property<Glib::ustring>("string");

    label->set_text(name);

    load->update_property(Gtk::Accessible::Property::LABEL,
                          util::glib_value(Glib::ustring(_("Load Impulse")) + " " + name));

    remove->update_property(Gtk::Accessible::Property::LABEL,
                            util::glib_value(Glib::ustring(_("Remove Impulse")) + " " + name));

    auto connection_load = load->signal_clicked().connect([=, this]() {
      const auto irs_file = irs_dir / std::filesystem::path{name + irs_ext};

      settings->set_string("kernel-path", irs_file.c_str());
    });

    auto connection_remove = remove->signal_clicked().connect([=, this]() { remove_irs_file(name.raw()); });

    list_item->set_data("connection_load", new sigc::connection(connection_load),
                        Glib::destroy_notify_delete<sigc::connection>);

    list_item->set_data("connection_remove", new sigc::connection(connection_remove),
                        Glib::destroy_notify_delete<sigc::connection>);
  });

  factory->signal_unbind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    if (auto* connection = static_cast<sigc::connection*>(list_item->get_data("connection_load"))) {
      connection->disconnect();

      list_item->set_data("connection_load", nullptr);
    }

    if (auto* connection = static_cast<sigc::connection*>(list_item->get_data("connection_remove"))) {
      connection->disconnect();

      list_item->set_data("connection_remove", nullptr);
    }
  });
}

void ConvolverUi::setup_dropdown_kernels(Gtk::DropDown* dropdown, const Glib::RefPtr<Gtk::StringList>& string_list) {
  // set the search expression

  dropdown->set_expression(Gtk::PropertyExpression<Glib::ustring>::create(GTK_TYPE_STRING_OBJECT, "string"));

  // sorter

  const auto sorter =
      Gtk::StringSorter::create(Gtk::PropertyExpression<Glib::ustring>::create(GTK_TYPE_STRING_OBJECT, "string"));

  const auto sort_list_model = Gtk::SortListModel::create(string_list, sorter);

  // setting the dropdown model and factory

  const auto selection_model = Gtk::SingleSelection::create(sort_list_model);

  dropdown->set_model(selection_model);

  auto factory = Gtk::SignalListItemFactory::create();

  dropdown->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const box = Gtk::make_managed<Gtk::Box>();
    auto* const label = Gtk::make_managed<Gtk::Label>();

    label->set_hexpand(true);
    label->set_halign(Gtk::Align::START);

    box->set_spacing(6);
    box->append(*label);

    // setting list_item data

    list_item->set_data("name", label);

    list_item->set_child(*box);
  });

  factory->signal_bind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const label = static_cast<Gtk::Label*>(list_item->get_data("name"));

    const auto name = list_item->get_item()->get_property<Glib::ustring>("string");

    label->set_name(name);
    label->set_text(name);
  });
}

void ConvolverUi::reset() {
  bypass->set_active(false);

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("kernel-path");

  settings->reset("ir-width");
}

auto ConvolverUi::get_irs_names() -> std::vector<Glib::ustring> {
  std::vector<Glib::ustring> names;

  for (std::filesystem::directory_iterator it{irs_dir}; it != std::filesystem::directory_iterator{}; ++it) {
    if (std::filesystem::is_regular_file(it->status())) {
      if (it->path().extension().c_str() == irs_ext) {
        names.push_back(it->path().stem().string());
      }
    }
  }

  return names;
}

void ConvolverUi::import_irs_file(const std::string& file_path) {
  std::filesystem::path p{file_path};

  if (std::filesystem::is_regular_file(p)) {
    if (SndfileHandle file = SndfileHandle(file_path.c_str()); file.channels() != 2 || file.frames() == 0) {
      util::warning(log_tag + " Only stereo impulse files are supported!");
      util::warning(log_tag + file_path + " loading failed");

      return;
    }

    auto out_path = irs_dir / p.filename();

    out_path.replace_extension(irs_ext);

    std::filesystem::copy_file(p, out_path, std::filesystem::copy_options::overwrite_existing);

    util::debug(log_tag + "imported irs file to: " + out_path.string());
  } else {
    util::warning(log_tag + p.string() + " is not a file!");
  }
}

void ConvolverUi::remove_irs_file(const std::string& name) {
  const auto irs_file = irs_dir / std::filesystem::path{name + irs_ext};

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

  dialog->signal_response().connect([=, this](const auto& response_id) {
    switch (response_id) {
      case Gtk::ResponseType::ACCEPT: {
        import_irs_file(dialog->get_file()->get_path());

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
  const auto path = settings->get_string("kernel-path");

  if (path.empty()) {
    util::warning(log_tag + name + ": irs file path is null.");

    return;
  }

  auto [rate, kernel_L, kernel_R] = read_kernel(path);

  if (rate == 0) {
    // warning user that there is a problem

    connections.push_back(Glib::signal_idle().connect([=, this]() {
      label_sampling_rate->set_text(_("Failed"));
      label_samples->set_text(_("Failed"));

      label_duration->set_text(_("Failed"));

      label_file_name->set_text(_("Could Not Load The Impulse File"));

      return false;
    }));

    return;
  }

  const float dt = 1.0F / static_cast<float>(rate);

  const float duration = (static_cast<float>(kernel_L.size()) - 1.0F) * dt;

  time_axis.resize(kernel_L.size());

  left_mag = kernel_L;
  right_mag = kernel_R;

  for (size_t n = 0; n < time_axis.size(); n++) {
    time_axis[n] = static_cast<float>(n) * dt;
  }

  get_irs_spectrum(rate);

  size_t bin_size = (drawing_area->get_width() > 0) ? time_axis.size() / drawing_area->get_width() : 0;

  if (bin_size > 0) {
    // decimating the data so we can draw it

    std::vector<float> t;
    std::vector<float> l;
    std::vector<float> r;
    std::vector<float> bin_x;
    std::vector<float> bin_l_y;
    std::vector<float> bin_r_y;

    for (size_t n = 0; n < time_axis.size(); n++) {
      bin_x.push_back(time_axis[n]);

      bin_l_y.push_back(left_mag[n]);
      bin_r_y.push_back(right_mag[n]);

      if (bin_x.size() == bin_size) {
        const auto [min, max] = std::ranges::minmax_element(bin_l_y);

        t.push_back(bin_x[min - bin_l_y.begin()]);
        t.push_back(bin_x[max - bin_l_y.begin()]);

        l.push_back(*min);
        l.push_back(*max);

        const auto [minr, maxr] = std::ranges::minmax_element(bin_r_y);

        r.push_back(*minr);
        r.push_back(*maxr);

        bin_x.resize(0);
        bin_l_y.resize(0);
        bin_r_y.resize(0);
      }
    }

    time_axis = t;
    left_mag = l;
    right_mag = r;
  }

  time_axis.shrink_to_fit();
  left_mag.shrink_to_fit();
  right_mag.shrink_to_fit();

  // find min and max values

  const auto min_left = std::ranges::min(left_mag);
  const auto max_left = std::ranges::max(left_mag);

  const auto min_right = std::ranges::min(right_mag);
  const auto max_right = std::ranges::max(right_mag);

  // rescaling between 0 and 1

  for (size_t n = 0U; n < left_mag.size(); n++) {
    left_mag[n] = (left_mag[n] - min_left) / (max_left - min_left);
    right_mag[n] = (right_mag[n] - min_right) / (max_right - min_right);
  }

  // updating interface with ir file info

  auto rate_copy = rate;
  auto n_samples = kernel_L.size();

  connections.push_back(Glib::signal_idle().connect([=, this]() {
    label_sampling_rate->set_text(Glib::ustring::format(rate_copy) + " Hz");
    label_samples->set_text(Glib::ustring::format(n_samples));

    label_duration->set_text(level_to_localized_string(duration, 3) + " s");

    const auto fpath = std::filesystem::path{path.raw()};

    label_file_name->set_text(fpath.stem().c_str());

    if (!show_fft->get_active()) {
      plot_waveform();
    }

    return false;
  }));
}

void ConvolverUi::get_irs_spectrum(const int& rate) {
  if (left_mag.empty() || right_mag.empty()) {
    return;
  }

  util::debug(log_tag + "calculating the impulse fft...");

  left_spectrum.resize(left_mag.size() / 2U + 1U);
  right_spectrum.resize(right_mag.size() / 2U + 1U);

  auto real_input = left_mag;

  for (uint n = 0U; n < real_input.size(); n++) {
    // https://en.wikipedia.org/wiki/Hann_function

    const float w = 0.5F * (1.0F - std::cos(2.0F * std::numbers::pi_v<float> * static_cast<float>(n) /
                                            static_cast<float>(real_input.size() - 1U)));

    real_input[n] *= w;
  }

  auto* complex_output = fftwf_alloc_complex(real_input.size());

  auto* plan =
      fftwf_plan_dft_r2c_1d(static_cast<int>(real_input.size()), real_input.data(), complex_output, FFTW_ESTIMATE);

  fftwf_execute(plan);

  for (uint i = 0U; i < left_spectrum.size(); i++) {
    float sqr = complex_output[i][0] * complex_output[i][0] + complex_output[i][1] * complex_output[i][1];

    sqr /= static_cast<float>(left_spectrum.size() * left_spectrum.size());

    left_spectrum[i] = sqr;
  }

  // right channel fft

  real_input = right_mag;

  for (uint n = 0U; n < real_input.size(); n++) {
    // https://en.wikipedia.org/wiki/Hann_function

    const float w = 0.5F * (1.0F - std::cos(2.0F * std::numbers::pi_v<float> * static_cast<float>(n) /
                                            static_cast<float>(real_input.size() - 1U)));

    real_input[n] *= w;
  }

  fftwf_execute(plan);

  for (uint i = 0U; i < right_spectrum.size(); i++) {
    float sqr = complex_output[i][0] * complex_output[i][0] + complex_output[i][1] * complex_output[i][1];

    sqr /= static_cast<float>(right_spectrum.size() * right_spectrum.size());

    right_spectrum[i] = sqr;
  }

  // cleaning

  if (complex_output != nullptr) {
    fftwf_free(complex_output);
  }

  fftwf_destroy_plan(plan);

  // initializing the frequency axis

  freq_axis.resize(left_spectrum.size());

  for (uint n = 0U; n < left_spectrum.size(); n++) {
    freq_axis[n] = 0.5F * static_cast<float>(rate) * static_cast<float>(n) / static_cast<float>(left_spectrum.size());
  }

  size_t bin_size = (drawing_area->get_width() > 0) ? drawing_area->get_width() : 100;

  // initializing the logarithmic frequency axis

  const auto log_axis = util::logspace(20.0F, 22000.0F, bin_size);
  // auto log_axis = util::linspace(20.0F, 22000.0F, spectrum_settings->get_int("n-points"));

  std::vector<float> l(log_axis.size());
  std::vector<float> r(log_axis.size());
  std::vector<uint> bin_count(log_axis.size());

  std::ranges::fill(l, 0.0F);
  std::ranges::fill(r, 0.0F);
  std::ranges::fill(bin_count, 0U);

  // reducing the amount of data we have to plot and converting the frequency axis to the logarithimic scale

  for (size_t j = 0U; j < freq_axis.size(); j++) {
    for (size_t n = 0U; n < log_axis.size(); n++) {
      if (n > 0U) {
        if (freq_axis[j] <= log_axis[n] && freq_axis[j] > log_axis[n - 1U]) {
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

  for (size_t n = 0U; n < bin_count.size(); n++) {
    if (bin_count[n] == 0U && n > 0U) {
      l[n] = l[n - 1U];
      r[n] = r[n - 1U];
    }
  }

  freq_axis = log_axis;
  left_spectrum = l;
  right_spectrum = r;

  // find min and max values

  const auto fft_min_left = std::ranges::min(left_spectrum);
  const auto fft_max_left = std::ranges::max(left_spectrum);

  const auto fft_min_right = std::ranges::min(right_spectrum);
  const auto fft_max_right = std::ranges::max(right_spectrum);

  // rescaling between 0 and 1

  for (uint n = 0; n < left_spectrum.size(); n++) {
    left_spectrum[n] = (left_spectrum[n] - fft_min_left) / (fft_max_left - fft_min_left);
    right_spectrum[n] = (right_spectrum[n] - fft_min_right) / (fft_max_right - fft_min_right);
  }

  connections.push_back(Glib::signal_idle().connect([=, this]() {
    if (show_fft->get_active()) {
      plot_fft();
    }

    return false;
  }));
}

void ConvolverUi::plot_waveform() {
  if (time_axis.empty() || left_spectrum.empty() || right_spectrum.empty()) {
    return;
  }

  plot->set_plot_type(PlotType::line);

  plot->set_plot_scale(PlotScale::linear);

  plot->set_fill_bars(false);

  plot->set_line_width(static_cast<float>(spectrum_settings->get_double("line-width")));

  plot->set_x_unit("s");
  plot->set_n_x_decimals(2);
  plot->set_n_y_decimals(2);

  if (check_left->get_active()) {
    plot->set_data(time_axis, left_mag);
  } else if (check_right->get_active()) {
    plot->set_data(time_axis, right_mag);
  }
}

void ConvolverUi::plot_fft() {
  if (freq_axis.empty() || left_spectrum.empty() || right_spectrum.empty()) {
    return;
  }

  plot->set_plot_type(PlotType::line);

  plot->set_plot_scale(PlotScale::logarithmic);

  plot->set_fill_bars(false);

  plot->set_line_width(static_cast<float>(spectrum_settings->get_double("line-width")));

  plot->set_x_unit("Hz");
  plot->set_n_x_decimals(0);
  plot->set_n_y_decimals(2);

  if (check_left->get_active()) {
    plot->set_data(freq_axis, left_spectrum);
  } else if (check_right->get_active()) {
    plot->set_data(freq_axis, right_spectrum);
  }
}

auto ConvolverUi::read_kernel(const std::string& file_name) -> std::tuple<int, std::vector<float>, std::vector<float>> {
  int rate = 0;
  std::vector<float> buffer;
  std::vector<float> kernel_L;
  std::vector<float> kernel_R;

  auto file_path = irs_dir / std::filesystem::path{file_name};

  util::debug(log_tag + "reading the impulse file: " + file_path.string());

  if (file_path.extension() != irs_ext) {
    file_path += irs_ext;
  }

  if (!std::filesystem::exists(file_path)) {
    util::debug(log_tag + "file: " + file_path.string() + " does not exist");

    return std::make_tuple(rate, kernel_L, kernel_R);
  }

  auto sndfile = SndfileHandle(file_path.string());

  if (sndfile.channels() != 2 || sndfile.frames() == 0) {
    util::warning(log_tag + name + " Only stereo impulse responses are supported.");
    util::warning(log_tag + name + " The impulse file was not loaded!");

    return std::make_tuple(rate, kernel_L, kernel_R);
  }

  buffer.resize(sndfile.frames() * sndfile.channels());
  kernel_L.resize(sndfile.frames());
  kernel_R.resize(sndfile.frames());

  sndfile.readf(buffer.data(), sndfile.frames());

  for (size_t n = 0U; n < kernel_L.size(); n++) {
    kernel_L[n] = buffer[2U * n];
    kernel_R[n] = buffer[2U * n + 1U];
  }

  rate = sndfile.samplerate();

  return std::make_tuple(rate, kernel_L, kernel_R);
}

void ConvolverUi::combine_kernels(const std::string& kernel_1_name,
                                  const std::string& kernel_2_name,
                                  const std::string& output_file_name) {
  if (output_file_name.empty()) {
    // The method combine_kernels run in a secondary thread. But the widgets have to be used in the main thread.
    Glib::signal_idle().connect_once([=, this] { spinner_combine_kernel->stop(); });

    return;
  }

  auto [rate1, kernel_1_L, kernel_1_R] = read_kernel(kernel_1_name);
  auto [rate2, kernel_2_L, kernel_2_R] = read_kernel(kernel_2_name);

  if (rate1 == 0 || rate2 == 0) {
    Glib::signal_idle().connect_once([=, this] { spinner_combine_kernel->stop(); });

    return;
  }

  if (rate1 > rate2) {
    util::debug(log_tag + name + " resampling the kernel " + kernel_2_name + " to " + std::to_string(rate1) + " Hz");

    auto resampler = std::make_unique<Resampler>(rate2, rate1);

    kernel_2_L = resampler->process(kernel_2_L, true);

    resampler = std::make_unique<Resampler>(rate2, rate1);

    kernel_2_R = resampler->process(kernel_2_R, true);
  } else if (rate2 > rate1) {
    util::debug(log_tag + name + " resampling the kernel " + kernel_1_name + " to " + std::to_string(rate2) + " Hz");

    auto resampler = std::make_unique<Resampler>(rate1, rate2);

    kernel_1_L = resampler->process(kernel_1_L, true);

    resampler = std::make_unique<Resampler>(rate1, rate2);

    kernel_1_R = resampler->process(kernel_1_R, true);
  }

  std::vector<float> kernel_L(kernel_1_L.size() + kernel_2_L.size() - 1);
  std::vector<float> kernel_R(kernel_1_R.size() + kernel_2_R.size() - 1);

  // As the convolution is commutative we change the order based on which will run faster

  if (kernel_1_L.size() > kernel_2_L.size()) {
    direct_conv(kernel_1_L, kernel_2_L, kernel_L);
    direct_conv(kernel_1_R, kernel_2_R, kernel_R);
  } else {
    direct_conv(kernel_2_L, kernel_1_L, kernel_L);
    direct_conv(kernel_2_R, kernel_1_R, kernel_R);
  }

  std::vector<float> buffer(kernel_L.size() * 2);  // 2 channels interleaved

  for (size_t n = 0; n < kernel_L.size(); n++) {
    buffer[2 * n] = kernel_L[n];
    buffer[2 * n + 1] = kernel_R[n];
  }

  const auto output_file_path = irs_dir / std::filesystem::path{output_file_name + irs_ext};

  auto mode = SFM_WRITE;
  auto format = SF_FORMAT_WAV | SF_FORMAT_PCM_32;
  auto n_channels = 2;
  auto rate = (rate1 > rate2) ? rate1 : rate2;

  auto sndfile = SndfileHandle(output_file_path.string(), mode, format, n_channels, rate);

  sndfile.writef(buffer.data(), static_cast<sf_count_t>(kernel_L.size()));

  util::debug(log_tag + name + " combined kernel saved: " + output_file_path.string());

  Glib::signal_idle().connect_once([=, this] { spinner_combine_kernel->stop(); });
}

void ConvolverUi::direct_conv(const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& c) {
  std::vector<size_t> indices(c.size());

  std::iota(indices.begin(), indices.end(), 0);

  std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](size_t n) {
    c[n] = 0.0F;

    for (uint m = 0U; m < b.size(); m++) {
      if (n - m >= 0U && n - m < a.size() - 1U) {
        c[n] += b[m] * a[n - m];
      }
    }
  });
}
