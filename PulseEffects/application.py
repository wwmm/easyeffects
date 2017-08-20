# -*- coding: utf-8 -*-

import configparser
import logging
import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GLib, Gtk
from PulseEffects.list_sink_inputs import ListSinkInputs
from PulseEffects.list_source_outputs import ListSourceOutputs
from PulseEffects.pulse_manager import PulseManager
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
            'on_MainWindow_delete_event': self.on_MainWindow_delete_event,
            'on_buffer_time_value_changed': self.on_buffer_time_value_changed,
            'on_latency_time_value_changed':
                self.on_latency_time_value_changed,
            'on_show_spectrum_state_set': self.on_show_spectrum_state_set,
            'on_spectrum_n_points_value_changed':
                self.on_spectrum_n_points_value_changed,
            'on_panorama_value_changed': self.on_panorama_value_changed,
            'on_save_user_preset_clicked': self.on_save_user_preset_clicked,
            'on_load_user_preset_clicked': self.on_load_user_preset_clicked,
            'on_theme_switch_state_set': self.on_theme_switch_state_set,
            'on_reset_all_settings_clicked': self.on_reset_all_settings_clicked
        }

        main_ui_handlers.update(self.spectrum.handlers)

        self.builder.connect_signals(main_ui_handlers)

        # main window widgets initialization

        self.init_theme()
        self.init_settings_menu()
        self.init_buffer_time()
        self.init_latency_time()
        self.init_panorama_widgets()
        self.init_spectrum_widgets()
        self.init_sink_inputs_widgets()
        self.init_source_outputs_widgets()

        self.sie.init_ui()
        self.soe.init_ui()

        # init stack widgets
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

    def on_MainWindow_delete_event(self, event, data):
        self.sie.set_state('null')
        self.soe.set_state('null')

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

        stack = Gtk.Stack()
        stack.set_transition_type(Gtk.StackTransitionType.CROSSFADE)
        stack.set_transition_duration(250)
        stack.set_homogeneous(False)

        stack.add_named(self.sie.ui_window, 'sink_inputs')

        stack.child_set_property(self.sie.ui_window, 'icon-name',
                                 'audio-speakers-symbolic')

        stack.add_named(self.soe.ui_window, "source_outputs")
        stack.child_set_property(self.soe.ui_window, 'icon-name',
                                 'audio-input-microphone-symbolic')

        self.stack_current_child_name = 'sink_inputs'

        def on_visible_child_changed(stack, visible_child):
            name = stack.get_visible_child_name()

            if name == 'sink_inputs':
                if self.stack_current_child_name == 'source_outputs':
                    self.soe.disconnect(self.spectrum_handler_id)

                self.spectrum_handler_id = self.sie.connect('new_spectrum',
                                                            self.spectrum
                                                            .on_new_spectrum)

                self.stack_current_child_name = 'sink_inputs'
            elif name == 'source_outputs':
                if self.stack_current_child_name == 'sink_inputs':
                    self.sie.disconnect(self.spectrum_handler_id)

                self.spectrum_handler_id = self.soe.connect('new_spectrum',
                                                            self.spectrum
                                                            .on_new_spectrum)

                self.stack_current_child_name = 'source_outputs'

            self.spectrum.clear()

        stack.connect("notify::visible-child", on_visible_child_changed)

        stack_switcher.set_stack(stack)

        stack_box.pack_start(stack, True, True, 0)
        stack_box.show_all()

    def init_sink_inputs_widgets(self):
        self.list_sink_inputs = ListSinkInputs(self.sie, self.pm)

        self.list_sink_inputs.init()

    def init_source_outputs_widgets(self):
        self.list_source_outputs = ListSourceOutputs(self.soe, self.pm)

        self.list_source_outputs.init()

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

    def init_panorama_widgets(self):
        value = self.sie.settings.get_value('panorama').unpack()

        self.panorama = self.builder.get_object('panorama')

        self.panorama.set_value(value)

        self.sie.set_panorama(value)

    def on_panorama_value_changed(self, obj):
        value = obj.get_value()

        self.sie.set_panorama(value)

        out = GLib.Variant('d', value)
        self.sie.settings.set_value('panorama', out)

    def on_reset_all_settings_clicked(self, obj):
        self.settings.reset('buffer-time')
        self.settings.reset('latency-time')
        self.settings.reset('autovolume-state')
        self.settings.reset('autovolume-window')
        self.settings.reset('autovolume-target')
        self.settings.reset('autovolume-tolerance')
        self.settings.reset('autovolume-threshold')
        self.settings.reset('panorama')
        self.settings.reset('show-spectrum')
        self.settings.reset('spectrum-n-points')
        self.settings.reset('use-dark-theme')

        self.init_theme()
        self.init_buffer_time()
        self.init_latency_time()
        self.init_autovolume_widgets()
        self.init_panorama_widgets()
        self.init_spectrum_widgets()

        self.sie.reset()
        self.soe.reset()

    def add_file_filter(self, dialog):
        file_filter = Gtk.FileFilter()
        file_filter.set_name("preset")
        file_filter.add_mime_type("text/plain")

        dialog.add_filter(file_filter)

    def store_sink_inputs_preset(self, config):
        limiter = self.sie.settings.get_value('limiter-user')

        config['apps_limiter'] = {'input gain': str(limiter[0]),
                                  'limit': str(limiter[1]),
                                  'release time': str(limiter[2])}

        panorama = self.sie.settings.get_value('panorama')

        config['apps_panorama'] = {'panorama': str(panorama)}

        compressor = self.sie.settings.get_value('compressor-user')

        config['apps_compressor'] = {'rms-peak': str(compressor[0]),
                                     'attack': str(compressor[1]),
                                     'release': str(compressor[2]),
                                     'threshold': str(compressor[3]),
                                     'ratio': str(compressor[4]),
                                     'knee': str(compressor[5]),
                                     'makeup': str(compressor[6])}

        reverb = self.sie.settings.get_value('reverb-user')

        config['apps_reverb'] = {'room size': str(reverb[0]),
                                 'damping': str(reverb[1]),
                                 'width': str(reverb[2]),
                                 'level': str(reverb[3])}

        highpass_cutoff = self.sie.settings.get_value('highpass-cutoff')
        highpass_poles = self.sie.settings.get_value('highpass-poles')

        config['apps_highpass'] = {'cutoff':
                                   str(highpass_cutoff),
                                   'poles':
                                   str(highpass_poles)}

        lowpass_cutoff = self.sie.settings.get_value('lowpass-cutoff')
        lowpass_poles = self.sie.settings.get_value('lowpass-poles')

        config['apps_lowpass'] = {'cutoff':
                                  str(lowpass_cutoff),
                                  'poles':
                                  str(lowpass_poles)}

        equalizer_input_gain = self.sie.settings.get_value(
            'equalizer-input-gain')
        equalizer_output_gain = self.sie.settings.get_value(
            'equalizer-output-gain')
        equalizer = self.sie.settings.get_value('equalizer-user')
        eq_freqs = self.sie.settings.get_value('equalizer-freqs')
        eq_qfactors = self.sie.settings.get_value('equalizer-qfactors')

        config['apps_equalizer'] = {'input_gain': str(equalizer_input_gain),
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
                                    'band0_freq': str(eq_freqs[0]),
                                    'band1_freq': str(eq_freqs[1]),
                                    'band2_freq': str(eq_freqs[2]),
                                    'band3_freq': str(eq_freqs[3]),
                                    'band4_freq': str(eq_freqs[4]),
                                    'band5_freq': str(eq_freqs[5]),
                                    'band6_freq': str(eq_freqs[6]),
                                    'band7_freq': str(eq_freqs[7]),
                                    'band8_freq': str(eq_freqs[8]),
                                    'band9_freq': str(eq_freqs[9]),
                                    'band10_freq': str(eq_freqs[10]),
                                    'band11_freq': str(eq_freqs[11]),
                                    'band12_freq': str(eq_freqs[12]),
                                    'band13_freq': str(eq_freqs[13]),
                                    'band14_freq': str(eq_freqs[14]),
                                    'band0_qfactor': str(eq_qfactors[0]),
                                    'band1_qfactor': str(eq_qfactors[1]),
                                    'band2_qfactor': str(eq_qfactors[2]),
                                    'band3_qfactor': str(eq_qfactors[3]),
                                    'band4_qfactor': str(eq_qfactors[4]),
                                    'band5_qfactor': str(eq_qfactors[5]),
                                    'band6_qfactor': str(eq_qfactors[6]),
                                    'band7_qfactor': str(eq_qfactors[7]),
                                    'band8_qfactor': str(eq_qfactors[8]),
                                    'band9_qfactor': str(eq_qfactors[9]),
                                    'band10_qfactor': str(eq_qfactors[10]),
                                    'band11_qfactor': str(eq_qfactors[11]),
                                    'band12_qfactor': str(eq_qfactors[12]),
                                    'band13_qfactor': str(eq_qfactors[13]),
                                    'band14_qfactor': str(eq_qfactors[14])}

    def store_source_outputs_preset(self, config):
        limiter = self.soe.settings.get_value('limiter-user')

        config['mic_limiter'] = {'input gain': str(limiter[0]),
                                 'limit': str(limiter[1]),
                                 'release time': str(limiter[2])}

        compressor = self.soe.settings.get_value('compressor-user')

        config['mic_compressor'] = {'rms-peak': str(compressor[0]),
                                    'attack': str(compressor[1]),
                                    'release': str(compressor[2]),
                                    'threshold': str(compressor[3]),
                                    'ratio': str(compressor[4]),
                                    'knee': str(compressor[5]),
                                    'makeup': str(compressor[6])}

        reverb = self.soe.settings.get_value('reverb-user')

        config['mic_reverb'] = {'room size': str(reverb[0]),
                                'damping': str(reverb[1]),
                                'width': str(reverb[2]),
                                'level': str(reverb[3])}

        highpass_cutoff = self.soe.settings.get_value('highpass-cutoff')
        highpass_poles = self.soe.settings.get_value('highpass-poles')

        config['mic_highpass'] = {'cutoff':
                                  str(highpass_cutoff),
                                  'poles':
                                  str(highpass_poles)}

        lowpass_cutoff = self.soe.settings.get_value('lowpass-cutoff')
        lowpass_poles = self.soe.settings.get_value('lowpass-poles')

        config['mic_lowpass'] = {'cutoff':
                                 str(lowpass_cutoff),
                                 'poles':
                                 str(lowpass_poles)}

        equalizer_input_gain = self.soe.settings.get_value(
            'equalizer-input-gain')
        equalizer_output_gain = self.soe.settings.get_value(
            'equalizer-output-gain')
        equalizer = self.soe.settings.get_value('equalizer-user')
        eq_freqs = self.soe.settings.get_value('equalizer-freqs')
        eq_qfactors = self.soe.settings.get_value('equalizer-qfactors')

        config['mic_equalizer'] = {'input_gain': str(equalizer_input_gain),
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
                                   'band0_freq': str(eq_freqs[0]),
                                   'band1_freq': str(eq_freqs[1]),
                                   'band2_freq': str(eq_freqs[2]),
                                   'band3_freq': str(eq_freqs[3]),
                                   'band4_freq': str(eq_freqs[4]),
                                   'band5_freq': str(eq_freqs[5]),
                                   'band6_freq': str(eq_freqs[6]),
                                   'band7_freq': str(eq_freqs[7]),
                                   'band8_freq': str(eq_freqs[8]),
                                   'band9_freq': str(eq_freqs[9]),
                                   'band10_freq': str(eq_freqs[10]),
                                   'band11_freq': str(eq_freqs[11]),
                                   'band12_freq': str(eq_freqs[12]),
                                   'band13_freq': str(eq_freqs[13]),
                                   'band14_freq': str(eq_freqs[14]),
                                   'band0_qfactor': str(eq_qfactors[0]),
                                   'band1_qfactor': str(eq_qfactors[1]),
                                   'band2_qfactor': str(eq_qfactors[2]),
                                   'band3_qfactor': str(eq_qfactors[3]),
                                   'band4_qfactor': str(eq_qfactors[4]),
                                   'band5_qfactor': str(eq_qfactors[5]),
                                   'band6_qfactor': str(eq_qfactors[6]),
                                   'band7_qfactor': str(eq_qfactors[7]),
                                   'band8_qfactor': str(eq_qfactors[8]),
                                   'band9_qfactor': str(eq_qfactors[9]),
                                   'band10_qfactor': str(eq_qfactors[10]),
                                   'band11_qfactor': str(eq_qfactors[11]),
                                   'band12_qfactor': str(eq_qfactors[12]),
                                   'band13_qfactor': str(eq_qfactors[13]),
                                   'band14_qfactor': str(eq_qfactors[14])}

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

            self.store_sink_inputs_preset(config)
            self.store_source_outputs_preset(config)

            config.write(output)

        dialog.destroy()

    def load_sink_inputs_preset(self, config):
        settings = self.sie.settings

        autovolume_state = self.settings.get_value(
            'autovolume-state').unpack()

        if autovolume_state is False:
            limiter = dict(config['apps_limiter']).values()
            limiter = [float(v) for v in limiter]

            settings.set_value('limiter-user', GLib.Variant('ad', limiter))

        panorama_value = config.getfloat('apps_panorama', 'panorama',
                                         fallback=0.0)

        self.panorama.set_value(panorama_value)

        compressor = dict(config['apps_compressor']).values()
        compressor = [float(v) for v in compressor]

        reverb = dict(config['apps_reverb']).values()
        reverb = [float(v) for v in reverb]

        highpass_cutoff = config.getint('apps_highpass',
                                        'cutoff',
                                        fallback=20)
        highpass_poles = config.getint('apps_highpass',
                                       'poles',
                                       fallback=4)
        lowpass_cutoff = config.getint('apps_lowpass',
                                       'cutoff',
                                       fallback=20000)
        lowpass_poles = config.getint('apps_lowpass',
                                      'poles',
                                      fallback=4)

        equalizer_input_gain = config.getfloat('apps_equalizer', 'input_gain',
                                               fallback=0)
        equalizer_output_gain = config.getfloat('apps_equalizer',
                                                'output_gain',
                                                fallback=0)

        equalizer_band0 = config.getfloat('apps_equalizer', 'band0')
        equalizer_band1 = config.getfloat('apps_equalizer', 'band1')
        equalizer_band2 = config.getfloat('apps_equalizer', 'band2')
        equalizer_band3 = config.getfloat('apps_equalizer', 'band3')
        equalizer_band4 = config.getfloat('apps_equalizer', 'band4')
        equalizer_band5 = config.getfloat('apps_equalizer', 'band5')
        equalizer_band6 = config.getfloat('apps_equalizer', 'band6')
        equalizer_band7 = config.getfloat('apps_equalizer', 'band7')
        equalizer_band8 = config.getfloat('apps_equalizer', 'band8')
        equalizer_band9 = config.getfloat('apps_equalizer', 'band9')
        equalizer_band10 = config.getfloat('apps_equalizer', 'band10')
        equalizer_band11 = config.getfloat('apps_equalizer', 'band11')
        equalizer_band12 = config.getfloat('apps_equalizer', 'band12')
        equalizer_band13 = config.getfloat('apps_equalizer', 'band13')
        equalizer_band14 = config.getfloat('apps_equalizer', 'band14')

        eq_band0_freq = config.getfloat('apps_equalizer', 'band0_freq',
                                        fallback=26)
        eq_band1_freq = config.getfloat('apps_equalizer', 'band1_freq',
                                        fallback=41)
        eq_band2_freq = config.getfloat('apps_equalizer', 'band2_freq',
                                        fallback=65)
        eq_band3_freq = config.getfloat('apps_equalizer', 'band3_freq',
                                        fallback=103)
        eq_band4_freq = config.getfloat('apps_equalizer', 'band4_freq',
                                        fallback=163)
        eq_band5_freq = config.getfloat('apps_equalizer', 'band5_freq',
                                        fallback=259)
        eq_band6_freq = config.getfloat('apps_equalizer', 'band6_freq',
                                        fallback=410)
        eq_band7_freq = config.getfloat('apps_equalizer', 'band7_freq',
                                        fallback=649)
        eq_band8_freq = config.getfloat('apps_equalizer', 'band8_freq',
                                        fallback=1029)
        eq_band9_freq = config.getfloat('apps_equalizer', 'band9_freq',
                                        fallback=1631)
        eq_band10_freq = config.getfloat('apps_equalizer', 'band10_freq',
                                         fallback=2585)
        eq_band11_freq = config.getfloat('apps_equalizer', 'band11_freq',
                                         fallback=4097)
        eq_band12_freq = config.getfloat('apps_equalizer', 'band12_freq',
                                         fallback=6493)
        eq_band13_freq = config.getfloat('apps_equalizer', 'band13_freq',
                                         fallback=10291)
        eq_band14_freq = config.getfloat('apps_equalizer', 'band14_freq',
                                         fallback=16310)

        eq_band0_qfactor = config.getfloat('apps_equalizer', 'band0_qfactor',
                                           fallback=2.21)
        eq_band1_qfactor = config.getfloat('apps_equalizer', 'band1_qfactor',
                                           fallback=2.21)
        eq_band2_qfactor = config.getfloat('apps_equalizer', 'band2_qfactor',
                                           fallback=2.21)
        eq_band3_qfactor = config.getfloat('apps_equalizer', 'band3_qfactor',
                                           fallback=2.21)
        eq_band4_qfactor = config.getfloat('apps_equalizer', 'band4_qfactor',
                                           fallback=2.21)
        eq_band5_qfactor = config.getfloat('apps_equalizer', 'band5_qfactor',
                                           fallback=2.21)
        eq_band6_qfactor = config.getfloat('apps_equalizer', 'band6_qfactor',
                                           fallback=2.21)
        eq_band7_qfactor = config.getfloat('apps_equalizer', 'band7_qfactor',
                                           fallback=2.21)
        eq_band8_qfactor = config.getfloat('apps_equalizer', 'band8_qfactor',
                                           fallback=2.21)
        eq_band9_qfactor = config.getfloat('apps_equalizer', 'band9_qfactor',
                                           fallback=2.21)
        eq_band10_qfactor = config.getfloat('apps_equalizer', 'band10_qfactor',
                                            fallback=2.21)
        eq_band11_qfactor = config.getfloat('apps_equalizer', 'band11_qfactor',
                                            fallback=2.21)
        eq_band12_qfactor = config.getfloat('apps_equalizer', 'band12_qfactor',
                                            fallback=2.21)
        eq_band13_qfactor = config.getfloat('apps_equalizer', 'band13_qfactor',
                                            fallback=2.21)
        eq_band14_qfactor = config.getfloat('apps_equalizer', 'band14_qfactor',
                                            fallback=2.21)

        equalizer_bands = [equalizer_band0, equalizer_band1,
                           equalizer_band2, equalizer_band3,
                           equalizer_band4, equalizer_band5,
                           equalizer_band6, equalizer_band7,
                           equalizer_band8, equalizer_band9,
                           equalizer_band10, equalizer_band11,
                           equalizer_band12, equalizer_band13,
                           equalizer_band14]

        eq_freqs = [eq_band0_freq, eq_band1_freq, eq_band2_freq, eq_band3_freq,
                    eq_band4_freq, eq_band5_freq, eq_band6_freq, eq_band7_freq,
                    eq_band8_freq, eq_band9_freq, eq_band10_freq,
                    eq_band11_freq, eq_band12_freq, eq_band13_freq,
                    eq_band14_freq]

        eq_qfactors = [eq_band0_qfactor, eq_band1_qfactor, eq_band2_qfactor,
                       eq_band3_qfactor, eq_band4_qfactor, eq_band5_qfactor,
                       eq_band6_qfactor, eq_band7_qfactor, eq_band8_qfactor,
                       eq_band9_qfactor, eq_band10_qfactor, eq_band11_qfactor,
                       eq_band12_qfactor, eq_band13_qfactor, eq_band14_qfactor]

        settings.set_value('compressor-user', GLib.Variant('ad', compressor))

        settings.set_value('reverb-user', GLib.Variant('ad', reverb))

        settings.set_value('highpass-cutoff',
                           GLib.Variant('i', highpass_cutoff))
        settings.set_value('highpass-poles',
                           GLib.Variant('i', highpass_poles))

        settings.set_value('lowpass-cutoff',
                           GLib.Variant('i', lowpass_cutoff))
        settings.set_value('lowpass-poles',
                           GLib.Variant('i', lowpass_poles))

        settings.set_value('equalizer-input-gain',
                           GLib.Variant('d', equalizer_input_gain))
        settings.set_value('equalizer-output-gain',
                           GLib.Variant('d', equalizer_output_gain))
        settings.set_value('equalizer-user',
                           GLib.Variant('ad', equalizer_bands))
        settings.set_value('equalizer-freqs',
                           GLib.Variant('ad', eq_freqs))
        settings.set_value('equalizer-qfactors',
                           GLib.Variant('ad', eq_qfactors))

        self.sie.init_ui()

    def load_source_outputs_preset(self, config):
        settings = self.soe.settings

        limiter = dict(config['mic_limiter']).values()
        limiter = [float(v) for v in limiter]

        compressor = dict(config['mic_compressor']).values()
        compressor = [float(v) for v in compressor]

        reverb = dict(config['mic_reverb']).values()
        reverb = [float(v) for v in reverb]

        highpass_cutoff = config.getint('mic_equalizer',
                                        'cutoff',
                                        fallback=20)
        highpass_poles = config.getint('mic_equalizer',
                                       'poles',
                                       fallback=4)
        lowpass_cutoff = config.getint('mic_equalizer',
                                       'cutoff',
                                       fallback=20000)
        lowpass_poles = config.getint('mic_equalizer',
                                      'poles',
                                      fallback=4)

        equalizer_input_gain = config.getfloat('mic_equalizer', 'input_gain',
                                               fallback=0)
        equalizer_output_gain = config.getfloat('mic_equalizer',
                                                'output_gain',
                                                fallback=0)

        equalizer_band0 = config.getfloat('mic_equalizer', 'band0')
        equalizer_band1 = config.getfloat('mic_equalizer', 'band1')
        equalizer_band2 = config.getfloat('mic_equalizer', 'band2')
        equalizer_band3 = config.getfloat('mic_equalizer', 'band3')
        equalizer_band4 = config.getfloat('mic_equalizer', 'band4')
        equalizer_band5 = config.getfloat('mic_equalizer', 'band5')
        equalizer_band6 = config.getfloat('mic_equalizer', 'band6')
        equalizer_band7 = config.getfloat('mic_equalizer', 'band7')
        equalizer_band8 = config.getfloat('mic_equalizer', 'band8')
        equalizer_band9 = config.getfloat('mic_equalizer', 'band9')
        equalizer_band10 = config.getfloat('mic_equalizer', 'band10')
        equalizer_band11 = config.getfloat('mic_equalizer', 'band11')
        equalizer_band12 = config.getfloat('mic_equalizer', 'band12')
        equalizer_band13 = config.getfloat('mic_equalizer', 'band13')
        equalizer_band14 = config.getfloat('mic_equalizer', 'band14')

        eq_band0_freq = config.getfloat('mic_equalizer', 'band0_freq',
                                        fallback=26)
        eq_band1_freq = config.getfloat('mic_equalizer', 'band1_freq',
                                        fallback=41)
        eq_band2_freq = config.getfloat('mic_equalizer', 'band2_freq',
                                        fallback=65)
        eq_band3_freq = config.getfloat('mic_equalizer', 'band3_freq',
                                        fallback=103)
        eq_band4_freq = config.getfloat('mic_equalizer', 'band4_freq',
                                        fallback=163)
        eq_band5_freq = config.getfloat('mic_equalizer', 'band5_freq',
                                        fallback=259)
        eq_band6_freq = config.getfloat('mic_equalizer', 'band6_freq',
                                        fallback=410)
        eq_band7_freq = config.getfloat('mic_equalizer', 'band7_freq',
                                        fallback=649)
        eq_band8_freq = config.getfloat('mic_equalizer', 'band8_freq',
                                        fallback=1029)
        eq_band9_freq = config.getfloat('mic_equalizer', 'band9_freq',
                                        fallback=1631)
        eq_band10_freq = config.getfloat('mic_equalizer', 'band10_freq',
                                         fallback=2585)
        eq_band11_freq = config.getfloat('mic_equalizer', 'band11_freq',
                                         fallback=4097)
        eq_band12_freq = config.getfloat('mic_equalizer', 'band12_freq',
                                         fallback=6493)
        eq_band13_freq = config.getfloat('mic_equalizer', 'band13_freq',
                                         fallback=10291)
        eq_band14_freq = config.getfloat('mic_equalizer', 'band14_freq',
                                         fallback=16310)

        eq_band0_qfactor = config.getfloat('mic_equalizer', 'band0_qfactor',
                                           fallback=2.21)
        eq_band1_qfactor = config.getfloat('mic_equalizer', 'band1_qfactor',
                                           fallback=2.21)
        eq_band2_qfactor = config.getfloat('mic_equalizer', 'band2_qfactor',
                                           fallback=2.21)
        eq_band3_qfactor = config.getfloat('mic_equalizer', 'band3_qfactor',
                                           fallback=2.21)
        eq_band4_qfactor = config.getfloat('mic_equalizer', 'band4_qfactor',
                                           fallback=2.21)
        eq_band5_qfactor = config.getfloat('mic_equalizer', 'band5_qfactor',
                                           fallback=2.21)
        eq_band6_qfactor = config.getfloat('mic_equalizer', 'band6_qfactor',
                                           fallback=2.21)
        eq_band7_qfactor = config.getfloat('mic_equalizer', 'band7_qfactor',
                                           fallback=2.21)
        eq_band8_qfactor = config.getfloat('mic_equalizer', 'band8_qfactor',
                                           fallback=2.21)
        eq_band9_qfactor = config.getfloat('mic_equalizer', 'band9_qfactor',
                                           fallback=2.21)
        eq_band10_qfactor = config.getfloat('mic_equalizer', 'band10_qfactor',
                                            fallback=2.21)
        eq_band11_qfactor = config.getfloat('mic_equalizer', 'band11_qfactor',
                                            fallback=2.21)
        eq_band12_qfactor = config.getfloat('mic_equalizer', 'band12_qfactor',
                                            fallback=2.21)
        eq_band13_qfactor = config.getfloat('mic_equalizer', 'band13_qfactor',
                                            fallback=2.21)
        eq_band14_qfactor = config.getfloat('mic_equalizer', 'band14_qfactor',
                                            fallback=2.21)

        equalizer_bands = [equalizer_band0, equalizer_band1,
                           equalizer_band2, equalizer_band3,
                           equalizer_band4, equalizer_band5,
                           equalizer_band6, equalizer_band7,
                           equalizer_band8, equalizer_band9,
                           equalizer_band10, equalizer_band11,
                           equalizer_band12, equalizer_band13,
                           equalizer_band14]

        eq_freqs = [eq_band0_freq, eq_band1_freq, eq_band2_freq, eq_band3_freq,
                    eq_band4_freq, eq_band5_freq, eq_band6_freq, eq_band7_freq,
                    eq_band8_freq, eq_band9_freq, eq_band10_freq,
                    eq_band11_freq, eq_band12_freq, eq_band13_freq,
                    eq_band14_freq]

        eq_qfactors = [eq_band0_qfactor, eq_band1_qfactor, eq_band2_qfactor,
                       eq_band3_qfactor, eq_band4_qfactor, eq_band5_qfactor,
                       eq_band6_qfactor, eq_band7_qfactor, eq_band8_qfactor,
                       eq_band9_qfactor, eq_band10_qfactor, eq_band11_qfactor,
                       eq_band12_qfactor, eq_band13_qfactor, eq_band14_qfactor]

        settings.set_value('limiter-user', GLib.Variant('ad', limiter))

        settings.set_value('compressor-user', GLib.Variant('ad', compressor))

        settings.set_value('reverb-user', GLib.Variant('ad', reverb))

        settings.set_value('highpass-cutoff',
                           GLib.Variant('i', highpass_cutoff))
        settings.set_value('highpass-poles',
                           GLib.Variant('i', highpass_poles))

        settings.set_value('lowpass-cutoff',
                           GLib.Variant('i', lowpass_cutoff))
        settings.set_value('lowpass-poles',
                           GLib.Variant('i', lowpass_poles))

        settings.set_value('equalizer-input-gain',
                           GLib.Variant('d', equalizer_input_gain))
        settings.set_value('equalizer-output-gain',
                           GLib.Variant('d', equalizer_output_gain))
        settings.set_value('equalizer-user',
                           GLib.Variant('ad', equalizer_bands))
        settings.set_value('equalizer-freqs',
                           GLib.Variant('ad', eq_freqs))
        settings.set_value('equalizer-qfactors',
                           GLib.Variant('ad', eq_qfactors))

        self.soe.init_ui()

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

            self.load_sink_inputs_preset(config)
            self.load_source_outputs_preset(config)

        dialog.destroy()

    def onAbout(self, action, parameter):
        builder = Gtk.Builder()

        builder.add_from_file(self.module_path + '/ui/about.glade')

        dialog = builder.get_object('about_dialog')

        dialog.set_transient_for(self.window)

        dialog.run()

        dialog.destroy()
