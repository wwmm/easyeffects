# -*- coding: utf-8 -*-
import gettext
import os

import gi
import numpy as np
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, Gtk
from PulseEffects.compressor import Compressor
from PulseEffects.equalizer import Equalizer
from PulseEffects.highpass import Highpass
from PulseEffects.limiter import Limiter
from PulseEffects.lowpass import Lowpass
from PulseEffects.pipeline_base import PipelineBase
from PulseEffects.reverb import Reverb
from scipy.interpolate import CubicSpline

gettext.textdomain('PulseEffects')
_ = gettext.gettext


class SourceOutputEffects(PipelineBase):

    def __init__(self, sampling_rate):
        self.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs')

        PipelineBase.__init__(self, sampling_rate)

        self.module_path = os.path.dirname(__file__)

        self.log_tag = 'mic: '

        self.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs')

        self.builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + '/ui/effects_box.glade')

        self.ui_window = self.builder.get_object('window')

        self.stack = self.builder.get_object('stack')

        self.limiter = Limiter(self.settings)
        self.compressor = Compressor(self.settings)
        self.reverb = Reverb(self.settings)
        self.highpass = Highpass(self.settings)
        self.lowpass = Lowpass(self.settings)
        self.equalizer = Equalizer(self.settings)

        # it makes no sense to show the calibration button here

        self.equalizer.ui_eq_calibrate_button.destroy()

        # adding effects widgets to the stack

        self.stack.add_titled(self.limiter.ui_window, 'Limiter',
                              _('Input Limiter'))
        self.stack.add_titled(self.compressor.ui_window, 'Compressor',
                              _('Compressor'))
        self.stack.add_titled(self.reverb.ui_window, 'Reverb',
                              _('Reverberation'))
        self.stack.add_titled(self.highpass.ui_window, 'Highpass',
                              _('High pass'))
        self.stack.add_titled(self.lowpass.ui_window, 'Lowpass',
                              _('Low pass'))
        self.stack.add_titled(self.equalizer.ui_window, 'Equalizer',
                              _('Equalizer'))

        # on/off switches connections

        self.limiter.ui_limiter_enable.connect('state-set',
                                               self.on_limiter_enable)
        self.compressor.ui_enable.connect('state-set',
                                          self.on_compressor_enable)
        self.reverb.ui_reverb_enable.connect('state-set',
                                             self.on_reverb_enable)
        self.highpass.ui_highpass_enable.connect('state-set',
                                                 self.on_highpass_enable)
        self.lowpass.ui_lowpass_enable.connect('state-set',
                                               self.on_lowpass_enable)
        self.equalizer.ui_equalizer_enable.connect('state-set',
                                                   self.on_equalizer_enable)

        # order is important

        self.limiter.bind()
        self.compressor.bind()
        self.reverb.bind()
        self.highpass.bind()
        self.lowpass.bind()

    def on_message_element(self, bus, msg):
        plugin = msg.src.get_name()

        if plugin == 'limiter_input_level':
            peak = msg.get_structure().get_value('peak')

            self.limiter.ui_update_limiter_input_level(peak)
        elif plugin == 'limiter_output_level':
            peak = msg.get_structure().get_value('peak')

            self.limiter.ui_update_limiter_output_level(peak)
        elif plugin == 'autovolume':
            peak = msg.get_structure().get_value('peak')

            max_value = max(peak)

            if max_value > self.limiter.autovolume_threshold:
                self.limiter.auto_gain(max_value)
        elif plugin == 'compressor_input_level':
            peak = msg.get_structure().get_value('peak')

            self.compressor.ui_update_compressor_input_level(peak)
        elif plugin == 'compressor_output_level':
            peak = msg.get_structure().get_value('peak')

            self.compressor.ui_update_compressor_output_level(peak)
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

    def on_limiter_enable(self, obj, state):
        if state:
            self.effects_bin.prepend(self.limiter.bin, self.on_filter_added,
                                     self.log_tag)
        else:
            self.effects_bin.remove(self.limiter.bin, self.on_filter_removed,
                                    self.log_tag)

    def on_compressor_enable(self, obj, state):
        limiter_enabled = self.settings.get_value('limiter-state').unpack()

        if state:
            if limiter_enabled:
                self.effects_bin.insert_after(self.compressor.bin,
                                              self.limiter.bin,
                                              self.on_filter_added,
                                              self.log_tag)
            else:
                self.effects_bin.prepend(self.compressor.bin,
                                         self.on_filter_added, self.log_tag)
        else:
            self.effects_bin.remove(self.compressor.bin,
                                    self.on_filter_removed,
                                    self.log_tag)

    def on_reverb_enable(self, obj, state):
        limiter_enabled = self.settings.get_value('limiter-state').unpack()
        compressor_enabled = self.settings.get_value(
            'compressor-state').unpack()

        if state:
            if compressor_enabled:
                self.effects_bin.insert_after(self.reverb.bin,
                                              self.compressor.bin,
                                              self.on_filter_added,
                                              self.log_tag)
            elif limiter_enabled:
                self.effects_bin.insert_after(self.reverb.bin,
                                              self.limiter.bin,
                                              self.on_filter_added,
                                              self.log_tag)
            else:
                self.effects_bin.prepend(self.reverb.bin,
                                         self.on_filter_added, self.log_tag)
        else:
            self.effects_bin.remove(self.reverb.bin, self.on_filter_removed,
                                    self.log_tag)

    def on_highpass_enable(self, obj, state):
        limiter_enabled = self.settings.get_value('limiter-state').unpack()
        compressor_enabled = self.settings.get_value(
            'compressor-state').unpack()
        reverb_enabled = self.settings.get_value(
            'reverb-state').unpack()

        if state:
            if reverb_enabled:
                self.effects_bin.insert_after(self.highpass.bin,
                                              self.reverb.bin,
                                              self.on_filter_added,
                                              self.log_tag)
            elif compressor_enabled:
                self.effects_bin.insert_after(self.highpass.bin,
                                              self.compressor.bin,
                                              self.on_filter_added,
                                              self.log_tag)
            elif limiter_enabled:
                self.effects_bin.insert_after(self.highpass.bin,
                                              self.limiter.bin,
                                              self.on_filter_added,
                                              self.log_tag)
            else:
                self.effects_bin.prepend(self.highpass.bin,
                                         self.on_filter_added, self.log_tag)
        else:
            self.effects_bin.remove(self.highpass.bin, self.on_filter_removed,
                                    self.log_tag)

    def on_lowpass_enable(self, obj, state):
        limiter_enabled = self.settings.get_value('limiter-state').unpack()
        compressor_enabled = self.settings.get_value(
            'compressor-state').unpack()
        reverb_enabled = self.settings.get_value(
            'reverb-state').unpack()
        highpass_enabled = self.settings.get_value(
            'highpass-state').unpack()

        if state:
            if highpass_enabled:
                self.effects_bin.insert_after(self.lowpass.bin,
                                              self.highpass.bin,
                                              self.on_filter_added,
                                              self.log_tag)
            elif reverb_enabled:
                self.effects_bin.insert_after(self.lowpass.bin,
                                              self.reverb.bin,
                                              self.on_filter_added,
                                              self.log_tag)
            elif compressor_enabled:
                self.effects_bin.insert_after(self.lowpass.bin,
                                              self.compressor.bin,
                                              self.on_filter_added,
                                              self.log_tag)
            elif limiter_enabled:
                self.effects_bin.insert_after(self.lowpass.bin,
                                              self.limiter.bin,
                                              self.on_filter_added,
                                              self.log_tag)
            else:
                self.effects_bin.prepend(self.lowpass.bin,
                                         self.on_filter_added, self.log_tag)
        else:
            self.effects_bin.remove(self.lowpass.bin, self.on_filter_removed,
                                    self.log_tag)

    def on_equalizer_enable(self, obj, state):
        limiter_enabled = self.settings.get_value('limiter-state').unpack()
        compressor_enabled = self.settings.get_value(
            'compressor-state').unpack()
        reverb_enabled = self.settings.get_value(
            'reverb-state').unpack()
        highpass_enabled = self.settings.get_value(
            'highpass-state').unpack()
        lowpass_enabled = self.settings.get_value(
            'lowpass-state').unpack()

        if state:
            if lowpass_enabled:
                self.effects_bin.insert_after(self.equalizer.bin,
                                              self.lowpass.bin,
                                              self.on_filter_added,
                                              self.log_tag)
            elif highpass_enabled:
                self.effects_bin.insert_after(self.equalizer.bin,
                                              self.highpass.bin,
                                              self.on_filter_added,
                                              self.log_tag)
            elif reverb_enabled:
                self.effects_bin.insert_after(self.equalizer.bin,
                                              self.reverb.bin,
                                              self.on_filter_added,
                                              self.log_tag)
            elif compressor_enabled:
                self.effects_bin.insert_after(self.equalizer.bin,
                                              self.compressor.bin,
                                              self.on_filter_added,
                                              self.log_tag)
            elif limiter_enabled:
                self.effects_bin.insert_after(self.equalizer.bin,
                                              self.limiter.bin,
                                              self.on_filter_added,
                                              self.log_tag)
            else:
                self.effects_bin.prepend(self.equalizer.bin,
                                         self.on_filter_added, self.log_tag)
        else:
            self.effects_bin.remove(self.equalizer.bin, self.on_filter_removed,
                                    self.log_tag)

    def enable_spectrum(self, state):
        if state:
            self.effects_bin.append(self.spectrum, self.on_filter_added,
                                    self.log_tag)
        else:
            self.effects_bin.remove(self.spectrum, self.on_filter_removed,
                                    self.log_tag)

    def init_ui(self):
        self.equalizer.bind()
        self.equalizer.init_ui()

    def reset(self):
        self.limiter.reset()
        self.compressor.reset()
        self.reverb.reset()
        self.highpass.reset()
        self.lowpass.reset()
        self.equalizer.reset()

        self.init_ui()
