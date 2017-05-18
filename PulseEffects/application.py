# -*- coding: utf-8 -*-

import configparser
import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GLib, Gtk
from PulseEffects.gst import GstEffects
from PulseEffects.pulse_manager import PulseManager
from PulseEffects.setup_compressor import SetupCompressor
from PulseEffects.setup_equalizer import SetupEqualizer
from PulseEffects.setup_limiter import SetupLimiter
from PulseEffects.setup_reverb import SetupReverb
from PulseEffects.spectrum import Spectrum
from PulseEffects.test_signal import TestSignal


class Application(Gtk.Application):

    def __init__(self):
        app_id = 'com.github.wwmm.pulseeffects'

        Gtk.Application.__init__(self, application_id=app_id)

        self.ui_initialized = False
        self.generating_test_signal = False
        self.module_path = os.path.dirname(__file__)

        self.settings = Gio.Settings('com.github.wwmm.pulseeffects')

        # pulseaudio

        self.pm = PulseManager()

        self.pm.connect('sink_inputs_changed', self.on_sink_inputs_changed)

        # gstreamer audio effects

        self.gst = GstEffects(self.pm.default_sink_rate)

        self.gst.set_output_sink_name(self.pm.default_sink_name)

        # creating user presets folder
        self.user_config_dir = os.path.expanduser('~/.config/PulseEffects')
        os.makedirs(self.user_config_dir, exist_ok=True)

    def do_startup(self):
        Gtk.Application.do_startup(self)

        self.builder = Gtk.Builder()

        ui_handlers = {
            'on_MainWindow_delete_event': self.on_MainWindow_delete_event,
            'on_buffer_time_value_changed': self.on_buffer_time_value_changed,
            'on_save_user_preset_clicked': self.on_save_user_preset_clicked,
            'on_load_user_preset_clicked': self.on_load_user_preset_clicked
        }

        self.builder.add_from_file(self.module_path + '/ui/main_ui.glade')
        self.builder.add_from_file(self.module_path + '/ui/headerbar.glade')

        headerbar = self.builder.get_object('headerbar')

        self.window = self.builder.get_object('MainWindow')
        self.window.set_titlebar(headerbar)
        self.window.set_application(self)

        self.create_appmenu()

        self.setup_limiter = SetupLimiter(self)
        self.setup_compressor = SetupCompressor(self)
        self.setup_reverb = SetupReverb(self)
        self.setup_equalizer = SetupEqualizer(self)
        self.test_signal = TestSignal(self)
        self.spectrum = Spectrum(self)

        ui_handlers.update(self.setup_limiter.handlers)
        ui_handlers.update(self.setup_compressor.handlers)
        ui_handlers.update(self.setup_reverb.handlers)
        ui_handlers.update(self.setup_equalizer.handlers)
        ui_handlers.update(self.test_signal.handlers)
        ui_handlers.update(self.spectrum.handlers)

        self.builder.connect_signals(ui_handlers)

        self.setup_limiter.init()
        self.setup_compressor.init()
        self.setup_reverb.init()
        self.setup_equalizer.init()
        self.test_signal.init()

        self.init_settings_menu()

        self.apps_box = self.builder.get_object('apps_box')

        self.build_apps_list()

        # buffer-time
        buffer_time_obj = self.builder.get_object('buffer_time')

        buffer_time = self.settings.get_value('buffer-time').unpack()

        buffer_time_obj.set_value(buffer_time)

        # now that all elements were initialized we set pipeline to ready
        self.gst.set_state('ready')

    def do_activate(self):
        self.window.present()

        self.ui_initialized = True

    def on_MainWindow_delete_event(self, event, data):
        self.gst.set_state('null')

        self.pm.unload_sink()

        self.quit()

    def create_appmenu(self):
        menu = Gio.Menu()

        menu.append('About', 'app.about')
        menu.append('Quit', 'app.quit')

        self.set_app_menu(menu)

        about_action = Gio.SimpleAction.new('about', None)
        about_action.connect('activate', self.onAbout)
        self.add_action(about_action)

        quit_action = Gio.SimpleAction.new('quit', None)
        quit_action.connect('activate', self.on_MainWindow_delete_event)
        self.add_action(quit_action)

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

    def on_buffer_time_value_changed(self, obj):
        value = obj.get_value()

        out = GLib.Variant('i', value)
        self.settings.set_value('buffer-time', out)

        if self.ui_initialized:
            self.gst.set_buffer_time(value * 1000)
        else:
            self.gst.init_buffer_time(value * 1000)

    def build_apps_list(self):
        children = self.apps_box.get_children()

        for child in children:
            self.apps_box.remove(child)

        sink_inputs = self.pm.sink_inputs

        for i in sink_inputs:
            idx = i[0]
            app_name = i[1]
            media_name = i[2]
            icon_name = i[3]
            connected = i[4]

            row = Gtk.ListBoxRow()

            hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=2)

            row.add(hbox)

            # app icon
            icon_size = Gtk.IconSize.BUTTON
            icon = Gtk.Image.new_from_icon_name(icon_name, icon_size)

            hbox.pack_start(icon, False, False, 0)

            # label
            label_text = '<b>' + app_name + '</b>' + ': ' + media_name

            label = Gtk.Label(label_text, xalign=0)
            label.set_use_markup(True)

            hbox.pack_start(label, True, True, 0)

            # switch
            switch = Gtk.Switch()

            switch.set_active(connected)
            switch.set_name('switch_' + str(idx))

            def move_sink_input(obj, state):
                idx = int(obj.get_name().split('_')[1])

                if state:
                    self.pm.grab_input(idx)
                else:
                    self.pm.move_input_to_default_sink(idx)

            switch.connect('state-set', move_sink_input)

            hbox.pack_end(switch, False, False, 0)

            self.apps_box.add(row)

        self.apps_box.show_all()

    def on_sink_inputs_changed(self, obj):
        if self.ui_initialized:
            self.build_apps_list()

        if len(self.pm.sink_inputs) > 0 or self.generating_test_signal:
            self.gst.set_state('playing')
        else:
            self.gst.set_state('paused')

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

            output = open(path, 'w')

            config = configparser.ConfigParser()

            limiter = self.settings.get_value('limiter-user')

            config['limiter'] = {'input gain': str(limiter[0]),
                                 'limit': str(limiter[1]),
                                 'release time': str(limiter[2])}

            compressor = self.settings.get_value('compressor-user')

            config['compressor'] = {'rms-peak': str(compressor[0]),
                                    'attack': str(compressor[1]),
                                    'release': str(compressor[2]),
                                    'threshold': str(compressor[3]),
                                    'ratio': str(compressor[4]),
                                    'knee': str(compressor[5]),
                                    'makeup': str(compressor[6])}

            reverb = self.settings.get_value('reverb-user')

            config['reverb'] = {'room size': str(reverb[0]),
                                'damping': str(reverb[1]),
                                'width': str(reverb[2]),
                                'level': str(reverb[3])}

            equalizer_preamp = self.settings.get_value('equalizer-preamp')
            equalizer = self.settings.get_value('equalizer-user')

            config['equalizer'] = {'preamp': str(equalizer_preamp),
                                   'band0': str(equalizer[0]),
                                   'band1': str(equalizer[1]),
                                   'band2': str(equalizer[2]),
                                   'band3': str(equalizer[3]),
                                   'band4': str(equalizer[4]),
                                   'band5': str(equalizer[5]),
                                   'band6': str(equalizer[6]),
                                   'band7': str(equalizer[7]),
                                   'band8': str(equalizer[8]),
                                   'band9': str(equalizer[9])}

            config.write(output)

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

            config = configparser.ConfigParser()

            config.read(path)

            limiter = dict(config['limiter']).values()
            limiter = [float(v) for v in limiter]
            self.setup_limiter.apply_limiter_preset(limiter)

            compressor = dict(config['compressor']).values()
            compressor = [float(v) for v in compressor]
            self.setup_compressor.apply_compressor_preset(compressor)

            reverb = dict(config['reverb']).values()
            reverb = [float(v) for v in reverb]
            self.setup_reverb.apply_reverb_preset(reverb)

            equalizer = list(dict(config['equalizer']).values())

            if len(equalizer) == 11:  # one day this check will be removed...
                equalizer_preamp = float(equalizer.pop(0))
                self.setup_equalizer.eq_preamp.set_value(equalizer_preamp)

            equalizer = [float(v) for v in equalizer]
            self.setup_equalizer.apply_eq_preset(equalizer)

        dialog.destroy()

    def onAbout(self, action, parameter):
        builder = Gtk.Builder()

        builder.add_from_file(self.module_path + '/ui/about.glade')

        dialog = builder.get_object('about_dialog')

        dialog.set_transient_for(self.window)

        dialog.show()
