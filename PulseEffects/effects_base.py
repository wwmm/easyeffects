import gettext
import os

import gi
import numpy as np
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GstInsertBin, Gtk, Pango
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

        # adding applications entry
        row = Gtk.ListBoxRow()
        row.set_name('applications')
        row.set_margin_top(6)
        row.set_margin_bottom(6)

        entry_label = Gtk.Label(_('<b>Applications</b>'))
        entry_label.set_halign(Gtk.Align.START)
        entry_label.set_use_markup(True)
        entry_label.set_ellipsize(Pango.EllipsizeMode.END)

        row.add(entry_label)

        self.listbox.add(row)

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
        self.stack.set_visible_child_name(row.get_name())

    def build_app_ui(self, parameters):
        idx = parameters['index']
        app_name = parameters['name']
        icon_name = parameters['icon']
        audio_channels = parameters['channels']
        max_volume_linear = parameters['volume']
        rate = parameters['rate']
        resample_method = parameters['resampler']
        sample_format = parameters['format']
        mute = parameters['mute']
        connected = parameters['connected']

        builder = Gtk.Builder.new_from_file(self.module_path +
                                            '/ui/app_info.glade')

        app_box = builder.get_object('app_box')
        app_icon = builder.get_object('app_icon')
        label_name = builder.get_object('app_name')
        label_format = builder.get_object('format')
        label_rate = builder.get_object('rate')
        label_channels = builder.get_object('channels')
        label_resampler = builder.get_object('resampler')
        switch = builder.get_object('enable')
        volume_scale = builder.get_object('volume_scale')
        mute_button = builder.get_object('mute')
        mute_icon = builder.get_object('mute_icon')

        app_box.set_name('app_box_' + str(idx))

        label_name.set_text(app_name)
        label_format.set_text(sample_format)
        label_channels.set_text(str(audio_channels))
        label_resampler.set_text(resample_method)

        rate_str = '{:.1f}'.format(round(rate / 1000.0, 1)) + ' kHz'
        label_rate.set_text(rate_str)

        app_icon.set_from_icon_name(icon_name, Gtk.IconSize.LARGE_TOOLBAR)

        switch.connect('state-set', self.on_enable_app, idx)
        volume_scale.connect('value-changed', self.on_volume_changed, idx,
                             audio_channels)
        mute_button.connect('toggled', self.on_mute, idx,
                            mute_icon)

        if self.switch_on_all_apps:
            switch.set_active(True)
            switch.set_sensitive(False)
        else:
            switch.set_active(connected)

        volume_scale.set_value(max_volume_linear)
        mute_button.set_active(mute)

        return app_box

    def on_app_added(self, obj, parameters):
        app_box = self.build_app_ui(parameters)

        self.apps_box.add(app_box)
        self.apps_box.show_all()

        if not self.is_playing:
            self.set_state('playing')

    def on_app_changed(self, obj, parameters):
        idx = parameters['index']
        audio_channels = parameters['channels']
        max_volume_linear = parameters['volume']
        rate = parameters['rate']
        resample_method = parameters['resampler']
        sample_format = parameters['format']
        mute = parameters['mute']
        connected = parameters['connected']

        children = self.apps_box.get_children()

        for child in children:
            if child.get_name() == 'app_box_' + str(idx):
                for node in child.get_children():
                    node_name = node.get_name()

                    if node_name == 'switch':
                        node.set_active(connected)
                    elif node_name == 'volume':
                        node.set_value(max_volume_linear)

                        if mute:
                            node.set_sensitive(False)
                        else:
                            node.set_sensitive(True)
                    elif node_name == 'mute':
                        node.set_active(mute)
                    elif node_name == 'stream_props':
                        for label in node.get_children():
                            label_name = label.get_name()

                            if label_name == 'format':
                                label.set_text(sample_format)
                            elif label_name == 'rate':
                                rate_str = '{:.1f}'.format(round(
                                    rate / 1000.0, 1)) + ' kHz'

                                label.set_text(rate_str)
                            elif label_name == 'channels':
                                label.set_text(str(audio_channels))
                            elif label_name == 'resampler':
                                label.set_text(resample_method)

    def on_app_removed(self, obj, idx):
        children = self.apps_box.get_children()

        n_children_before = len(children)

        for child in children:
            child_name = child.get_name()

            if child_name == 'app_box_' + str(idx):
                self.apps_box.remove(child)
                break

        n_children_after = len(self.apps_box.get_children())

        if n_children_before == 1 and n_children_after == 0:
            self.set_state('ready')

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

    def reset(self):
        self.limiter.reset()
        self.compressor.reset()
        self.reverb.reset()
        self.highpass.reset()
        self.lowpass.reset()
        self.equalizer.reset()
