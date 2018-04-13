# -*- coding: utf-8 -*-

import logging
import os
from gettext import gettext as _

import gi
gi.require_version('Gdk', '3.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gdk, Gio, GLib, Gtk
from PulseEffects.draw_spectrum import DrawSpectrum
from PulseEffects.presets_manager import PresetsManager
from PulseEffects.pulse_manager import PulseManager
from PulseEffects.sink_input_effects import SinkInputEffects
from PulseEffects.source_output_effects import SourceOutputEffects
from PulseEffectsTest.application import Application as Test


class Application(Gtk.Application):

    def __init__(self):
        app_id = 'com.github.wwmm.pulseeffects'
        app_flags = Gio.ApplicationFlags.HANDLES_COMMAND_LINE

        GLib.set_application_name('PulseEffects')
        GLib.setenv('PULSE_PROP_application.icon_name', 'pulseeffects', True)
        GLib.setenv('PULSE_PROP_application.id',
                    'com.github.wwmm.pulseeffects', True)
        GLib.setenv('PULSE_PROP_application.version', '3.2.2', True)
        GLib.unix_signal_add(GLib.PRIORITY_DEFAULT, 2, self.quit)  # sigint

        Gtk.Application.__init__(self, application_id=app_id, flags=app_flags)

        help_msg = _('Quit PulseEffects. Useful when running in service mode.')

        self.add_main_option('quit', ord('q'), GLib.OptionFlags.NONE,
                             GLib.OptionArg.NONE, help_msg, None)

        help_msg = _('Show available presets.')

        self.add_main_option('presets', ord('p'), GLib.OptionFlags.NONE,
                             GLib.OptionArg.NONE, help_msg, None)

        help_msg = _('Load a preset. Example: pulseeffects -l music')

        self.add_main_option('load-preset', ord('l'), GLib.OptionFlags.NONE,
                             GLib.OptionArg.STRING, help_msg, None)

        log_format = '%(asctime)s.%(msecs)d - %(name)s - %(levelname)s'
        log_format = log_format + ' - %(message)s'

        logging.basicConfig(format=log_format, datefmt='%H:%M:%S',
                            level=logging.INFO)

        self.log = logging.getLogger('PulseEffects')

        self.log_tag = 'MAIN - '

        if os.environ.get('PULSEEFFECTS_DEBUG'):
            self.log.setLevel(logging.DEBUG)

            self.log.debug(self.log_tag + 'debug logging enabled')

    def do_startup(self):
        Gtk.Application.do_startup(self)

        self.ui_initialized = False
        self.window_activated = False
        self.running_as_service = False
        self.module_path = os.path.dirname(__file__)

        self.sink_list = []
        self.source_list = []

        self.gtk_settings = Gtk.Settings.get_default()

        self.settings = Gio.Settings('com.github.wwmm.pulseeffects')

        # creating user presets folder
        self.user_config_dir = os.path.join(GLib.get_user_config_dir(),
                                            'PulseEffects')

        os.makedirs(self.user_config_dir, exist_ok=True)

        # autostart file path
        autostart_file_name = 'autostart/pulseeffects-service.desktop'

        self.autostart_file = os.path.join(GLib.get_user_config_dir(),
                                           autostart_file_name)

        self.create_appmenu()

        # custom css styles

        self.apply_css_style('listbox.css')

        icon_theme = Gtk.IconTheme.get_default()
        icon_theme.append_search_path(self.module_path + '/ui')

        # pulseaudio

        self.pm = PulseManager()
        self.pm.load_apps_sink()
        self.pm.load_mic_sink()
        self.pm.connect('new_default_sink', self.on_new_default_sink)
        self.pm.connect('new_default_source', self.on_new_default_source)
        self.pm.connect('sink_added', self.on_sink_added)
        self.pm.connect('sink_removed', self.on_sink_removed)
        self.pm.connect('source_added', self.on_source_added)
        self.pm.connect('source_removed', self.on_source_removed)

        flag = Gio.SettingsBindFlags.DEFAULT

        self.settings.bind('use-default-sink', self.pm, 'use_default_sink',
                           flag)
        self.settings.bind('use-default-source', self.pm, 'use_default_source',
                           flag)

        # sink inputs and source outputs effects managers

        self.sie = SinkInputEffects(self.pm)
        self.soe = SourceOutputEffects(self.pm)

        self.settings.bind('enable-all-apps', self.sie, 'switch_on_all_apps',
                           flag)
        self.settings.bind('enable-all-apps', self.soe, 'switch_on_all_apps',
                           flag)

        self.init_buffer_time()
        self.init_latency_time()

        self.presets = PresetsManager(self)

        if self.props.flags & Gio.ApplicationFlags.IS_SERVICE:
            self.running_as_service = True

            self.init_ui()

            self.sie.post_messages(False)
            self.soe.post_messages(False)

            self.pm.find_sink_inputs()
            self.pm.find_source_outputs()

            self.log.info(self.log_tag + _('Running in background'))

            self.hold()

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/main_ui.glade')

        self.builder.connect_signals(self)

        self.window = self.builder.get_object('MainWindow')
        self.window.set_application(self)
        self.window.connect('destroy', self.on_window_destroy)

        self.sie.init_ui()
        self.soe.init_ui()

        self.draw_spectrum = DrawSpectrum(self)

        self.init_spectrum_widgets()
        self.init_stack_widgets()
        self.init_autostart_switch()

        # Gsettings bindings

        flag = Gio.SettingsBindFlags.DEFAULT
        flag_invert_boolean = Gio.SettingsBindFlags.INVERT_BOOLEAN

        enable_all_apps = self.builder.get_object('enable_all_apps')
        theme_switch = self.builder.get_object('theme_switch')
        use_default_sink = self.builder.get_object('use_default_sink')
        use_default_source = self.builder.get_object('use_default_source')
        self.ui_output_device = self.builder.get_object('output_device')
        self.ui_input_device = self.builder.get_object('input_device')
        buffer_time = self.builder.get_object('buffer_time')
        latency_time = self.builder.get_object('latency_time')

        self.settings.bind('use-dark-theme', theme_switch, 'active', flag)

        self.settings.bind('use-dark-theme', self.gtk_settings,
                           'gtk_application_prefer_dark_theme', flag)

        self.settings.bind('enable-all-apps', enable_all_apps, 'active', flag)

        self.settings.bind('use-default-sink', use_default_sink, 'active',
                           flag)

        self.settings.bind('use-default-sink', self.ui_output_device,
                           'sensitive', flag | flag_invert_boolean)

        self.settings.bind('use-default-source', use_default_source, 'active',
                           flag)

        self.settings.bind('use-default-source', self.ui_input_device,
                           'sensitive', flag | flag_invert_boolean)

        self.settings.bind('buffer-time', buffer_time, 'value', flag)
        self.settings.bind('latency-time', latency_time, 'value', flag)

        # this connection is changed inside the stack switch handler
        # depending on the selected child. The connection below is not
        # permanent but just a default

        self.spectrum_handler_id = self.sie.connect('new_spectrum',
                                                    self.draw_spectrum
                                                    .on_new_spectrum)

        self.presets.load_menu()

        self.ui_initialized = True

    def on_window_destroy(self, window):
        self.ui_initialized = False
        self.window_activated = False
        self.sie.there_is_window = False
        self.soe.there_is_window = False

        if self.running_as_service:
            self.sie.post_messages(False)
            self.soe.post_messages(False)

    def do_activate(self):
        if not self.ui_initialized:
            self.init_ui()

        self.window.present()

        self.window_activated = True
        self.sie.there_is_window = True
        self.soe.there_is_window = True

        self.pm.find_sink_inputs()
        self.pm.find_source_outputs()
        self.pm.find_sinks()
        self.pm.find_sources()

        self.sie.post_messages(True)
        self.soe.post_messages(True)

    def do_command_line(self, command_line):
        options = command_line.get_options_dict()

        if options.contains('quit'):
            self.quit()
        elif options.contains('presets'):
            self.presets.list_presets()
        elif options.contains('load-preset'):
            value = options.lookup_value('load-preset', None).unpack()

            self.presets.load_preset(value)
        else:
            self.do_activate()

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

    def init_stack_widgets(self):
        self.stack = self.builder.get_object('stack')

        self.stack.add_named(self.sie.ui_window, 'sink_inputs')
        self.stack.child_set_property(self.sie.ui_window, 'icon-name',
                                      'audio-speakers-symbolic')

        self.stack.add_named(self.soe.ui_window, 'source_outputs')
        self.stack.child_set_property(self.soe.ui_window, 'icon-name',
                                      'audio-input-microphone-symbolic')

        self.stack_current_child_name = 'sink_inputs'

        self.stack.connect('notify::visible-child',
                           self.on_stack_visible_child_changed)

    def on_stack_visible_child_changed(self, stack, visible_child):
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

    def init_buffer_time(self):
        value = self.settings.get_value('buffer-time').unpack()

        self.sie.init_buffer_time(value * 1000)

    def on_buffer_time_value_changed(self, obj):
        value = obj.get_value()

        out = GLib.Variant('i', value)
        self.settings.set_value('buffer-time', out)

        if self.window_activated:
            self.sie.set_buffer_time(value * 1000)

    def init_latency_time(self):
        value = self.settings.get_value('latency-time').unpack()

        self.sie.init_latency_time(value * 1000)

    def on_latency_time_value_changed(self, obj):
        value = obj.get_value()

        out = GLib.Variant('i', value)
        self.settings.set_value('latency-time', out)

        if self.window_activated:
            self.sie.set_latency_time(value * 1000)

    def init_autostart_switch(self):
        switch = self.builder.get_object('enable_autostart')

        if os.path.isfile(self.autostart_file):
            switch.set_state(True)
        else:
            switch.set_state(False)

    def on_enable_autostart_state_set(self, obj, state):
        if state:
            with open(self.autostart_file, 'w') as f:
                f.write('[Desktop Entry]\n')
                f.write('Name=PulseEffects\n')
                f.write('Comment=PulseEffects Service\n')
                f.write('Exec=pulseeffects --gapplication-service\n')
                f.write('Icon=pulseeffects\n')
                f.write('StartupNotify=false\n')
                f.write('Terminal=false\n')
                f.write('Type=Application\n')
                f.close()

            self.log.debug(self.log_tag +
                           'autostart_file created successfully')
        else:
            os.remove(self.autostart_file)

            self.log.debug(self.log_tag +
                           'autostart_file removed successfully')

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

    def on_new_default_sink(self, obj, value):
        if self.ui_initialized:
            for s in self.sink_list:
                name = s['name']

                if name == self.pm.default_sink_name:
                    idx = self.sink_list.index(s)

                    self.ui_output_device.set_active(idx)
        else:
            self.sie.set_output_sink_name(value)

    def on_new_default_source(self, obj, value):
        if self.ui_initialized:
            for s in self.source_list:
                name = s['name']

                if name == self.pm.default_source_name:
                    idx = self.source_list.index(s)

                    self.ui_input_device.set_active(idx)
        else:
            self.soe.set_source_monitor_name(value)

    def update_output_dev_combobox(self):
        active_text = self.ui_output_device.get_active_text()

        use_default = self.settings.get_value('use-default-sink').unpack()

        self.ui_output_device.remove_all()

        if use_default:
            for s in self.sink_list:
                name = s['name']

                self.ui_output_device.append_text(name)

                if name == self.pm.default_sink_name:
                    idx = self.sink_list.index(s)

                    self.ui_output_device.set_active(idx)
        else:
            active_is_set = False

            for s in self.sink_list:
                name = s['name']

                self.ui_output_device.append_text(name)

                if not active_is_set:
                    if name == active_text:
                        idx = self.sink_list.index(s)

                        self.ui_output_device.set_active(idx)

                        active_is_set = True
                    elif name == self.pm.default_sink_name:
                        idx = self.sink_list.index(s)

                        self.ui_output_device.set_active(idx)

                        active_is_set = True

    def update_input_dev_combobox(self):
        active_text = self.ui_input_device.get_active_text()

        use_default = self.settings.get_value('use-default-source').unpack()

        self.ui_input_device.remove_all()

        if use_default:
            for s in self.source_list:
                name = s['name']

                self.ui_input_device.append_text(name)

                if name == self.pm.default_source_name:
                    idx = self.source_list.index(s)

                    self.ui_input_device.set_active(idx)
        else:
            active_is_set = False

            for s in self.source_list:
                name = s['name']

                self.ui_input_device.append_text(name)

                if not active_is_set:
                    if name == active_text:
                        idx = self.source_list.index(s)

                        self.ui_input_device.set_active(idx)

                        active_is_set = True
                    elif name == self.pm.default_source_name:
                        idx = self.source_list.index(s)

                        self.ui_input_device.set_active(idx)

                        active_is_set = True

    def on_sink_added(self, obj, sink):
        add_to_list = True

        for s in self.sink_list:
            if s['idx'] == sink['idx']:
                add_to_list = False

                break

        if add_to_list:
            self.sink_list.append(sink)

            self.log.debug(self.log_tag + 'added sink: ' + sink['name'])

            if self.ui_initialized:
                self.update_output_dev_combobox()

    def on_sink_removed(self, obj, idx):
        for s in self.sink_list:
            if s['idx'] == idx:
                name = s['name']

                self.sink_list.remove(s)

                self.log.debug(self.log_tag + 'removed sink: ' + name)

                if self.ui_initialized:
                    self.update_output_dev_combobox()

                break

    def on_source_added(self, obj, source):
        add_to_list = True

        for s in self.source_list:
            if s['idx'] == source['idx']:
                add_to_list = False

                break

        if add_to_list:
            self.source_list.append(source)

            self.log.debug(self.log_tag + 'added source: ' + source['name'])

            if self.ui_initialized:
                self.update_input_dev_combobox()

    def on_source_removed(self, obj, idx):
        for s in self.source_list:
            if s['idx'] == idx:
                name = s['name']

                self.source_list.remove(s)

                self.log.debug(self.log_tag + 'removed source: ' + name)

                if self.ui_initialized:
                    self.update_input_dev_combobox()

                break

    def on_use_default_sink_state_set(self, obj, state):
        if state:
            for s in self.sink_list:
                name = s['name']

                if name == self.pm.default_sink_name:
                    idx = self.sink_list.index(s)

                    self.ui_output_device.set_active(idx)

            self.log.debug(self.log_tag + 'using default sink')

    def on_use_default_source_state_set(self, obj, state):
        if state:
            for s in self.source_list:
                name = s['name']

                if name == self.pm.default_source_name:
                    idx = self.source_list.index(s)

                    self.ui_input_device.set_active(idx)

            self.log.debug(self.log_tag + 'using default source')

    def on_output_device_changed(self, obj):
        name = obj.get_active_text()

        for s in self.sink_list:
            if s['name'] == name:
                self.sie.set_output_sink_name(name)

                self.log.debug(self.log_tag + 'output device changed: ' + name)

    def on_input_device_changed(self, obj):
        name = obj.get_active_text()

        for s in self.source_list:
            if s['name'] == name:
                self.soe.set_source_monitor_name(name)

                self.log.debug(self.log_tag + 'input device changed: ' + name)

    def apply_css_style(self, css_file):
        provider = Gtk.CssProvider()

        css_file = Gio.File.new_for_path(self.module_path + '/ui/' + css_file)

        provider.load_from_file(css_file)

        screen = Gdk.Screen.get_default()
        priority = Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION

        Gtk.StyleContext.add_provider_for_screen(screen, provider, priority)

    def on_test_clicked(self, obj):
        t = Test(self.pm)

        t.run()

    def on_reset_all_settings_clicked(self, obj):
        self.settings.reset('buffer-time')
        self.settings.reset('latency-time')
        self.settings.reset('show-spectrum')
        self.settings.reset('spectrum-n-points')
        self.settings.reset('use-dark-theme')
        self.settings.reset('enable-all-apps')

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
