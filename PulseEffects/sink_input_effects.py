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
from PulseEffects.panorama import Panorama
from PulseEffects.reverb import Reverb
from PulseEffects.pipeline_base import PipelineBase
from scipy.interpolate import CubicSpline

gettext.textdomain('PulseEffects')
_ = gettext.gettext


class SinkInputEffects(PipelineBase):

    def __init__(self, sampling_rate):
        PipelineBase.__init__(self, sampling_rate)

        self.module_path = os.path.dirname(__file__)

        self.settings = Gio.Settings('com.github.wwmm.pulseeffects.sinkinputs')

        self.builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + '/ui/effects_box.glade')

        self.ui_window = self.builder.get_object('window')

        self.stack = self.builder.get_object('stack')

        self.limiter = Limiter(self.settings)
        self.panorama = Panorama(self.settings)
        self.compressor = Compressor(self.settings)
        self.reverb = Reverb(self.settings)
        self.highpass = Highpass(self.settings)
        self.lowpass = Lowpass(self.settings)
        self.equalizer = Equalizer(self.settings)

        # adding effects widgets to the stack

        self.stack.add_titled(self.limiter.ui_window, 'Limiter',
                              _('Input Limiter'))
        self.stack.add_titled(self.panorama.ui_window, 'Panorama',
                              _('Panorama'))
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

        # adding effects to the pipeline

        self.effects_bin.append(self.panorama.bin, self.on_filter_added, None)
        self.effects_bin.append(self.compressor.bin, self.on_filter_added,
                                None)
        self.effects_bin.append(self.reverb.bin, self.on_filter_added, None)
        self.effects_bin.append(self.highpass.bin, self.on_filter_added, None)
        self.effects_bin.append(self.lowpass.bin, self.on_filter_added, None)
        self.effects_bin.append(self.equalizer.bin, self.on_filter_added, None)
        self.effects_bin.append(self.spectrum, self.on_filter_added, None)

    def on_message_element(self, bus, msg):
        plugin = msg.src.get_name()

        if plugin == 'limiter_input_level':
            peak = msg.get_structure().get_value('peak')

            self.limiter.ui_update_limiter_input_level(peak)
        elif plugin == 'limiter_output_level':
            peak = msg.get_structure().get_value('peak')

            self.limiter.ui_update_limiter_output_level(peak)
        elif plugin == 'autovolume':
            if self.limiter.autovolume_enabled:
                peak = msg.get_structure().get_value('peak')

                max_value = max(peak)

                if max_value > self.limiter.autovolume_threshold:
                    self.limiter.auto_gain(max_value)
        elif plugin == 'panorama_input_level':
            peak = msg.get_structure().get_value('peak')

            self.panorama.ui_update_panorama_input_level(peak)
        elif plugin == 'panorama_output_level':
            peak = msg.get_structure().get_value('peak')

            self.panorama.ui_update_panorama_output_level(peak)
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
                                     None)

            self.limiter.ui_autovolume_box.set_sensitive(True)
        else:
            self.effects_bin.remove(self.limiter.bin, self.on_filter_added,
                                    None)

            self.limiter.ui_autovolume_box.set_sensitive(False)

    def init_ui(self):
        self.limiter.init_ui()
        self.panorama.init_ui()
        self.compressor.init_ui()
        self.reverb.init_ui()
        self.highpass.init_ui()
        self.lowpass.init_ui()
        self.equalizer.init_ui()

    def reset(self):
        self.limiter.reset()
        self.panorama.reset()
        self.compressor.reset()
        self.reverb.reset()
        self.highpass.reset()
        self.lowpass.reset()
        self.equalizer.reset()

        self.init_ui()
