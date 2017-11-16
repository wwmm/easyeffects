# -*- coding: utf-8 -*-

import logging
import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GLib, Gtk
from PulseEffects.draw_spectrum import DrawSpectrum
from PulseEffects.presets_manager import PresetsManager
from PulseEffects.pulse_manager import PulseManager
from PulseEffects.sink_input_effects import SinkInputEffects
from PulseEffects.source_output_effects import SourceOutputEffects


class Application(Gtk.Application):

    def __init__(self):
        app_id = 'com.github.wwmm.pulseeffects'
        app_flags = Gio.ApplicationFlags.HANDLES_COMMAND_LINE

        GLib.set_application_name('PulseEffects')
        GLib.setenv('PULSE_PROP_media.role', 'production', True)
        GLib.setenv('PULSE_PROP_application.icon_name', 'pulseeffects', True)

        Gtk.Application.__init__(self, application_id=app_id, flags=app_flags)

        GLib.unix_signal_add(GLib.PRIORITY_DEFAULT, 2, self.quit)  # sigint

        self.add_main_option('no-window', ord('n'), GLib.OptionFlags.NONE,
                             GLib.OptionArg.NONE, 'do not show window', None)
        self.add_main_option('switch-on-all-apps', ord('s'),
                             GLib.OptionFlags.NONE, GLib.OptionArg.NONE,
                             'Force effects for all applications', None)

    def do_startup(self):
        Gtk.Application.do_startup(self)

        self.ui_initialized = False
        self.module_path = os.path.dirname(__file__)

        log_format = '%(asctime)s.%(msecs)d - %(name)s - %(levelname)s'
        log_format = log_format + ' - %(message)s'

        logging.basicConfig(format=log_format,
                            datefmt='%H:%M:%S',
                            level=logging.INFO)

        self.log = logging.getLogger('PulseEffects')

        self.gtk_settings = Gtk.Settings.get_default()

        self.settings = Gio.Settings('com.github.wwmm.pulseeffects')

        # creating user presets folder
        self.user_config_dir = os.path.join(GLib.get_user_config_dir(),
                                            'PulseEffects')
        os.makedirs(self.user_config_dir, exist_ok=True)

        self.create_appmenu()

        # pulseaudio

        self.pm = PulseManager()
        self.pm.load_apps_sink()
        self.pm.load_mic_sink()

        self.sie = SinkInputEffects(self.pm)
        self.soe = SourceOutputEffects(self.pm)

        self.hold()

        # if self.props.flags & Gio.ApplicationFlags.IS_SERVICE:
        #     self.hold()

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/main_ui.glade')

        self.builder.connect_signals(self)

        self.window = self.builder.get_object('MainWindow')
        self.window.set_application(self)

        self.sie.init_ui()
        self.soe.init_ui()

        self.draw_spectrum = DrawSpectrum(self)

        self.init_theme()
        self.init_settings_menu()
        self.init_buffer_time()
        self.init_latency_time()
        self.init_spectrum_widgets()
        self.init_stack_widgets()

        # this connection is changed inside the stack switch handler
        # depending on the selected child. The connection below is not
        # permanent but just a default

        self.spectrum_handler_id = self.sie.connect('new_spectrum',
                                                    self.draw_spectrum
                                                    .on_new_spectrum)

        self.presets = PresetsManager(self)

    def do_activate(self):
        if not self.ui_initialized:
            self.init_ui()

            self.pm.find_sink_inputs()
            self.pm.find_source_outputs()

            def on_window_destroy(window):
                self.ui_initialized = False

            self.window.connect('destroy', on_window_destroy)

        self.window.present()

        self.ui_initialized = True

    def do_command_line(self, command_line):
        options = command_line.get_options_dict()

        if options.contains('switch-on-all-apps'):
            self.sie.switch_on_all_apps = True
            self.soe.switch_on_all_apps = True

        if options.contains('no-window'):
            self.log.info('Running in background')
        else:
            self.activate()

        # searching for apps

        # self.pm.find_sink_inputs()
        # self.pm.find_source_outputs()

        return Gtk.Application.do_command_line(self, command_line)

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
        self.stack = self.builder.get_object('stack')

        self.stack.add_named(self.sie.ui_window, 'sink_inputs')
        self.stack.child_set_property(self.sie.ui_window, 'icon-name',
                                      'audio-speakers-symbolic')

        self.stack.add_named(self.soe.ui_window, "source_outputs")
        self.stack.child_set_property(self.soe.ui_window, 'icon-name',
                                      'audio-input-microphone-symbolic')

        self.stack_current_child_name = 'sink_inputs'

        def on_visible_child_changed(stack, visible_child):
            name = stack.get_visible_child_name()

            if name == 'sink_inputs':
                if self.stack_current_child_name == 'source_outputs':
                    self.soe.disconnect(self.spectrum_handler_id)

                self.spectrum_handler_id = self.sie.connect('new_spectrum',
                                                            self.draw_spectrum
                                                            .on_new_spectrum)

                self.stack_current_child_name = 'sink_inputs'
            elif name == 'source_outputs':
                if self.stack_current_child_name == 'sink_inputs':
                    self.sie.disconnect(self.spectrum_handler_id)

                self.spectrum_handler_id = self.soe.connect('new_spectrum',
                                                            self.draw_spectrum
                                                            .on_new_spectrum)

                self.stack_current_child_name = 'source_outputs'

            self.draw_spectrum.clear()

        self.stack.connect("notify::visible-child", on_visible_child_changed)

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

    def on_latency_time_value_changed(self, obj):
        value = obj.get_value()

        out = GLib.Variant('i', value)
        self.settings.set_value('latency-time', out)

        if self.ui_initialized:
            self.sie.set_latency_time(value * 1000)
        else:
            self.sie.init_latency_time(value * 1000)

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
            self.draw_spectrum.show()
        else:
            self.draw_spectrum.hide()

    def on_show_spectrum_state_set(self, obj, state):
        if state:
            self.draw_spectrum.show()
            self.sie.enable_spectrum(True)
            self.soe.enable_spectrum(True)
        else:
            self.sie.enable_spectrum(False)
            self.soe.enable_spectrum(False)
            self.draw_spectrum.hide()

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

    def onAbout(self, action, parameter):
        builder = Gtk.Builder()

        builder.add_from_file(self.module_path + '/ui/about.glade')

        dialog = builder.get_object('about_dialog')

        dialog.set_transient_for(self.window)

        dialog.run()

        dialog.destroy()
