# -*- coding: utf-8 -*-

import logging
import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk

from PulseEffectsTest.microphone_pipeline import MicrophonePipeline
from PulseEffectsTest.setup_test_signal import SetupTestSignal
from PulseEffectsTest.spectrum import Spectrum
from PulseEffectsTest.test_signal import TestSignal


class Application(Gtk.Application):

    def __init__(self, pulse_manager):
        app_id = 'com.github.wwmm.pulseeffects.test'
        self.pm = pulse_manager

        Gtk.Application.__init__(self, application_id=app_id)

    def do_startup(self):
        Gtk.Application.do_startup(self)

        self.ui_initialized = False
        self.module_path = os.path.dirname(__file__)

        log_format = '%(asctime)s.%(msecs)d - %(name)s - %(levelname)s'
        log_format = log_format + ' - %(message)s'

        logging.basicConfig(format=log_format,
                            datefmt='%H:%M:%S',
                            level=logging.INFO)

        self.log = logging.getLogger('PulseEffectsTest')

        self.mp = MicrophonePipeline()
        self.ts = TestSignal()

        self.pm.connect('new_default_source', self.update_source_monitor_name)
        self.mp.connect('noise_measured', self.on_noise_measured)

        self.mp.set_source_monitor_name(self.pm.default_source_name)

        self.mp.set_state('ready')
        self.ts.set_state('ready')

        self.builder = Gtk.Builder()
        self.calibration_mic_builder = Gtk.Builder()
        self.test_signal_builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + '/ui/main_ui.glade')
        self.calibration_mic_builder.add_from_file(
            self.module_path + '/ui/calibration_mic.glade')
        self.test_signal_builder.add_from_file(self.module_path +
                                               '/ui/test_signal.glade')

        self.window = self.builder.get_object('MainWindow')
        self.window.set_application(self)

        # main window handlers

        self.spectrum = Spectrum(self)

        main_ui_handlers = {}

        main_ui_handlers.update(self.spectrum.handlers)

        self.builder.connect_signals(main_ui_handlers)

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

        self.mp.set_state('playing')

    def do_activate(self):
        self.window.present()

        self.ui_initialized = True

    def do_shutdown(self):
        Gtk.Application.do_shutdown(self)

        self.mp.set_state('null')
        self.ts.set_state('null')

    def init_stack_widgets(self):
        stack = self.builder.get_object('stack')

        calibration_mic_ui = self.calibration_mic_builder.get_object('window')
        test_signal_ui = self.test_signal_builder.get_object('window')

        stack.add_named(test_signal_ui, "test_signal")
        stack.child_set_property(test_signal_ui, 'icon-name',
                                 'pulseeffects-sine-symbolic')

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

    def update_source_monitor_name(self, obj, name):
        self.mp.set_source_monitor_name(name)
