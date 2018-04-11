# -*- coding: utf-8 -*-

import os
from gettext import gettext as _

import gi
import numpy as np
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import GstInsertBin, Gtk, Pango
from PulseEffects.compressor import Compressor
from PulseEffects.equalizer import Equalizer
from PulseEffects.highpass import Highpass
from PulseEffects.limiter import Limiter
from PulseEffects.lowpass import Lowpass
from PulseEffects.pipeline_base import PipelineBase
from PulseEffects.reverb import Reverb
from scipy.interpolate import CubicSpline


class EffectsBase(PipelineBase):

    def __init__(self, sampling_rate, settings):
        PipelineBase.__init__(self, sampling_rate)

        self.module_path = os.path.dirname(__file__)
        self.settings = settings

        self.disable_app_level_meter = False
        self.there_is_window = False

        # [[idx, state], [idx, state],...]
        # state tells if the app wants the pipeline to be running
        self.apps_list = []

        self.limiter = Limiter()
        self.compressor = Compressor(self.settings)
        self.highpass = Highpass(self.settings)
        self.lowpass = Lowpass(self.settings)
        self.equalizer = Equalizer(self.settings)
        self.reverb = Reverb(self.settings)

        # effects wrappers
        self.limiter_wrapper = GstInsertBin.InsertBin.new('limiter_wrapper')
        self.compressor_wrapper = GstInsertBin.InsertBin.new(
            'compressor_wrapper')
        self.highpass_wrapper = GstInsertBin.InsertBin.new('highpass_wrapper')
        self.lowpass_wrapper = GstInsertBin.InsertBin.new('lowpass_wrapper')
        self.equalizer_wrapper = GstInsertBin.InsertBin.new(
            'equalizer_wrapper')
        self.reverb_wrapper = GstInsertBin.InsertBin.new('reverb_wrapper')
        self.spectrum_wrapper = GstInsertBin.InsertBin.new(
            'spectrum_wrapper')

    def init_ui(self):
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

        entry_label = Gtk.Label('<b>' + _('Applications') + '</b>')
        entry_label.set_halign(Gtk.Align.START)
        entry_label.set_use_markup(True)
        entry_label.set_ellipsize(Pango.EllipsizeMode.END)

        row.add(entry_label)

        self.listbox.add(row)
        self.listbox.connect('row-activated', self.on_listbox_row_activated)

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
        buffer_latency = parameters['buffer_latency']
        latency = parameters['latency']
        corked = parameters['corked']

        builder = Gtk.Builder.new_from_file(self.module_path +
                                            '/ui/app_info.glade')

        app_icon = builder.get_object('app_icon')
        mute_icon = builder.get_object('mute_icon')

        setattr(self, 'app_box_' + str(idx),
                builder.get_object('app_box'))
        setattr(self, 'app_name_' + str(idx),
                builder.get_object('app_name'))
        setattr(self, 'app_format_' + str(idx),
                builder.get_object('format'))
        setattr(self, 'app_rate_' + str(idx),
                builder.get_object('rate'))
        setattr(self, 'app_channels_' + str(idx),
                builder.get_object('channels'))
        setattr(self, 'app_resampler_' + str(idx),
                builder.get_object('resampler'))
        setattr(self, 'app_buffer_' + str(idx),
                builder.get_object('buffer_latency'))
        setattr(self, 'app_latency_' + str(idx),
                builder.get_object('latency'))
        setattr(self, 'app_state_' + str(idx),
                builder.get_object('state'))
        setattr(self, 'app_switch_' + str(idx),
                builder.get_object('enable'))
        setattr(self, 'app_volume_' + str(idx),
                builder.get_object('volume_scale'))
        setattr(self, 'app_mute_' + str(idx),
                builder.get_object('mute'))
        setattr(self, 'app_level_' + str(idx),
                builder.get_object('level'))

        # there is no point in showing it for source outputs
        # their class disables the level
        if self.disable_app_level_meter:
            getattr(self, 'app_level_' + str(idx)).destroy()

        getattr(self, 'app_name_' + str(idx)).set_text(app_name)
        getattr(self, 'app_format_' + str(idx)).set_text(sample_format)
        getattr(self, 'app_channels_' + str(idx)).set_text(str(audio_channels))
        getattr(self, 'app_resampler_' + str(idx)).set_text(resample_method)

        rate_str = '{:.1f}'.format(round(rate / 1000.0, 1)) + ' kHz'
        getattr(self, 'app_rate_' + str(idx)).set_text(rate_str)

        buffer_str = '{:.1f}'.format(round(buffer_latency / 1000.0, 1)) + ' ms'
        getattr(self, 'app_buffer_' + str(idx)).set_text(buffer_str)

        latency_str = '{:.1f}'.format(round(latency / 1000.0, 1)) + ' ms'
        getattr(self, 'app_latency_' + str(idx)).set_text(latency_str)

        if corked:
            getattr(self, 'app_state_' + str(idx)).set_text(_('paused'))
        else:
            getattr(self, 'app_state_' + str(idx)).set_text(_('playing'))

        app_icon.set_from_icon_name(icon_name, Gtk.IconSize.LARGE_TOOLBAR)

        # connecting signals
        getattr(self, 'app_switch_' + str(idx)).connect('state-set',
                                                        self.on_enable_app,
                                                        idx)
        getattr(self, 'app_volume_' + str(idx)).connect('value-changed',
                                                        self.on_volume_changed,
                                                        idx,
                                                        audio_channels)
        getattr(self, 'app_mute_' + str(idx)).connect('toggled', self.on_mute,
                                                      idx,
                                                      mute_icon)

        if self.switch_on_all_apps:
            getattr(self, 'app_switch_' + str(idx)).set_active(True)
        else:
            getattr(self, 'app_switch_' + str(idx)).set_active(connected)

        getattr(self, 'app_volume_' + str(idx)).set_value(max_volume_linear)

        getattr(self, 'app_mute_' + str(idx)).set_active(mute)

        self.apps_box.add(getattr(self, 'app_box_' + str(idx)))
        self.apps_box.show_all()

    def change_app_ui(self, parameters):
        idx = parameters['index']
        audio_channels = parameters['channels']
        max_volume_linear = parameters['volume']
        rate = parameters['rate']
        resample_method = parameters['resampler']
        sample_format = parameters['format']
        mute = parameters['mute']
        connected = parameters['connected']
        buffer_latency = parameters['buffer_latency']
        latency = parameters['latency']
        corked = parameters['corked']

        if hasattr(self, 'app_format_' + str(idx)):
            getattr(self, 'app_format_' + str(idx)).set_text(sample_format)

        if hasattr(self, 'app_channels_' + str(idx)):
            getattr(self, 'app_channels_' + str(idx)).set_text(str(
                audio_channels))

        if hasattr(self, 'app_resampler_' + str(idx)):
            getattr(self, 'app_resampler_' + str(idx)).set_text(
                resample_method)

        if hasattr(self, 'app_rate_' + str(idx)):
            rate_str = '{:.1f}'.format(round(rate / 1000.0, 1)) + ' kHz'
            getattr(self, 'app_rate_' + str(idx)).set_text(rate_str)

        if hasattr(self, 'app_buffer_' + str(idx)):
            buffer_str = '{:.1f}'.format(round(buffer_latency / 1000.0, 1))
            buffer_str += ' ms'
            getattr(self, 'app_buffer_' + str(idx)).set_text(buffer_str)

        if hasattr(self, 'app_latency_' + str(idx)):
            latency_str = '{:.1f}'.format(round(latency / 1000.0, 1)) + ' ms'
            getattr(self, 'app_latency_' + str(idx)).set_text(latency_str)

        if hasattr(self, 'app_state_' + str(idx)):
            if corked:
                getattr(self, 'app_state_' + str(idx)).set_text(_('paused'))
            else:
                getattr(self, 'app_state_' + str(idx)).set_text(_('playing'))

        if hasattr(self, 'app_switch_' + str(idx)):
            getattr(self, 'app_switch_' + str(idx)).set_active(connected)

        if hasattr(self, 'app_volume_' + str(idx)):
            volume = getattr(self, 'app_volume_' + str(idx))

            volume.set_value(max_volume_linear)

            if mute:
                volume.set_sensitive(False)
            else:
                volume.set_sensitive(True)

        if hasattr(self, 'app_mute_' + str(idx)):
            getattr(self, 'app_mute_' + str(idx)).set_active(mute)

    def remove_app_ui(self, idx):
        if hasattr(self, 'app_box_' + str(idx)):
            self.apps_box.remove(getattr(self, 'app_box_' + str(idx)))

            delattr(self, 'app_box_' + str(idx))
            delattr(self, 'app_name_' + str(idx))
            delattr(self, 'app_format_' + str(idx))
            delattr(self, 'app_rate_' + str(idx))
            delattr(self, 'app_channels_' + str(idx))
            delattr(self, 'app_resampler_' + str(idx))
            delattr(self, 'app_buffer_' + str(idx))
            delattr(self, 'app_latency_' + str(idx))
            delattr(self, 'app_state_' + str(idx))
            delattr(self, 'app_switch_' + str(idx))
            delattr(self, 'app_volume_' + str(idx))
            delattr(self, 'app_mute_' + str(idx))
            delattr(self, 'app_level_' + str(idx))

    def set_pipeline_state(self):
        # Deciding if GStreamer pipeline should be put in the playing state
        # based on the apps state. If no app is playing the GStreamer pipeline
        # should also be in the paused state in order to not waste cpu

        pipeline_state = 'ready'

        for a in self.apps_list:
            if a[1]:
                pipeline_state = 'playing'

                break

        self.set_state(pipeline_state)

    def on_app_added(self, obj, parameters):
        idx = parameters['index']
        corked = parameters['corked']  # cork = 1 means that the app is paused
        connected = parameters['connected']  # app switch state

        if self.there_is_window:
            self.build_app_ui(parameters)
        elif self.switch_on_all_apps:
            # necessary when running as service with the window closed
            self.on_enable_app(None, True, idx)

        # checking if this app is already on the list
        # if it isn't we add it

        add_to_list = True

        for a in self.apps_list:
            if a[0] == idx:
                add_to_list = False

                break

        if add_to_list:
            state = False

            # app wants the pipeline to be running only if it is connected
            # and not corked

            if connected and not corked:
                state = True

            self.apps_list.append([idx, state])

        self.set_pipeline_state()

    def on_app_changed(self, obj, parameters):
        if self.there_is_window:
            self.change_app_ui(parameters)

        idx = parameters['index']
        corked = parameters['corked']
        connected = parameters['connected']

        # updating app state if it is in app_list

        for a in self.apps_list:
            if a[0] == idx:
                state = False

                if connected and not corked:
                    state = True

                a[1] = state

                break

        self.set_pipeline_state()

    def on_app_removed(self, obj, idx):
        for a in self.apps_list:
            if a[0] == idx:
                self.remove_app_ui(idx)
                self.apps_list.remove(a)

                self.set_pipeline_state()

                break

    def on_app_level_changed(self, obj, idx, level):
        if hasattr(self, 'app_level_' + str(idx)):
            getattr(self, 'app_level_' + str(idx)).set_value(level)

    def post_messages(self, state):
        self.limiter.post_messages(state)
        self.compressor.post_messages(state)
        self.highpass.post_messages(state)
        self.lowpass.post_messages(state)
        self.equalizer.post_messages(state)
        self.reverb.post_messages(state)

        self.spectrum.set_property('post-messages', state)

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

            self.compressor.ui_update_input_level(peak)
        elif plugin == 'compressor_output_level':
            peak = msg.get_structure().get_value('peak')

            self.compressor.ui_update_output_level(peak)
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
        elif plugin == 'reverb_input_level':
            peak = msg.get_structure().get_value('peak')

            self.reverb.ui_update_reverb_input_level(peak)
        elif plugin == 'reverb_output_level':
            peak = msg.get_structure().get_value('peak')

            self.reverb.ui_update_reverb_output_level(peak)
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
            if not self.limiter_wrapper.get_by_name('limiter_bin'):
                self.limiter_wrapper.append(self.limiter.bin,
                                            self.on_filter_added,
                                            self.log_tag)
        else:
            self.limiter_wrapper.remove(self.limiter.bin,
                                        self.on_filter_removed,
                                        self.log_tag)

    def on_compressor_enable(self, obj, state):
        if state:
            if not self.compressor_wrapper.get_by_name('compressor_bin'):
                self.compressor_wrapper.append(self.compressor.bin,
                                               self.on_filter_added,
                                               self.log_tag)
        else:
            self.compressor_wrapper.remove(self.compressor.bin,
                                           self.on_filter_removed,
                                           self.log_tag)

    def on_highpass_enable(self, obj, state):
        if state:
            if not self.highpass_wrapper.get_by_name('highpass_bin'):
                self.highpass_wrapper.append(self.highpass.bin,
                                             self.on_filter_added,
                                             self.log_tag)
        else:
            self.highpass_wrapper.remove(self.highpass.bin,
                                         self.on_filter_removed,
                                         self.log_tag)

    def on_lowpass_enable(self, obj, state):
        if state:
            if not self.lowpass_wrapper.get_by_name('lowpass_bin'):
                self.lowpass_wrapper.append(self.lowpass.bin,
                                            self.on_filter_added,
                                            self.log_tag)
        else:
            self.lowpass_wrapper.remove(self.lowpass.bin,
                                        self.on_filter_removed,
                                        self.log_tag)

    def on_equalizer_enable(self, obj, state):
        if state:
            if not self.equalizer_wrapper.get_by_name('equalizer_bin'):
                self.equalizer_wrapper.append(self.equalizer.bin,
                                              self.on_filter_added,
                                              self.log_tag)
        else:
            self.equalizer_wrapper.remove(self.equalizer.bin,
                                          self.on_filter_removed,
                                          self.log_tag)

    def on_reverb_enable(self, obj, state):
        if state:
            if not self.reverb_wrapper.get_by_name('reverb_bin'):
                self.reverb_wrapper.append(self.reverb.bin,
                                           self.on_filter_added,
                                           self.log_tag)
        else:
            self.reverb_wrapper.remove(self.reverb.bin,
                                       self.on_filter_removed,
                                       self.log_tag)

    def enable_spectrum(self, state):
        if state:
            if not self.spectrum_wrapper.get_by_name('spectrum'):
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
        self.highpass.reset()
        self.lowpass.reset()
        self.equalizer.reset()
        self.reverb.reset()
