# -*- coding: utf-8 -*-

import gi
import numpy as np
gi.require_version('Gst', '1.0')
from gi.repository import Gio, GLib
from PulseEffects.effects_ui_base import EffectsUiBase
from PulseEffects.sink_input_pipeline import SinkInputPipeline
from PulseEffectsCalibration.application import Application as Calibration
from scipy.interpolate import CubicSpline


class SinkInputEffects(EffectsUiBase, SinkInputPipeline):

    def __init__(self, sampling_rate):
        self.autovolume_enabled = False
        self.autovolume_target = -12  # dB
        self.autovolume_tolerance = 1  # dB
        self.autovolume_threshold = -50  # autovolume only if avg > threshold

        self.settings = Gio.Settings('com.github.wwmm.pulseeffects.sinkinputs')

        SinkInputPipeline.__init__(self, sampling_rate)
        EffectsUiBase.__init__(self, '/ui/sink_inputs_plugins.glade',
                               self.settings)

        self.ui_autovolume_enable = self.builder.get_object(
            'autovolume_enable')
        self.ui_autovolume_window = self.builder.get_object(
            'autovolume_window')
        self.ui_autovolume_target = self.builder.get_object(
            'autovolume_target')
        self.ui_autovolume_tolerance = self.builder.get_object(
            'autovolume_tolerance')
        self.ui_autovolume_threshold = self.builder.get_object(
            'autovolume_threshold')

        self.ui_panorama = self.builder.get_object('panorama_position')

        self.ui_panorama_input_level_left = self.builder.get_object(
            'panorama_input_level_left')
        self.ui_panorama_input_level_right = self.builder.get_object(
            'panorama_input_level_right')
        self.ui_panorama_output_level_left = self.builder.get_object(
            'panorama_output_level_left')
        self.ui_panorama_output_level_right = self.builder.get_object(
            'panorama_output_level_right')

        self.ui_panorama_input_level_left_label = self.builder.get_object(
            'panorama_input_level_left_label')
        self.ui_panorama_input_level_right_label = self.builder.get_object(
            'panorama_input_level_right_label')
        self.ui_panorama_output_level_left_label = self.builder.get_object(
            'panorama_output_level_left_label')
        self.ui_panorama_output_level_right_label = self.builder.get_object(
            'panorama_output_level_right_label')

        self.builder.connect_signals(self)

        self.connect('new_autovolume', self.on_new_autovolume)

    def auto_gain(self, max_value):
        max_value = int(max_value)

        if max_value > self.autovolume_target + self.autovolume_tolerance:
            gain = self.limiter.get_property('input-gain')

            if gain - 1 >= -20:
                gain = gain - 1

                self.emit('new_autovolume', gain)
        elif max_value < self.autovolume_target - self.autovolume_tolerance:
            gain = self.limiter.get_property('input-gain')

            if gain + 1 <= 20:
                gain = gain + 1

                self.emit('new_autovolume', gain)

    def on_message_element(self, bus, msg):
        plugin = msg.src.get_name()

        if plugin == 'limiter_input_level':
            peak = msg.get_structure().get_value('peak')

            self.ui_update_limiter_input_level(peak)
        elif plugin == 'limiter_output_level':
            peak = msg.get_structure().get_value('peak')

            self.ui_update_limiter_output_level(peak)
        elif plugin == 'autovolume':
            if self.autovolume_enabled:
                peak = msg.get_structure().get_value('peak')

                max_value = max(peak)

                if max_value > self.autovolume_threshold:
                    self.auto_gain(max_value)
        elif plugin == 'panorama_output_level':
            peak = msg.get_structure().get_value('peak')

            left, right = peak[0], peak[1]

            if left >= -99:
                l_value = 10**(left / 20)
                self.ui_panorama_output_level_left.set_value(l_value)
                self.ui_panorama_output_level_left_label.set_text(
                    str(round(left)))
            else:
                self.ui_panorama_output_level_left.set_value(0)
                self.ui_panorama_output_level_left_label.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.ui_panorama_output_level_right.set_value(r_value)
                self.ui_panorama_output_level_right_label.set_text(
                    str(round(right)))
            else:
                self.ui_panorama_output_level_right.set_value(0)
                self.ui_panorama_output_level_right_label.set_text('-99')
        elif plugin == 'compressor_input_level':
            peak = msg.get_structure().get_value('peak')

            self.ui_update_compressor_input_level(peak)
        elif plugin == 'compressor_output_level':
            peak = msg.get_structure().get_value('peak')

            self.ui_update_compressor_output_level(peak)

        elif plugin == 'reverb_input_level':
            peak = msg.get_structure().get_value('peak')

            self.ui_update_reverb_input_level(peak)
        elif plugin == 'reverb_output_level':
            peak = msg.get_structure().get_value('peak')

            self.ui_update_reverb_output_level(peak)
        elif plugin == 'highpass_input_level':
            peak = msg.get_structure().get_value('peak')

            self.ui_update_highpass_input_level(peak)
        elif plugin == 'highpass_output_level':
            peak = msg.get_structure().get_value('peak')

            self.ui_update_highpass_output_level(peak)
        elif plugin == 'lowpass_input_level':
            peak = msg.get_structure().get_value('peak')

            self.ui_update_lowpass_input_level(peak)
        elif plugin == 'lowpass_output_level':
            peak = msg.get_structure().get_value('peak')

            self.ui_update_lowpass_output_level(peak)
        elif plugin == 'equalizer_input_level':
            peak = msg.get_structure().get_value('peak')

            self.ui_update_equalizer_input_level(peak)
        elif plugin == 'equalizer_output_level':
            peak = msg.get_structure().get_value('peak')

            self.ui_update_equalizer_output_level(peak)
        elif plugin == 'spectrum':
            magnitudes = msg.get_structure().get_value('magnitude')

            cs = CubicSpline(self.spectrum_freqs,
                             magnitudes[:self.spectrum_nfreqs])

            magnitudes = cs(self.spectrum_x_axis)

            max_mag = np.amax(magnitudes)
            min_mag = self.spectrum_threshold

            if max_mag > min_mag:
                magnitudes = (min_mag - magnitudes) / min_mag

                self.emit('new_spectrum', magnitudes)

        return True

    def init_autovolume_ui(self):
        self.autovolume_enabled = self.settings.get_value(
            'autovolume-state').unpack()
        autovolume_window = self.settings.get_value(
            'autovolume-window').unpack()
        self.autovolume_target = self.settings.get_value(
            'autovolume-target').unpack()
        self.autovolume_tolerance = self.settings.get_value(
            'autovolume-tolerance').unpack()
        self.autovolume_threshold = self.settings.get_value(
            'autovolume-threshold').unpack()

        self.ui_autovolume_enable.set_state(self.autovolume_enabled)
        self.ui_autovolume_window.set_value(autovolume_window)
        self.ui_autovolume_target.set_value(self.autovolume_target)
        self.ui_autovolume_tolerance.set_value(self.autovolume_tolerance)
        self.ui_autovolume_threshold.set_value(self.autovolume_threshold)

        if self.autovolume_enabled:
            self.enable_autovolume(True)

    def init_panorama_ui(self):
        panorama = self.settings.get_value('panorama-position').unpack()

        self.ui_panorama.set_value(panorama)

        self.panorama.set_property('panorama', panorama)

    def init_ui(self):
        self.init_limiter_ui()
        self.init_autovolume_ui()
        self.init_panorama_ui()
        self.init_compressor_ui()
        self.init_reverb_ui()
        self.init_highpass_ui()
        self.init_lowpass_ui()
        self.init_equalizer_ui()

    def enable_autovolume(self, state):
        self.autovolume_enabled = state

        if state:
            window = self.settings.get_value('autovolume-window').unpack()
            target = self.settings.get_value('autovolume-target').unpack()
            tolerance = self.settings.get_value(
                'autovolume-tolerance').unpack()

            self.ui_limiter_input_gain.set_value(-10)
            self.ui_limiter_limit.set_value(target + tolerance)
            self.ui_limiter_release_time.set_value(window)

            self.ui_limiter_input_gain.set_sensitive(False)
            self.ui_limiter_limit.set_sensitive(False)
            self.ui_limiter_release_time.set_sensitive(False)
        else:
            self.ui_limiter_input_gain.set_value(-10)
            self.ui_limiter_limit.set_value(0)
            self.ui_limiter_release_time.set_value(1.0)

            self.ui_limiter_input_gain.set_sensitive(True)
            self.ui_limiter_limit.set_sensitive(True)
            self.ui_limiter_release_time.set_sensitive(True)

        out = GLib.Variant('b', state)
        self.settings.set_value('autovolume-state', out)

    def on_autovolume_enable_state_set(self, obj, state):
        self.enable_autovolume(state)

    def on_autovolume_window_value_changed(self, obj):
        value = obj.get_value()

        # value must be in seconds
        self.autovolume_level.set_property('interval', int(value * 1000000000))

        self.ui_limiter_release_time.set_value(value)

        out = GLib.Variant('d', value)
        self.settings.set_value('autovolume-window', out)

    def on_autovolume_target_value_changed(self, obj):
        value = obj.get_value()

        self.autovolume_target = value

        tolerance = self.settings.get_value('autovolume-tolerance').unpack()

        self.ui_limiter_limit.set_value(value + tolerance)

        out = GLib.Variant('i', value)
        self.settings.set_value('autovolume-target', out)

    def on_autovolume_tolerance_value_changed(self, obj):
        value = obj.get_value()

        self.autovolume_tolerance = value

        target = self.settings.get_value('autovolume-target').unpack()

        self.ui_limiter_limit.set_value(target + value)

        out = GLib.Variant('i', value)
        self.settings.set_value('autovolume-tolerance', out)

    def on_autovolume_threshold_value_changed(self, obj):
        value = obj.get_value()

        self.autovolume_threshold = value

        out = GLib.Variant('i', value)
        self.settings.set_value('autovolume-threshold', out)

    def on_new_autovolume(self, obj, gain):
        self.ui_limiter_input_gain.set_value(gain)

    def on_panorama_position_value_changed(self, obj):
        value = obj.get_value()

        self.panorama.set_property('panorama', value)

        out = GLib.Variant('d', value)
        self.settings.set_value('panorama-position', out)

    def on_eq_flat_response_button_clicked(self, obj):
        self.apply_eq_preset([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0])

    def on_eq_reset_freqs_button_clicked(self, obj):
        self.settings.reset('equalizer-freqs')
        self.init_eq_freq_and_qfactors()

    def on_eq_reset_qfactors_button_clicked(self, obj):
        self.settings.reset('equalizer-qfactors')
        self.init_eq_freq_and_qfactors()

    def on_eq_calibrate_button_clicked(self, obj):
        c = Calibration()
        c.run()

    def reset(self):
        self.settings.reset('limiter-user')
        self.settings.reset('autovolume-state')
        self.settings.reset('autovolume-window')
        self.settings.reset('autovolume-target')
        self.settings.reset('autovolume-tolerance')
        self.settings.reset('autovolume-threshold')
        self.settings.reset('panorama-position')
        self.settings.reset('compressor-user')
        self.settings.reset('reverb-user')
        self.settings.reset('highpass-cutoff')
        self.settings.reset('highpass-poles')
        self.settings.reset('lowpass-cutoff')
        self.settings.reset('lowpass-poles')
        self.settings.reset('equalizer-input-gain')
        self.settings.reset('equalizer-output-gain')
        self.settings.reset('equalizer-user')
        self.settings.reset('equalizer-freqs')
        self.settings.reset('equalizer-qfactors')

        self.init_ui()
