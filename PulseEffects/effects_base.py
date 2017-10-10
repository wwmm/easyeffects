import gettext
import os

import gi
import numpy as np
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GstInsertBin, Gtk
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


class EffectsBase(PipelineBase):

    def __init__(self, sampling_rate, settings):
        PipelineBase.__init__(self, sampling_rate)

        self.module_path = os.path.dirname(__file__)
        self.settings = settings
        self.log_tag = str()

        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/effects_box.glade')

        self.ui_window = self.builder.get_object('window')
        self.listbox = self.builder.get_object('listbox')
        self.stack = self.builder.get_object('stack')
        self.apps_box = self.builder.get_object('apps_box')

        self.listbox.connect('row-activated', self.on_listbox_row_activated)

        # listbox style
        provider = Gtk.CssProvider()

        css_file = Gio.File.new_for_path(self.module_path + '/ui/listbox.css')

        provider.load_from_file(css_file)

        Gtk.StyleContext.add_provider(self.listbox.get_style_context(),
                                      provider,
                                      Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.limiter = Limiter(self.settings)
        self.compressor = Compressor(self.settings)
        self.reverb = Reverb(self.settings)
        self.highpass = Highpass(self.settings)
        self.lowpass = Lowpass(self.settings)
        self.equalizer = Equalizer(self.settings)

        self.add_to_listbox('limiter')
        self.add_to_listbox('compressor')
        self.add_to_listbox('reverb')
        self.add_to_listbox('highpass')
        self.add_to_listbox('lowpass')
        self.add_to_listbox('equalizer')

        # on/off switches connections
        self.limiter.ui_limiter_enable.connect('state-set',
                                               self.on_limiter_enable)

        # effects wrappers
        self.limiter_wrapper = GstInsertBin.InsertBin.new('limiter_wrapper')
        self.compressor_wrapper = GstInsertBin.InsertBin.new(
            'compressor_wrapper')
        self.reverb_wrapper = GstInsertBin.InsertBin.new('reverb_wrapper')
        self.highpass_wrapper = GstInsertBin.InsertBin.new('highpass_wrapper')
        self.lowpass_wrapper = GstInsertBin.InsertBin.new('lowpass_wrapper')
        self.equalizer_wrapper = GstInsertBin.InsertBin.new(
            'equalizer_wrapper')
        self.spectrum_wrapper = GstInsertBin.InsertBin.new(
            'spectrum_wrapper')

        # appending effects wrappers to effects bin
        self.effects_bin.append(self.limiter_wrapper, self.on_filter_added,
                                self.log_tag)
        self.effects_bin.append(self.compressor_wrapper, self.on_filter_added,
                                self.log_tag)
        self.effects_bin.append(self.reverb_wrapper, self.on_filter_added,
                                self.log_tag)
        self.effects_bin.append(self.highpass_wrapper, self.on_filter_added,
                                self.log_tag)
        self.effects_bin.append(self.lowpass_wrapper, self.on_filter_added,
                                self.log_tag)
        self.effects_bin.append(self.equalizer_wrapper, self.on_filter_added,
                                self.log_tag)
        self.effects_bin.append(self.spectrum_wrapper, self.on_filter_added,
                                self.log_tag)

    def add_to_listbox(self, name):
        row = Gtk.ListBoxRow()

        row.add(getattr(self, name).ui_listbox_control)

        row.set_name(name)

        row.set_margin_top(6)
        row.set_margin_bottom(6)

        self.listbox.add(row)

    def insert_in_listbox(self, name, idx):
        row = Gtk.ListBoxRow()

        row.add(getattr(self, name).ui_listbox_control)

        row.set_name(name)

        row.set_margin_top(6)
        row.set_margin_bottom(6)

        self.listbox.insert(row, idx)

    def on_listbox_row_activated(self, obj, row):
        name = row.get_name()

        if name == 'limiter':
            self.stack.set_visible_child(self.limiter.ui_window)
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
            self.limiter_wrapper.append(self.limiter.bin,
                                        self.on_filter_added,
                                        self.log_tag)
        else:
            self.limiter_wrapper.remove(self.limiter.bin,
                                        self.on_filter_removed,
                                        self.log_tag)

    def on_compressor_enable(self, obj, state):
        if state:
            self.compressor_wrapper.append(self.compressor.bin,
                                           self.on_filter_added,
                                           self.log_tag)
        else:
            self.compressor_wrapper.remove(self.compressor.bin,
                                           self.on_filter_removed,
                                           self.log_tag)

    def on_reverb_enable(self, obj, state):
        if state:
            self.reverb_wrapper.append(self.reverb.bin,
                                       self.on_filter_added,
                                       self.log_tag)
        else:
            self.reverb_wrapper.remove(self.reverb.bin,
                                       self.on_filter_removed,
                                       self.log_tag)

    def on_highpass_enable(self, obj, state):
        if state:
            self.highpass_wrapper.append(self.highpass.bin,
                                         self.on_filter_added,
                                         self.log_tag)
        else:
            self.highpass_wrapper.remove(self.highpass.bin,
                                         self.on_filter_removed,
                                         self.log_tag)

    def on_lowpass_enable(self, obj, state):
        if state:
            self.lowpass_wrapper.append(self.lowpass.bin,
                                        self.on_filter_added,
                                        self.log_tag)
        else:
            self.lowpass_wrapper.remove(self.lowpass.bin,
                                        self.on_filter_removed,
                                        self.log_tag)

    def on_equalizer_enable(self, obj, state):
        if state:
            self.equalizer_wrapper.append(self.equalizer.bin,
                                          self.on_filter_added,
                                          self.log_tag)
        else:
            self.equalizer_wrapper.remove(self.equalizer.bin,
                                          self.on_filter_removed,
                                          self.log_tag)

    def enable_spectrum(self, state):
        if state:
            self.spectrum_wrapper.append(self.spectrum,
                                         self.on_filter_added,
                                         self.log_tag)
        else:
            self.spectrum_wrapper.remove(self.spectrum,
                                         self.on_filter_removed,
                                         self.log_tag)
