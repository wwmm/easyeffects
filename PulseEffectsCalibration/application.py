# -*- coding: utf-8 -*-

import logging
import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk

from PulseEffectsCalibration.microphone_pipeline import MicrophonePipeline
from PulseEffectsCalibration.setup_equalizer import SetupEqualizer
from PulseEffectsCalibration.setup_test_signal import SetupTestSignal
from PulseEffectsCalibration.spectrum import Spectrum
from PulseEffectsCalibration.test_signal import TestSignal


class Application(Gtk.Application):

    def __init__(self):
        app_id = 'com.github.wwmm.pulseeffects.calibration'

        Gtk.Application.__init__(self, application_id=app_id)

        self.ui_initialized = False
        self.module_path = os.path.dirname(__file__)

        log_format = '%(asctime)s.%(msecs)d - %(name)s - %(levelname)s'
        log_format = log_format + ' - %(message)s'

        logging.basicConfig(format=log_format,
                            datefmt='%H:%M:%S',
                            level=logging.INFO)

        self.log = logging.getLogger('PulseEffectsCalibration')

        self.mp = MicrophonePipeline()
        self.ts = TestSignal()

        self.mp.connect('noise_measured', self.on_noise_measured)

        self.mp.set_state('ready')
        self.ts.set_state('ready')

    def do_startup(self):
        Gtk.Application.do_startup(self)

        self.builder = Gtk.Builder()
        self.calibration_mic_builder = Gtk.Builder()
        self.test_signal_builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + '/ui/main_ui.glade')
        self.builder.add_from_file(self.module_path + '/ui/headerbar.glade')
        self.calibration_mic_builder.add_from_file(
            self.module_path + '/ui/calibration_mic_plugins.glade')
        self.test_signal_builder.add_from_file(self.module_path +
                                               '/ui/test_signal.glade')

        headerbar = self.builder.get_object('headerbar')

        self.window = self.builder.get_object('MainWindow')
        self.window.set_titlebar(headerbar)
        self.window.set_application(self)

        # main window handlers

        self.spectrum = Spectrum(self)

        main_ui_handlers = {
            'on_MainWindow_delete_event': self.on_MainWindow_delete_event
        }

        main_ui_handlers.update(self.spectrum.handlers)

        self.builder.connect_signals(main_ui_handlers)

        self.setup_equalizer = SetupEqualizer(self.calibration_mic_builder,
                                              self.mp)

        self.setup_equalizer.init()

        calibration_mic_ui_handlers = {
            'on_time_window_value_changed':
                self.on_time_window_value_changed,
            'on_measure_noise_clicked':
                self.on_measure_noise_clicked,
            'on_subtract_noise_toggled':
                self.on_subtract_noise_toggled,
            'on_guideline_position_value_changed':
                self.on_guideline_position_value_changed
        }

        calibration_mic_ui_handlers.update(self.setup_equalizer.handlers)

        self.calibration_mic_builder.connect_signals(
            calibration_mic_ui_handlers)

        # init test signal widgets
        self.setup_test_signal = SetupTestSignal(self.test_signal_builder,
                                                 self.ts)

        # init stack widgets
        self.init_stack_widgets()

        # other initializations

        time_window = self.calibration_mic_builder.get_object('time_window')
        guideline_position = self.calibration_mic_builder.get_object(
            'guideline_position')
        self.measure_noise_spinner = self.calibration_mic_builder.get_object(
            'measure_noise_spinner')

        time_window.set_value(2)
        guideline_position.set_value(0.5)

        self.setup_equalizer.connect_signals()

        self.mp.set_state('playing')

    def do_activate(self):
        self.window.present()

        self.ui_initialized = True

    def on_MainWindow_delete_event(self, event, data):
        self.mp.set_state('null')
        self.ts.set_state('null')

        self.quit()

    def init_stack_widgets(self):
        stack_switcher = self.builder.get_object('stack_switcher')
        stack_box = self.builder.get_object('stack_box')

        calibration_mic_ui = self.calibration_mic_builder.get_object('window')
        test_signal_ui = self.test_signal_builder.get_object('window')

        stack = Gtk.Stack()
        stack.set_transition_type(Gtk.StackTransitionType.CROSSFADE)
        stack.set_transition_duration(250)
        stack.set_homogeneous(False)

        stack.add_named(test_signal_ui, "test_signal")
        stack.child_set_property(test_signal_ui, 'icon-name',
                                 'emblem-music-symbolic')

        stack.add_named(calibration_mic_ui, 'calibration_mic')
        stack.child_set_property(calibration_mic_ui, 'icon-name',
                                 'audio-input-microphone-symbolic')

        self.stack_current_child_name = 'test_signal'

        self.spectrum_handler_id = self.ts.connect('new_spectrum',
                                                   self.spectrum
                                                   .on_new_spectrum)

        def on_visible_child_changed(stack, visible_child):
            name = stack.get_visible_child_name()

            if name == 'test_signal':
                if self.stack_current_child_name == 'calibration_mic':
                    self.mp.disconnect(self.spectrum_handler_id)

                self.spectrum_handler_id = self.ts.connect('new_spectrum',
                                                           self.spectrum
                                                           .on_new_spectrum)

                self.spectrum.draw_guideline = False

                self.stack_current_child_name = 'test_signal'
            elif name == 'calibration_mic':
                if self.stack_current_child_name == 'test_signal':
                    self.ts.disconnect(self.spectrum_handler_id)

                self.spectrum_handler_id = self.mp.connect('new_spectrum',
                                                           self.spectrum
                                                           .on_new_spectrum)

                self.spectrum.draw_guideline = True

                self.stack_current_child_name = 'calibration_mic'

            self.spectrum.clear()

        stack.connect("notify::visible-child", on_visible_child_changed)

        stack_switcher.set_stack(stack)

        stack_box.pack_start(stack, True, True, 0)
        stack_box.show_all()

    def on_time_window_value_changed(self, obj):
        value = obj.get_value()

        self.mp.set_time_window(value)

    def on_measure_noise_clicked(self, obj):
        self.mp.measure_noise = True
        self.measure_noise_spinner.start()

    def on_subtract_noise_toggled(self, obj):
        self.mp.subtract_noise = obj.get_active()

    def on_noise_measured(self, obj):
        self.measure_noise_spinner.stop()

    def on_guideline_position_value_changed(self, obj):
        self.spectrum.set_guideline_position(obj.get_value())
