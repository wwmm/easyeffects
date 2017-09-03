# -*- coding: utf-8 -*-

import logging
import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GLib, Gtk
from PulseEffects.list_sink_inputs import ListSinkInputs
from PulseEffects.list_source_outputs import ListSourceOutputs
from PulseEffects.load_presets import LoadPresets
from PulseEffects.pulse_manager import PulseManager
from PulseEffects.save_presets import SavePresets
from PulseEffects.sink_input_effects import SinkInputEffects
from PulseEffects.source_output_effects import SourceOutputEffects
from PulseEffects.spectrum import Spectrum


class Application(Gtk.Application):

    def __init__(self):
        app_id = 'com.github.wwmm.pulseeffects'

        GLib.set_application_name('PulseEffects')
        GLib.setenv('PULSE_PROP_media.role', 'production', True)
        GLib.setenv('PULSE_PROP_application.icon_name', 'pulseeffects', True)

        Gtk.Application.__init__(self, application_id=app_id)

        self.gtk_settings = Gtk.Settings.get_default()

        self.ui_initialized = False
        self.module_path = os.path.dirname(__file__)

        log_format = '%(asctime)s.%(msecs)d - %(name)s - %(levelname)s'
        log_format = log_format + ' - %(message)s'

        logging.basicConfig(format=log_format,
                            datefmt='%H:%M:%S',
                            level=logging.INFO)

        self.log = logging.getLogger('PulseEffects')

        self.settings = Gio.Settings('com.github.wwmm.pulseeffects')

        # pulseaudio

        self.pm = PulseManager()
        self.pm.load_apps_sink()
        self.pm.load_mic_sink()

        # gstreamer sink input effects

        self.sie = SinkInputEffects(self.pm.default_sink_rate)
        self.sie.set_source_monitor_name(self.pm.apps_sink_monitor_name)
        self.sie.set_output_sink_name(self.pm.default_sink_name)

        # gstreamer source outputs effects

        self.soe = SourceOutputEffects(self.pm.default_source_rate)
        self.soe.set_source_monitor_name(self.pm.default_source_name)
        self.soe.set_output_sink_name('PulseEffects_mic')

        # putting pipelines in the ready state
        self.sie.set_state('ready')
        self.soe.set_state('ready')

        # creating user presets folder
        self.user_config_dir = os.path.join(GLib.get_user_config_dir(),
                                            'PulseEffects')
        os.makedirs(self.user_config_dir, exist_ok=True)

    def do_startup(self):
        Gtk.Application.do_startup(self)

        self.builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + '/ui/main_ui.glade')
        self.builder.add_from_file(self.module_path + '/ui/headerbar.glade')

        headerbar = self.builder.get_object('headerbar')

        self.window = self.builder.get_object('MainWindow')
        self.window.set_titlebar(headerbar)
        self.window.set_application(self)

        # app menu

        self.create_appmenu()

        # main window handlers

        self.spectrum = Spectrum(self)

        main_ui_handlers = {
            'on_buffer_time_value_changed': self.on_buffer_time_value_changed,
            'on_latency_time_value_changed':
                self.on_latency_time_value_changed,
            'on_show_spectrum_state_set': self.on_show_spectrum_state_set,
            'on_spectrum_n_points_value_changed':
                self.on_spectrum_n_points_value_changed,
            'on_save_user_preset_clicked': self.on_save_user_preset_clicked,
            'on_load_user_preset_clicked': self.on_load_user_preset_clicked,
            'on_theme_switch_state_set': self.on_theme_switch_state_set,
            'on_reset_all_settings_clicked': self.on_reset_all_settings_clicked
        }

        main_ui_handlers.update(self.spectrum.handlers)

        self.builder.connect_signals(main_ui_handlers)

        # main window widgets initialization

        self.server_info_label = self.builder.get_object('server_info_label')

        self.init_theme()
        self.init_settings_menu()
        self.init_buffer_time()
        self.init_latency_time()
        self.init_spectrum_widgets()

        self.list_sink_inputs = ListSinkInputs(self.sie, self.pm)
        self.list_sink_inputs.init()

        self.list_source_outputs = ListSourceOutputs(self.soe, self.pm)
        self.list_source_outputs.init()

        self.sie.init_ui()
        self.soe.init_ui()

        self.init_stack_widgets()

        # connecting signals

        # this connection is changed inside the stack switch handler
        # depending on the selected child. The connection below is not
        # permanent but just a default
        self.spectrum_handler_id = self.sie.connect('new_spectrum',
                                                    self.spectrum
                                                    .on_new_spectrum)

        self.list_sink_inputs.connect_signals()
        self.list_source_outputs.connect_signals()

        # now that signals are connected we search for apps and add them to the
        # main window

        self.pm.find_sink_inputs()
        self.pm.find_source_outputs()

    def do_activate(self):
        self.window.present()

        self.ui_initialized = True

    def do_shutdown(self):
        Gtk.Application.do_shutdown(self)

        self.sie.set_state('null')
        self.soe.set_state('null')

        self.pm.exit()

    def create_appmenu(self):
        menu = Gio.Menu()

        menu.append('About', 'app.about')
        menu.append('Quit', 'app.quit')

        self.set_app_menu(menu)

        about_action = Gio.SimpleAction.new('about', None)
        about_action.connect('activate', self.onAbout)
        self.add_action(about_action)

        quit_action = Gio.SimpleAction.new('quit', None)
        quit_action.connect('activate', lambda action, parameter: self.quit())
        self.add_action(quit_action)

    def init_theme(self):
        switch = self.builder.get_object('theme_switch')

        use_dark = self.settings.get_value('use-dark-theme').unpack()

        switch.set_active(use_dark)

    def on_theme_switch_state_set(self, obj, state):
        self.gtk_settings.props.gtk_application_prefer_dark_theme = state

        out = GLib.Variant('b', state)
        self.settings.set_value('use-dark-theme', out)

    def init_stack_widgets(self):
        stack_switcher = self.builder.get_object('stack_switcher')
        stack_box = self.builder.get_object('stack_box')

        self.stack = Gtk.Stack()
        self.stack.set_transition_type(Gtk.StackTransitionType.CROSSFADE)
        self.stack.set_transition_duration(250)
        self.stack.set_homogeneous(False)

        self.stack.add_named(self.sie.ui_window, 'sink_inputs')

        self.stack.child_set_property(self.sie.ui_window, 'icon-name',
                                      'audio-speakers-symbolic')

        self.stack.add_named(self.soe.ui_window, "source_outputs")
        self.stack.child_set_property(self.soe.ui_window, 'icon-name',
                                      'audio-input-microphone-symbolic')

        self.stack_current_child_name = 'sink_inputs'

        server_info = str(self.pm.default_sink_format) + ', ' + \
            str(round(self.pm.default_sink_rate / 1000.0, 1)) + ' kHz'

        self.server_info_label.set_text(server_info)

        def on_visible_child_changed(stack, visible_child):
            name = stack.get_visible_child_name()

            if name == 'sink_inputs':
                if self.stack_current_child_name == 'source_outputs':
                    self.soe.disconnect(self.spectrum_handler_id)

                self.spectrum_handler_id = self.sie.connect('new_spectrum',
                                                            self.spectrum
                                                            .on_new_spectrum)

                self.stack_current_child_name = 'sink_inputs'

                server_info = str(self.pm.default_sink_format) + ', ' + \
                    str(round(self.pm.default_sink_rate / 1000.0, 1)) + ' kHz'

                self.server_info_label.set_text(server_info)
            elif name == 'source_outputs':
                if self.stack_current_child_name == 'sink_inputs':
                    self.sie.disconnect(self.spectrum_handler_id)

                self.spectrum_handler_id = self.soe.connect('new_spectrum',
                                                            self.spectrum
                                                            .on_new_spectrum)

                self.stack_current_child_name = 'source_outputs'

                server_info = str(self.pm.default_source_format) + ', ' + \
                    str(round(self.pm.default_source_rate / 1000.0, 1)) + \
                    ' kHz'

                self.server_info_label.set_text(server_info)

            self.spectrum.clear()

        self.stack.connect("notify::visible-child", on_visible_child_changed)

        stack_switcher.set_stack(self.stack)

        stack_box.pack_start(self.stack, True, True, 0)
        stack_box.show_all()

    def init_settings_menu(self):
        button = self.builder.get_object('settings_popover_button')
        menu = self.builder.get_object('settings_menu')

        popover = Gtk.Popover.new(button)
        popover.props.transitions_enabled = True
        popover.add(menu)

        def button_clicked(arg):
            if popover.get_visible():
                popover.hide()
            else:
                popover.show_all()

        button.connect("clicked", button_clicked)

    def init_buffer_time(self):
        value = self.settings.get_value('buffer-time').unpack()

        buffer_time = self.builder.get_object('buffer_time')

        buffer_time.set_value(value)

        self.sie.init_buffer_time(value * 1000)
        self.soe.init_buffer_time(value * 1000)

    def on_buffer_time_value_changed(self, obj):
        value = obj.get_value()

        out = GLib.Variant('i', value)
        self.settings.set_value('buffer-time', out)

        if self.ui_initialized:
            self.sie.set_buffer_time(value * 1000)
            self.soe.set_buffer_time(value * 1000)
        else:
            self.sie.init_buffer_time(value * 1000)
            self.soe.init_buffer_time(value * 1000)

    def init_latency_time(self):
        value = self.settings.get_value('latency-time').unpack()

        latency_time = self.builder.get_object('latency_time')

        latency_time.set_value(value)

        self.sie.init_latency_time(value * 1000)
        self.soe.init_latency_time(value * 1000)

    def on_latency_time_value_changed(self, obj):
        value = obj.get_value()

        out = GLib.Variant('i', value)
        self.settings.set_value('latency-time', out)

        if self.ui_initialized:
            self.sie.set_latency_time(value * 1000)
            self.soe.set_latency_time(value * 1000)
        else:
            self.sie.init_latency_time(value * 1000)
            self.soe.init_latency_time(value * 1000)

    def init_spectrum_widgets(self):
        show_spectrum_switch = self.builder.get_object('show_spectrum')
        spectrum_n_points_obj = self.builder.get_object('spectrum_n_points')

        show_spectrum = self.settings.get_value('show-spectrum').unpack()
        spectrum_n_points = self.settings.get_value(
            'spectrum-n-points').unpack()

        show_spectrum_switch.set_active(show_spectrum)
        spectrum_n_points_obj.set_value(spectrum_n_points)

        self.sie.set_spectrum_n_points(spectrum_n_points)
        self.soe.set_spectrum_n_points(spectrum_n_points)

        if show_spectrum:
            self.spectrum.show()
        else:
            self.spectrum.hide()

    def on_show_spectrum_state_set(self, obj, state):
        if state:
            self.spectrum.show()
            self.sie.enable_spectrum(True)
            self.soe.enable_spectrum(True)
        else:
            self.sie.enable_spectrum(False)
            self.soe.enable_spectrum(False)
            self.spectrum.hide()

        out = GLib.Variant('b', state)
        self.settings.set_value('show-spectrum', out)

    def on_spectrum_n_points_value_changed(self, obj):
        value = obj.get_value()

        out = GLib.Variant('i', value)
        self.settings.set_value('spectrum-n-points', out)

        self.sie.set_spectrum_n_points(value)
        self.soe.set_spectrum_n_points(value)

    def on_reset_all_settings_clicked(self, obj):
        self.settings.reset('buffer-time')
        self.settings.reset('latency-time')
        self.settings.reset('show-spectrum')
        self.settings.reset('spectrum-n-points')
        self.settings.reset('use-dark-theme')

        self.init_theme()
        self.init_buffer_time()
        self.init_latency_time()
        self.init_spectrum_widgets()

        self.stack.set_visible_child(self.sie.ui_window)

        self.sie.reset()
        self.soe.reset()

    def add_file_filter(self, dialog):
        file_filter = Gtk.FileFilter()
        file_filter.set_name("preset")
        file_filter.add_mime_type("text/plain")

        dialog.add_filter(file_filter)

    def on_save_user_preset_clicked(self, obj):
        dialog = Gtk.FileChooserDialog('', self.window,
                                       Gtk.FileChooserAction.SAVE,
                                       (Gtk.STOCK_CANCEL,
                                        Gtk.ResponseType.CANCEL,
                                        Gtk.STOCK_SAVE, Gtk.ResponseType.OK))

        dialog.set_current_folder(self.user_config_dir)
        dialog.set_current_name('user.preset')

        self.add_file_filter(dialog)

        response = dialog.run()

        if response == Gtk.ResponseType.OK:
            path = dialog.get_filename()

            if not path.endswith(".preset"):
                path += ".preset"

            s = SavePresets(path)

            s.save_sink_inputs_presets(self.sie.settings)
            s.save_source_outputs_presets(self.soe.settings)

            s.write_config()

        dialog.destroy()

    def on_load_user_preset_clicked(self, obj):
        dialog = Gtk.FileChooserDialog('', self.window,
                                       Gtk.FileChooserAction.OPEN,
                                       (Gtk.STOCK_CANCEL,
                                        Gtk.ResponseType.CANCEL,
                                        Gtk.STOCK_OPEN, Gtk.ResponseType.OK))

        dialog.set_current_folder(self.user_config_dir)

        self.add_file_filter(dialog)

        response = dialog.run()

        if response == Gtk.ResponseType.OK:
            path = dialog.get_filename()

            l = LoadPresets(path)

            l.load_sink_inputs_presets(self.sie.settings)
            l.load_source_outputs_presets(self.soe.settings)

            self.sie.init_ui()
            self.soe.init_ui()

        dialog.destroy()

    def onAbout(self, action, parameter):
        builder = Gtk.Builder()

        builder.add_from_file(self.module_path + '/ui/about.glade')

        dialog = builder.get_object('about_dialog')

        dialog.set_transient_for(self.window)

        dialog.run()

        dialog.destroy()
