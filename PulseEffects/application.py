# -*- coding: utf-8 -*-

import configparser
import logging
import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GLib, Gtk
from PulseEffects.list_apps import ListApps
from PulseEffects.pulse_manager import PulseManager
from PulseEffects.setup_compressor import SetupCompressor
from PulseEffects.setup_equalizer import SetupEqualizer
from PulseEffects.setup_limiter import SetupLimiter
from PulseEffects.setup_reverb import SetupReverb
from PulseEffects.sink_input_effects import SinkInputEffects
from PulseEffects.spectrum import Spectrum
from PulseEffects.test_signal import TestSignal


class Application(Gtk.Application):

    def __init__(self):
        app_id = 'com.github.wwmm.pulseeffects'

        GLib.set_application_name('PulseEffects')
        GLib.setenv('PULSE_PROP_media.role', 'production', True)
        GLib.setenv('PULSE_PROP_application.icon_name', 'pulseeffects', True)

        Gtk.Application.__init__(self, application_id=app_id)

        self.ui_initialized = False
        self.generating_test_signal = False
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

        # gstreamer audio effects

        self.sie = SinkInputEffects(self.pm.default_sink_rate)

        self.sie.set_source_monitor_name(self.pm.apps_sink_monitor_name)
        self.sie.set_output_sink_name(self.pm.default_sink_name)

        # creating user presets folder
        self.user_config_dir = os.path.expanduser('~/.config/PulseEffects')
        os.makedirs(self.user_config_dir, exist_ok=True)

    def do_startup(self):
        Gtk.Application.do_startup(self)

        self.builder = Gtk.Builder()

        ui_handlers = {
            'on_MainWindow_delete_event': self.on_MainWindow_delete_event,
            'on_buffer_time_value_changed': self.on_buffer_time_value_changed,
            'on_latency_time_value_changed':
                self.on_latency_time_value_changed,
            'on_panorama_value_changed': self.on_panorama_value_changed,
            'on_save_user_preset_clicked': self.on_save_user_preset_clicked,
            'on_load_user_preset_clicked': self.on_load_user_preset_clicked,
            'on_reset_all_settings_clicked': self.on_reset_all_settings_clicked
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
        self.list_apps = ListApps(self)

        ui_handlers.update(self.setup_limiter.handlers)
        ui_handlers.update(self.setup_compressor.handlers)
        ui_handlers.update(self.setup_reverb.handlers)
        ui_handlers.update(self.setup_equalizer.handlers)
        ui_handlers.update(self.spectrum.handlers)
        ui_handlers.update(self.list_apps.handlers)

        self.builder.connect_signals(ui_handlers)

        self.setup_limiter.init()
        self.setup_compressor.init()
        self.setup_reverb.init()
        self.setup_equalizer.init()
        self.test_signal.init()
        self.spectrum.init()
        self.list_apps.init()

        self.init_settings_menu()
        self.init_buffer_time()
        self.init_latency_time()
        self.init_panorama()

        # label for sink format and rate
        sink_rate_label = self.builder.get_object('sink_rate')
        sink_rate_label.set_text(self.pm.default_sink_format + ', ' +
                                 str(self.pm.default_sink_rate) + ' Hz')

        # now that all elements were initialized we set pipeline to ready
        self.sie.set_state('ready')

    def do_activate(self):
        self.window.present()

        self.ui_initialized = True

        self.pm.find_sink_inputs()
        self.pm.find_source_outputs()

    def on_MainWindow_delete_event(self, event, data):
        self.sie.set_state('null')

        self.pm.exit()

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

    def init_buffer_time(self):
        value = self.settings.get_value('buffer-time').unpack()

        buffer_time = self.builder.get_object('buffer_time')

        buffer_time.set_value(value)

        self.sie.set_buffer_time(value * 1000)

    def on_buffer_time_value_changed(self, obj):
        value = obj.get_value()

        out = GLib.Variant('i', value)
        self.settings.set_value('buffer-time', out)

        if self.ui_initialized:
            self.sie.set_buffer_time(value * 1000)
        else:
            self.sie.init_buffer_time(value * 1000)

    def init_latency_time(self):
        value = self.settings.get_value('latency-time').unpack()

        latency_time = self.builder.get_object('latency_time')

        latency_time.set_value(value)

        self.sie.set_latency_time(value * 1000)

    def on_latency_time_value_changed(self, obj):
        value = obj.get_value()

        out = GLib.Variant('i', value)
        self.settings.set_value('latency-time', out)

        if self.ui_initialized:
            self.sie.set_latency_time(value * 1000)
        else:
            self.sie.init_latency_time(value * 1000)

    def init_panorama(self):
        value = self.settings.get_value('panorama').unpack()

        self.panorama = self.builder.get_object('panorama')

        self.panorama.set_value(value)

        self.sie.set_panorama(value)

    def on_panorama_value_changed(self, obj):
        value = obj.get_value()

        self.sie.set_panorama(value)

        out = GLib.Variant('d', value)
        self.settings.set_value('panorama', out)

    def on_reset_all_settings_clicked(self, obj):
        self.settings.reset('buffer-time')
        self.settings.reset('latency-time')
        self.settings.reset('panorama')

        self.init_buffer_time()
        self.init_latency_time()
        self.init_panorama()

        self.spectrum.reset()
        self.setup_limiter.reset()
        self.setup_compressor.reset()
        self.setup_reverb.reset()
        self.setup_equalizer.reset()

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

            panorama = self.settings.get_value('panorama')

            config['panorama'] = {'panorama': str(panorama)}

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

            equalizer_input_gain = self.settings.get_value(
                'equalizer-input-gain')
            equalizer_output_gain = self.settings.get_value(
                'equalizer-output-gain')
            equalizer = self.settings.get_value('equalizer-user')
            equalizer_highpass_cutoff = self.settings.get_value(
                'equalizer-highpass-cutoff')
            equalizer_highpass_poles = self.settings.get_value(
                'equalizer-highpass-poles')
            equalizer_lowpass_cutoff = self.settings.get_value(
                'equalizer-lowpass-cutoff')
            equalizer_lowpass_poles = self.settings.get_value(
                'equalizer-lowpass-poles')

            config['equalizer'] = {'input_gain': str(equalizer_input_gain),
                                   'output_gain': str(equalizer_output_gain),
                                   'band0': str(equalizer[0]),
                                   'band1': str(equalizer[1]),
                                   'band2': str(equalizer[2]),
                                   'band3': str(equalizer[3]),
                                   'band4': str(equalizer[4]),
                                   'band5': str(equalizer[5]),
                                   'band6': str(equalizer[6]),
                                   'band7': str(equalizer[7]),
                                   'band8': str(equalizer[8]),
                                   'band9': str(equalizer[9]),
                                   'band10': str(equalizer[10]),
                                   'band11': str(equalizer[11]),
                                   'band12': str(equalizer[12]),
                                   'band13': str(equalizer[13]),
                                   'band14': str(equalizer[14]),
                                   'highpass_cutoff':
                                   str(equalizer_highpass_cutoff),
                                   'highpass_poles':
                                   str(equalizer_highpass_poles),
                                   'lowpass_cutoff':
                                   str(equalizer_lowpass_cutoff),
                                   'lowpass_poles':
                                   str(equalizer_lowpass_poles)}

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

            panorama_value = config.getfloat('panorama', 'panorama',
                                             fallback=0.0)

            self.panorama.set_value(panorama_value)

            compressor = dict(config['compressor']).values()
            compressor = [float(v) for v in compressor]
            self.setup_compressor.apply_compressor_preset(compressor)

            reverb = dict(config['reverb']).values()
            reverb = [float(v) for v in reverb]
            self.setup_reverb.apply_reverb_preset(reverb)

            equalizer_input_gain = config.getfloat('equalizer', 'input_gain',
                                                   fallback=0)
            equalizer_output_gain = config.getfloat('equalizer', 'output_gain',
                                                    fallback=0)

            highpass_cutoff_freq = config.getint('equalizer',
                                                 'highpass_cutoff',
                                                 fallback=20)
            highpass_poles = config.getint('equalizer',
                                           'highpass_poles',
                                           fallback=4)
            lowpass_cutoff_freq = config.getint('equalizer',
                                                'lowpass_cutoff',
                                                fallback=20000)
            lowpass_poles = config.getint('equalizer',
                                          'lowpass_poles',
                                          fallback=4)

            equalizer_band0 = config.getfloat('equalizer', 'band0')
            equalizer_band1 = config.getfloat('equalizer', 'band1')
            equalizer_band2 = config.getfloat('equalizer', 'band2')
            equalizer_band3 = config.getfloat('equalizer', 'band3')
            equalizer_band4 = config.getfloat('equalizer', 'band4')
            equalizer_band5 = config.getfloat('equalizer', 'band5')
            equalizer_band6 = config.getfloat('equalizer', 'band6')
            equalizer_band7 = config.getfloat('equalizer', 'band7')
            equalizer_band8 = config.getfloat('equalizer', 'band8')
            equalizer_band9 = config.getfloat('equalizer', 'band9')
            equalizer_band10 = config.getfloat('equalizer', 'band10')
            equalizer_band11 = config.getfloat('equalizer', 'band11')
            equalizer_band12 = config.getfloat('equalizer', 'band12')
            equalizer_band13 = config.getfloat('equalizer', 'band13')
            equalizer_band14 = config.getfloat('equalizer', 'band14')

            equalizer_bands = [equalizer_band0, equalizer_band1,
                               equalizer_band2, equalizer_band3,
                               equalizer_band4, equalizer_band5,
                               equalizer_band6, equalizer_band7,
                               equalizer_band8, equalizer_band9,
                               equalizer_band10, equalizer_band11,
                               equalizer_band12,
                               equalizer_band13, equalizer_band14]

            self.setup_equalizer.equalizer_input_gain.set_value(
                equalizer_input_gain)
            self.setup_equalizer.equalizer_output_gain.set_value(
                equalizer_output_gain)

            self.setup_equalizer.apply_eq_preset(equalizer_bands)

            self.setup_equalizer.eq_highpass_cutoff_freq.set_value(
                highpass_cutoff_freq)
            self.setup_equalizer.eq_highpass_poles.set_value(highpass_poles)

            self.setup_equalizer.eq_lowpass_cutoff_freq.set_value(
                lowpass_cutoff_freq)
            self.setup_equalizer.eq_lowpass_poles.set_value(lowpass_poles)

        dialog.destroy()

    def onAbout(self, action, parameter):
        builder = Gtk.Builder()

        builder.add_from_file(self.module_path + '/ui/about.glade')

        dialog = builder.get_object('about_dialog')

        dialog.set_transient_for(self.window)

        dialog.run()

        dialog.destroy()
