# -*- coding: utf-8 -*-

import os

import gi
import numpy as np
gi.require_version('Gst', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GLib, Gtk
from PulseEffects.sink_input_pipeline import SinkInputPipeline
from scipy.interpolate import CubicSpline

from PulseEffects.equalizer import Equalizer
from PulseEffects.lowpass import Lowpass
from PulseEffects.highpass import Highpass
from PulseEffects.reverb import Reverb


class SinkInputEffects(SinkInputPipeline):

    def __init__(self, sampling_rate):
        SinkInputPipeline.__init__(self, sampling_rate)

        self.module_path = os.path.dirname(__file__)

        self.autovolume_enabled = False
        self.autovolume_target = -12  # dB
        self.autovolume_tolerance = 1  # dB
        self.autovolume_threshold = -50  # autovolume only if avg > threshold

        self.settings = Gio.Settings('com.github.wwmm.pulseeffects.sinkinputs')

        self.builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + '/ui/effects_box.glade')

        self.ui_window = self.builder.get_object('window')

        self.stack = self.builder.get_object('stack')

        # self.ui_autovolume_enable = self.builder.get_object(
        #     'autovolume_enable')
        # self.ui_autovolume_window = self.builder.get_object(
        #     'autovolume_window')
        # self.ui_autovolume_target = self.builder.get_object(
        #     'autovolume_target')
        # self.ui_autovolume_tolerance = self.builder.get_object(
        #     'autovolume_tolerance')
        # self.ui_autovolume_threshold = self.builder.get_object(
        #     'autovolume_threshold')
        #
        # self.ui_panorama = self.builder.get_object('panorama_position')
        #
        # self.ui_panorama_input_level_left = self.builder.get_object(
        #     'panorama_input_level_left')
        # self.ui_panorama_input_level_right = self.builder.get_object(
        #     'panorama_input_level_right')
        # self.ui_panorama_output_level_left = self.builder.get_object(
        #     'panorama_output_level_left')
        # self.ui_panorama_output_level_right = self.builder.get_object(
        #     'panorama_output_level_right')
        #
        # self.ui_panorama_input_level_left_label = self.builder.get_object(
        #     'panorama_input_level_left_label')
        # self.ui_panorama_input_level_right_label = self.builder.get_object(
        #     'panorama_input_level_right_label')
        # self.ui_panorama_output_level_left_label = self.builder.get_object(
        #     'panorama_output_level_left_label')
        # self.ui_panorama_output_level_right_label = self.builder.get_object(
        #     'panorama_output_level_right_label')

        self.reverb = Reverb(self.settings)
        self.highpass = Highpass(self.settings)
        self.lowpass = Lowpass(self.settings)
        self.equalizer = Equalizer(self.settings)

        self.stack.add_titled(self.reverb.ui_window, 'Reverb', 'Reverberation')
        self.stack.add_titled(self.highpass.ui_window, 'Highpass', 'High pass')
        self.stack.add_titled(self.lowpass.ui_window, 'Lowpass', 'Low pass')
        self.stack.add_titled(self.equalizer.ui_window, 'Equalizer',
                              'Equalizer')

        self.effects_bin.append(self.reverb.bin, self.on_filter_added, None)
        self.effects_bin.append(self.highpass.bin, self.on_filter_added, None)
        self.effects_bin.append(self.lowpass.bin, self.on_filter_added, None)
        self.effects_bin.append(self.equalizer.bin, self.on_filter_added, None)
        self.effects_bin.append(self.spectrum, self.on_filter_added, None)

        # self.builder.connect_signals(self)

        # self.connect('new_autovolume', self.on_new_autovolume)

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

    def ui_update_panorama_input_level(self, peak):
        widgets = [self.ui_panorama_input_level_left,
                   self.ui_panorama_input_level_right,
                   self.ui_panorama_input_level_left_label,
                   self.ui_panorama_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_panorama_output_level(self, peak):
        widgets = [self.ui_panorama_output_level_left,
                   self.ui_panorama_output_level_right,
                   self.ui_panorama_output_level_left_label,
                   self.ui_panorama_output_level_right_label]

        self.ui_update_level(widgets, peak)

    def on_message_element(self, bus, msg):
        plugin = msg.src.get_name()

        if plugin == 'limiter_input_level':
            peak = msg.get_structure().get_value('peak')

            # self.ui_update_limiter_input_level(peak)
        elif plugin == 'limiter_output_level':
            peak = msg.get_structure().get_value('peak')

            # self.ui_update_limiter_output_level(peak)
        elif plugin == 'autovolume':
            if self.autovolume_enabled:
                peak = msg.get_structure().get_value('peak')

                max_value = max(peak)

                if max_value > self.autovolume_threshold:
                    self.auto_gain(max_value)
        elif plugin == 'panorama_input_level':
            peak = msg.get_structure().get_value('peak')

            # self.ui_update_panorama_input_level(peak)
        elif plugin == 'panorama_output_level':
            peak = msg.get_structure().get_value('peak')

            # self.ui_update_panorama_output_level(peak)
        elif plugin == 'compressor_input_level':
            peak = msg.get_structure().get_value('peak')

            # self.ui_update_compressor_input_level(peak)
        elif plugin == 'compressor_output_level':
            peak = msg.get_structure().get_value('peak')

            # self.ui_update_compressor_output_level(peak)

        elif plugin == 'reverb_input_level':
            peak = msg.get_structure().get_value('peak')

            self.reverb.ui_update_reverb_input_level(peak)
        elif plugin == 'reverb_output_level':
            peak = msg.get_structure().get_value('peak')

            self.reverb.ui_update_reverb_output_level(peak)
        elif plugin == 'highpass_input_level':
            peak = msg.get_structure().get_value('peak')

            self.highpass.ui_update_highpass_input_level(peak)
        elif plugin == 'highpass_output_level':
            peak = msg.get_structure().get_value('peak')

            self.highpass.ui_update_highpass_output_level(peak)
        elif plugin == 'lowpass_input_level':
            peak = msg.get_structure().get_value('peak')

            self.lowpass.ui_update_lowpass_input_level(peak)
        elif plugin == 'lowpass_output_level':
            peak = msg.get_structure().get_value('peak')

            self.lowpass.ui_update_lowpass_output_level(peak)
        elif plugin == 'equalizer_input_level':
            peak = msg.get_structure().get_value('peak')

            self.equalizer.ui_update_equalizer_input_level(peak)
        elif plugin == 'equalizer_output_level':
            peak = msg.get_structure().get_value('peak')

            self.equalizer.ui_update_equalizer_output_level(peak)
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
        # self.init_limiter_ui()
        # self.init_autovolume_ui()
        # self.init_panorama_ui()
        # self.init_compressor_ui()
        self.reverb.init_ui()
        self.highpass.init_ui()
        self.lowpass.init_ui()
        self.equalizer.init_ui()

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

    def reset(self):
        self.settings.reset('limiter-user')
        self.settings.reset('autovolume-state')
        self.settings.reset('autovolume-window')
        self.settings.reset('autovolume-target')
        self.settings.reset('autovolume-tolerance')
        self.settings.reset('autovolume-threshold')
        self.settings.reset('panorama-position')
        self.settings.reset('compressor-user')

        self.reverb.reset()
        self.highpass.reset()
        self.lowpass.reset()
        self.equalizer.reset()

        self.init_ui()
