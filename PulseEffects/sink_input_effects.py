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

        self.log_tag = 'apps: '
        self.panorama_ready = False

        self.settings = Gio.Settings('com.github.wwmm.pulseeffects.sinkinputs')

        self.builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + '/ui/effects_box.glade')

        self.ui_window = self.builder.get_object('window')
        self.listbox = self.builder.get_object('listbox')
        self.stack = self.builder.get_object('stack')

        self.limiter = Limiter(self.settings)
        self.panorama = Panorama(self.settings)
        self.compressor = Compressor(self.settings)
        self.reverb = Reverb(self.settings)
        self.highpass = Highpass(self.settings)
        self.lowpass = Lowpass(self.settings)
        self.equalizer = Equalizer(self.settings)

        self.add_to_listbox('limiter')
        self.add_to_listbox('panorama')
        self.add_to_listbox('compressor')
        self.add_to_listbox('reverb')
        self.add_to_listbox('highpass')
        self.add_to_listbox('lowpass')
        self.add_to_listbox('equalizer')

        self.listbox.connect('row-activated', self.on_listbox_row_activated)

        self.listbox.show_all()

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
        self.panorama.ui_enable.connect('state-set', self.on_panorama_enable)
        self.compressor.ui_enable.connect('state-set',
                                          self.on_compressor_enable)
        self.reverb.ui_enable.connect('state-set', self.on_reverb_enable)
        self.highpass.ui_enable.connect('state-set', self.on_highpass_enable)
        self.lowpass.ui_enable.connect('state-set', self.on_lowpass_enable)
        self.equalizer.ui_enable.connect('state-set', self.on_equalizer_enable)

        # order is important

        self.limiter.bind()
        self.panorama.bind()
        self.compressor.bind()
        self.reverb.bind()
        self.highpass.bind()
        self.lowpass.bind()
        self.equalizer.bind()

    def add_to_listbox(self, name):
        row = Gtk.ListBoxRow()

        row.add(getattr(self, name).ui_listbox_control)

        row.set_name(name)

        row.set_margin_top(3)
        row.set_margin_bottom(3)

        self.listbox.add(row)

    def on_listbox_row_activated(self, obj, row):
        name = row.get_name()

        if name == 'limiter':
            self.stack.set_visible_child(self.limiter.ui_window)
        elif name == 'panorama':
            self.stack.set_visible_child(self.panorama.ui_window)
        elif name == 'compressor':
            self.stack.set_visible_child(self.compressor.ui_window)
        elif name == 'reverb':
            self.stack.set_visible_child(self.reverb.ui_window)
        elif name == 'highpass':
            self.stack.set_visible_child(self.highpass.ui_window)
        elif name == 'lowpass':
            self.stack.set_visible_child(self.lowpass.ui_window)
        elif name == 'equalizer':
            self.stack.set_visible_child(self.equalizer.ui_window)

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
        self.limiter_ready = False

        if state:
            self.effects_bin.prepend(self.limiter.bin, self.on_limiter_added,
                                     self.log_tag)
        else:
            self.effects_bin.remove(self.limiter.bin, self.on_filter_removed,
                                    self.log_tag)

    def on_panorama_enable(self, obj, state):
        self.panorama_ready = False

        if state:
            limiter_enabled = self.settings.get_value('limiter-state').unpack()

            if limiter_enabled:
                while not self.limiter_ready:
                    pass

                self.effects_bin.insert_after(self.panorama.bin,
                                              self.limiter.bin,
                                              self.on_panorama_added,
                                              self.log_tag)
            else:
                self.effects_bin.prepend(self.panorama.bin,
                                         self.on_panorama_added, self.log_tag)
        else:
            self.effects_bin.remove(self.panorama.bin, self.on_filter_removed,
                                    self.log_tag)

    def on_compressor_enable(self, obj, state):
        self.compressor_ready = False

        if state:
            limiter_enabled = self.settings.get_value('limiter-state').unpack()
            panorama_enabled = self.settings.get_value(
                'panorama-state').unpack()

            if panorama_enabled:
                while not self.panorama_ready:
                    pass

                self.effects_bin.insert_after(self.compressor.bin,
                                              self.panorama.bin,
                                              self.on_compressor_added,
                                              self.log_tag)
            elif limiter_enabled:
                while not self.limiter_ready:
                    pass

                self.effects_bin.insert_after(self.compressor.bin,
                                              self.limiter.bin,
                                              self.on_compressor_added,
                                              self.log_tag)
            else:
                self.effects_bin.prepend(self.compressor.bin,
                                         self.on_compressor_added,
                                         self.log_tag)
        else:
            self.effects_bin.remove(self.compressor.bin,
                                    self.on_filter_removed,
                                    self.log_tag)

    def on_reverb_enable(self, obj, state):
        self.reverb_ready = False

        if state:
            limiter_enabled = self.settings.get_value('limiter-state').unpack()
            panorama_enabled = self.settings.get_value(
                'panorama-state').unpack()
            compressor_enabled = self.settings.get_value(
                'compressor-state').unpack()

            if compressor_enabled:
                while not self.compressor_ready:
                    pass

                self.effects_bin.insert_after(self.reverb.bin,
                                              self.compressor.bin,
                                              self.on_reverb_added,
                                              self.log_tag)
            elif panorama_enabled:
                while not self.panorama_ready:
                    pass

                self.effects_bin.insert_after(self.reverb.bin,
                                              self.panorama.bin,
                                              self.on_reverb_added,
                                              self.log_tag)
            elif limiter_enabled:
                while not self.limiter_ready:
                    pass

                self.effects_bin.insert_after(self.reverb.bin,
                                              self.limiter.bin,
                                              self.on_reverb_added,
                                              self.log_tag)
            else:
                self.effects_bin.prepend(self.reverb.bin,
                                         self.on_reverb_added, self.log_tag)
        else:
            self.effects_bin.remove(self.reverb.bin, self.on_filter_removed,
                                    self.log_tag)

    def on_highpass_enable(self, obj, state):
        self.highpass_ready = False

        if state:
            limiter_enabled = self.settings.get_value('limiter-state').unpack()
            panorama_enabled = self.settings.get_value(
                'panorama-state').unpack()
            compressor_enabled = self.settings.get_value(
                'compressor-state').unpack()
            reverb_enabled = self.settings.get_value(
                'reverb-state').unpack()

            if reverb_enabled:
                while not self.reverb_ready:
                    pass

                self.effects_bin.insert_after(self.highpass.bin,
                                              self.reverb.bin,
                                              self.on_highpass_added,
                                              self.log_tag)
            elif compressor_enabled:
                while not self.compressor_ready:
                    pass

                self.effects_bin.insert_after(self.highpass.bin,
                                              self.compressor.bin,
                                              self.on_highpass_added,
                                              self.log_tag)
            elif panorama_enabled:
                while not self.panorama_ready:
                    pass

                self.effects_bin.insert_after(self.highpass.bin,
                                              self.panorama.bin,
                                              self.on_highpass_added,
                                              self.log_tag)
            elif limiter_enabled:
                while not self.limiter_ready:
                    pass

                self.effects_bin.insert_after(self.highpass.bin,
                                              self.limiter.bin,
                                              self.on_highpass_added,
                                              self.log_tag)
            else:
                self.effects_bin.prepend(self.highpass.bin,
                                         self.on_highpass_added, self.log_tag)
        else:
            self.effects_bin.remove(self.highpass.bin, self.on_filter_removed,
                                    self.log_tag)

    def on_lowpass_enable(self, obj, state):
        self.lowpass_ready = False

        if state:
            limiter_enabled = self.settings.get_value('limiter-state').unpack()
            panorama_enabled = self.settings.get_value(
                'panorama-state').unpack()
            compressor_enabled = self.settings.get_value(
                'compressor-state').unpack()
            reverb_enabled = self.settings.get_value(
                'reverb-state').unpack()
            highpass_enabled = self.settings.get_value(
                'highpass-state').unpack()

            if highpass_enabled:
                while not self.highpass_ready:
                    pass

                self.effects_bin.insert_after(self.lowpass.bin,
                                              self.highpass.bin,
                                              self.on_lowpass_added,
                                              self.log_tag)
            elif reverb_enabled:
                while not self.reverb_ready:
                    pass

                self.effects_bin.insert_after(self.lowpass.bin,
                                              self.reverb.bin,
                                              self.on_lowpass_added,
                                              self.log_tag)
            elif compressor_enabled:
                while not self.compressor_ready:
                    pass

                self.effects_bin.insert_after(self.lowpass.bin,
                                              self.compressor.bin,
                                              self.on_lowpass_added,
                                              self.log_tag)
            elif panorama_enabled:
                while not self.panorama_ready:
                    pass

                self.effects_bin.insert_after(self.lowpass.bin,
                                              self.panorama.bin,
                                              self.on_lowpass_added,
                                              self.log_tag)
            elif limiter_enabled:
                while not self.limiter_ready:
                    pass

                self.effects_bin.insert_after(self.lowpass.bin,
                                              self.limiter.bin,
                                              self.on_lowpass_added,
                                              self.log_tag)
            else:
                self.effects_bin.prepend(self.lowpass.bin,
                                         self.on_lowpass_added, self.log_tag)
        else:
            self.effects_bin.remove(self.lowpass.bin, self.on_filter_removed,
                                    self.log_tag)

    def on_equalizer_enable(self, obj, state):
        self.equalizer_ready = False

        if state:
            limiter_enabled = self.settings.get_value('limiter-state').unpack()
            panorama_enabled = self.settings.get_value(
                'panorama-state').unpack()
            compressor_enabled = self.settings.get_value(
                'compressor-state').unpack()
            reverb_enabled = self.settings.get_value(
                'reverb-state').unpack()
            highpass_enabled = self.settings.get_value(
                'highpass-state').unpack()
            lowpass_enabled = self.settings.get_value(
                'lowpass-state').unpack()

            if lowpass_enabled:
                while not self.lowpass_ready:
                    pass

                self.effects_bin.insert_after(self.equalizer.bin,
                                              self.lowpass.bin,
                                              self.on_equalizer_added,
                                              self.log_tag)
            elif highpass_enabled:
                while not self.highpass_ready:
                    pass

                self.effects_bin.insert_after(self.equalizer.bin,
                                              self.highpass.bin,
                                              self.on_equalizer_added,
                                              self.log_tag)
            elif reverb_enabled:
                while not self.reverb_ready:
                    pass

                self.effects_bin.insert_after(self.equalizer.bin,
                                              self.reverb.bin,
                                              self.on_equalizer_added,
                                              self.log_tag)
            elif compressor_enabled:
                while not self.compressor_ready:
                    pass

                self.effects_bin.insert_after(self.equalizer.bin,
                                              self.compressor.bin,
                                              self.on_equalizer_added,
                                              self.log_tag)
            elif panorama_enabled:
                while not self.panorama_ready:
                    pass

                self.effects_bin.insert_after(self.equalizer.bin,
                                              self.panorama.bin,
                                              self.on_equalizer_added,
                                              self.log_tag)
            elif limiter_enabled:
                while not self.limiter_ready:
                    pass

                self.effects_bin.insert_after(self.equalizer.bin,
                                              self.limiter.bin,
                                              self.on_equalizer_added,
                                              self.log_tag)
            else:
                self.effects_bin.prepend(self.equalizer.bin,
                                         self.on_equalizer_added, self.log_tag)
        else:
            self.effects_bin.remove(self.equalizer.bin, self.on_filter_removed,
                                    self.log_tag)

    def enable_spectrum(self, state):
        if state:
            self.effects_bin.append(self.spectrum, self.on_spectrum_added,
                                    self.log_tag)
        else:
            self.effects_bin.remove(self.spectrum, self.on_filter_removed,
                                    self.log_tag)

    def on_panorama_added(self, bin, element, success, user_data):
        bin_name = element.get_name()
        plugin_name = bin_name.split('_')[0]

        if success:
            self.panorama_ready = True
            self.log.info(user_data + plugin_name + ' plugin was enabled')
        else:
            self.log.critical(user_data + 'failed to enable ' + plugin_name)

    def reset(self):
        self.limiter.reset()
        self.panorama.reset()
        self.compressor.reset()
        self.reverb.reset()
        self.highpass.reset()
        self.lowpass.reset()
        self.equalizer.reset()
