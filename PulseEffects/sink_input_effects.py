# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GstInsertBin, Gtk
from PulseEffects.bass_enhancer import BassEnhancer
from PulseEffects.effects_base import EffectsBase
from PulseEffects.exciter import Exciter
from PulseEffects.maximizer import Maximizer
from PulseEffects.output_limiter import OutputLimiter
from PulseEffects.panorama import Panorama
from PulseEffects.stereo_enhancer import StereoEnhancer
from PulseEffects.stereo_spread import StereoSpread


class SinkInputEffects(EffectsBase):

    def __init__(self, pulse_manager):
        self.pm = pulse_manager
        self.settings = Gio.Settings('com.github.wwmm.pulseeffects.sinkinputs')

        EffectsBase.__init__(self, self.pm.default_sink_rate, self.settings)

        self.log_tag = 'apps: '

        self.set_source_monitor_name(self.pm.apps_sink_monitor_name)

        pulse_sink = os.environ.get('PULSE_SINK')

        if pulse_sink:
            self.set_output_sink_name(pulse_sink)

            self.log.info('$PULSE_SINK = ' + pulse_sink)

            msg = 'user has $PULSE_SINK set. Using it as output device'

            self.log.info(msg)
        else:
            self.set_output_sink_name(self.pm.default_sink_name)

        self.pm.connect('sink_input_added', self.on_app_added)
        self.pm.connect('sink_input_changed', self.on_app_changed)
        self.pm.connect('sink_input_removed', self.on_app_removed)
        self.pm.connect('sink_input_level_changed', self.on_app_level_changed)
        self.pm.connect('new_default_sink', self.update_output_sink_name)

        self.exciter = Exciter(self.settings)
        self.bass_enhancer = BassEnhancer(self.settings)
        self.stereo_enhancer = StereoEnhancer(self.settings)
        self.stereo_spread = StereoSpread(self.settings)
        self.panorama = Panorama(self.settings)
        self.maximizer = Maximizer(self.settings)
        self.output_limiter = OutputLimiter(self.settings)

        # effects wrappers
        self.exciter_wrapper = GstInsertBin.InsertBin.new('exciter_wrapper')
        self.bass_enhancer_wrapper = GstInsertBin.InsertBin.new(
            'bass_enhancer_wrapper')
        self.stereo_enhancer_wrapper = GstInsertBin.InsertBin.new(
            'stereo_enhancer_wrapper')
        self.stereo_spread_wrapper = GstInsertBin.InsertBin.new(
            'stereo_spread_wrapper')
        self.panorama_wrapper = GstInsertBin.InsertBin.new('panorama_wrapper')
        self.maximizer_wrapper = GstInsertBin.InsertBin.new(
            'maximizer_wrapper')
        self.output_limiter_wrapper = GstInsertBin.InsertBin.new(
            'output_limiter_wrapper')

        # appending effects wrappers to effects bin
        self.effects_bin.insert_after(self.exciter_wrapper,
                                      self.equalizer_wrapper,
                                      self.on_filter_added,
                                      self.log_tag)

        self.effects_bin.insert_after(self.bass_enhancer_wrapper,
                                      self.exciter_wrapper,
                                      self.on_filter_added,
                                      self.log_tag)

        self.effects_bin.insert_after(self.stereo_enhancer_wrapper,
                                      self.bass_enhancer_wrapper,
                                      self.on_filter_added,
                                      self.log_tag)

        self.effects_bin.insert_after(self.stereo_spread_wrapper,
                                      self.stereo_enhancer_wrapper,
                                      self.on_filter_added,
                                      self.log_tag)

        self.effects_bin.insert_after(self.panorama_wrapper,
                                      self.reverb_wrapper,
                                      self.on_filter_added,
                                      self.log_tag)

        self.effects_bin.insert_after(self.maximizer_wrapper,
                                      self.panorama_wrapper,
                                      self.on_filter_added,
                                      self.log_tag)

        self.effects_bin.insert_before(self.output_limiter_wrapper,
                                       self.spectrum_wrapper,
                                       self.on_filter_added,
                                       self.log_tag)

    def init_ui(self):
        EffectsBase.init_ui(self)

        self.limiter.init_ui()
        self.compressor.init_ui()
        self.highpass.init_ui()
        self.lowpass.init_ui()
        self.equalizer.init_ui()
        self.exciter.init_ui()
        self.bass_enhancer.init_ui()
        self.stereo_enhancer.init_ui()
        self.stereo_spread.init_ui()
        self.reverb.init_ui()
        self.panorama.init_ui()
        self.maximizer.init_ui()
        self.output_limiter.init_ui()

        self.add_to_listbox('limiter')
        self.add_to_listbox('compressor')
        self.add_to_listbox('highpass')
        self.add_to_listbox('lowpass')
        self.add_to_listbox('equalizer')
        self.add_to_listbox('exciter')
        self.add_to_listbox('bass_enhancer')
        self.add_to_listbox('stereo_enhancer')
        self.add_to_listbox('stereo_spread')
        self.add_to_listbox('reverb')
        self.add_to_listbox('panorama')
        self.add_to_listbox('maximizer')
        self.add_to_listbox('output_limiter')

        self.listbox.show_all()

        # adding effects widgets to the stack

        self.stack.add_named(self.limiter.ui_window, 'limiter')
        self.stack.add_named(self.compressor.ui_window, 'compressor')
        self.stack.add_named(self.highpass.ui_window, 'highpass')
        self.stack.add_named(self.lowpass.ui_window, 'lowpass')
        self.stack.add_named(self.equalizer.ui_window, 'equalizer')
        self.stack.add_named(self.exciter.ui_window, 'exciter')
        self.stack.add_named(self.bass_enhancer.ui_window, 'bass_enhancer')
        self.stack.add_named(self.stereo_enhancer.ui_window, 'stereo_enhancer')
        self.stack.add_named(self.stereo_spread.ui_window, 'stereo_spread')
        self.stack.add_named(self.reverb.ui_window, 'reverb')
        self.stack.add_named(self.panorama.ui_window, 'panorama')
        self.stack.add_named(self.maximizer.ui_window, 'maximizer')
        self.stack.add_named(self.output_limiter.ui_window, 'output_limiter')

        # on/off switches connections
        self.limiter.ui_limiter_enable.connect('state-set',
                                               self.on_limiter_enable)
        self.compressor.ui_enable.connect('state-set',
                                          self.on_compressor_enable)
        self.highpass.ui_enable.connect('state-set', self.on_highpass_enable)
        self.lowpass.ui_enable.connect('state-set', self.on_lowpass_enable)
        self.equalizer.ui_enable.connect('state-set', self.on_equalizer_enable)
        self.exciter.ui_enable.connect('state-set', self.on_exciter_enable)
        self.bass_enhancer.ui_enable.connect('state-set',
                                             self.on_bass_enhancer_enable)
        self.stereo_enhancer.ui_enable.connect('state-set',
                                               self.on_stereo_enhancer_enable)
        self.stereo_spread.ui_enable.connect('state-set',
                                             self.on_stereo_spread_enable)
        self.reverb.ui_enable.connect('state-set', self.on_reverb_enable)
        self.panorama.ui_enable.connect('state-set', self.on_panorama_enable)
        self.maximizer.ui_enable.connect('state-set', self.on_maximizer_enable)
        self.output_limiter.ui_limiter_enable\
            .connect('state-set', self.on_output_limiter_enable)

        if self.limiter.is_installed:
            self.limiter.bind()
        else:
            self.limiter.ui_window.set_sensitive(False)
            self.limiter.ui_limiter_enable.set_sensitive(False)
            self.limiter.ui_img_state.hide()

        if self.compressor.is_installed:
            self.compressor.bind()
        else:
            self.compressor.ui_window.set_sensitive(False)
            self.compressor.ui_enable.set_sensitive(False)
            self.compressor.ui_img_state.hide()

        if self.exciter.is_installed:
            self.exciter.bind()
        else:
            self.exciter.ui_window.set_sensitive(False)
            self.exciter.ui_enable.set_sensitive(False)
            self.exciter.ui_img_state.hide()

        if self.bass_enhancer.is_installed:
            self.bass_enhancer.bind()
        else:
            self.bass_enhancer.ui_window.set_sensitive(False)
            self.bass_enhancer.ui_enable.set_sensitive(False)
            self.bass_enhancer.ui_img_state.hide()

        if self.stereo_enhancer.is_installed:
            self.stereo_enhancer.bind()
        else:
            self.stereo_enhancer.ui_window.set_sensitive(False)
            self.stereo_enhancer.ui_enable.set_sensitive(False)
            self.stereo_enhancer.ui_img_state.hide()

        if self.stereo_spread.is_installed:
            self.stereo_spread.bind()
        else:
            self.stereo_spread.ui_window.set_sensitive(False)
            self.stereo_spread.ui_enable.set_sensitive(False)
            self.stereo_spread.ui_img_state.hide()

        if self.maximizer.is_installed:
            self.maximizer.bind()
        else:
            self.maximizer.ui_window.set_sensitive(False)
            self.maximizer.ui_enable.set_sensitive(False)
            self.maximizer.ui_img_state.hide()

        if self.output_limiter.is_installed:
            self.output_limiter.bind()
        else:
            self.output_limiter.ui_window.set_sensitive(False)
            self.output_limiter.ui_limiter_enable.set_sensitive(False)
            self.output_limiter.ui_img_state.hide()

        self.highpass.bind()
        self.lowpass.bind()
        self.equalizer.bind()
        self.reverb.bind()
        self.panorama.bind()

    def on_enable_app(self, obj, state, idx):
        if state:
            self.pm.move_sink_input_to_pulseeffects_sink(idx)
        else:
            self.pm.move_sink_input_to_default_sink(idx)

    def on_volume_changed(self, obj, idx, audio_channels):
        self.pm.set_sink_input_volume(idx, audio_channels, obj.get_value())

    def on_mute(self, button, idx, icon):
        state = button.get_active()

        if state:
            icon_name = 'audio-volume-muted-symbolic'
            icon.set_from_icon_name(icon_name, Gtk.IconSize.BUTTON)
        else:
            icon_name = 'audio-volume-high-symbolic'
            icon.set_from_icon_name(icon_name, Gtk.IconSize.BUTTON)

        self.pm.set_sink_input_mute(idx, state)

    def post_messages(self, state):
        EffectsBase.post_messages(self, state)

        self.exciter.post_messages(state)
        self.bass_enhancer.post_messages(state)
        self.stereo_enhancer.post_messages(state)
        self.stereo_spread.post_messages(state)
        self.panorama.post_messages(state)
        self.maximizer.post_messages(state)
        self.output_limiter.post_messages(state)

    def on_message_element(self, bus, msg):
        EffectsBase.on_message_element(self, bus, msg)

        plugin = msg.src.get_name()

        if plugin == 'exciter_input_level':
            peak = msg.get_structure().get_value('peak')

            self.exciter.ui_update_input_level(peak)
        elif plugin == 'exciter_output_level':
            peak = msg.get_structure().get_value('peak')

            self.exciter.ui_update_output_level(peak)
        elif plugin == 'bass_enhancer_input_level':
            peak = msg.get_structure().get_value('peak')

            self.bass_enhancer.ui_update_input_level(peak)
        elif plugin == 'bass_enhancer_output_level':
            peak = msg.get_structure().get_value('peak')

            self.bass_enhancer.ui_update_output_level(peak)
        elif plugin == 'stereo_enhancer_input_level':
            peak = msg.get_structure().get_value('peak')

            self.stereo_enhancer.ui_update_input_level(peak)
        elif plugin == 'stereo_enhancer_output_level':
            peak = msg.get_structure().get_value('peak')

            self.stereo_enhancer.ui_update_output_level(peak)
        elif plugin == 'stereo_spread_input_level':
            peak = msg.get_structure().get_value('peak')

            self.stereo_spread.ui_update_input_level(peak)
        elif plugin == 'stereo_spread_output_level':
            peak = msg.get_structure().get_value('peak')

            self.stereo_spread.ui_update_output_level(peak)
        elif plugin == 'panorama_input_level':
            peak = msg.get_structure().get_value('peak')

            self.panorama.ui_update_panorama_input_level(peak)
        elif plugin == 'panorama_output_level':
            peak = msg.get_structure().get_value('peak')

            self.panorama.ui_update_panorama_output_level(peak)
        elif plugin == 'maximizer_input_level':
            peak = msg.get_structure().get_value('peak')

            self.maximizer.ui_update_input_level(peak)
        elif plugin == 'maximizer_output_level':
            peak = msg.get_structure().get_value('peak')

            self.maximizer.ui_update_output_level(peak)
        elif plugin == 'output_limiter_input_level':
            peak = msg.get_structure().get_value('peak')

            self.output_limiter.ui_update_limiter_input_level(peak)
        elif plugin == 'output_limiter_output_level':
            peak = msg.get_structure().get_value('peak')

            self.output_limiter.ui_update_limiter_output_level(peak)

        return True

    def on_panorama_enable(self, obj, state):
        if state:
            if not self.panorama_wrapper.get_by_name('panorama_bin'):
                self.panorama_wrapper.append(self.panorama.bin,
                                             self.on_filter_added,
                                             self.log_tag)
        else:
            self.panorama_wrapper.remove(self.panorama.bin,
                                         self.on_filter_removed,
                                         self.log_tag)

    def on_exciter_enable(self, obj, state):
        if state:
            if not self.exciter_wrapper.get_by_name('exciter_bin'):
                self.exciter_wrapper.append(self.exciter.bin,
                                            self.on_filter_added,
                                            self.log_tag)
        else:
            self.exciter_wrapper.remove(self.exciter.bin,
                                        self.on_filter_removed,
                                        self.log_tag)

    def on_bass_enhancer_enable(self, obj, state):
        if state:
            if not self.bass_enhancer_wrapper.get_by_name('bass_enhancer_bin'):
                self.bass_enhancer_wrapper.append(self.bass_enhancer.bin,
                                                  self.on_filter_added,
                                                  self.log_tag)
        else:
            self.bass_enhancer_wrapper.remove(self.bass_enhancer.bin,
                                              self.on_filter_removed,
                                              self.log_tag)

    def on_stereo_enhancer_enable(self, obj, state):
        if state:
            if not self.stereo_enhancer_wrapper.get_by_name(
                    'stereo_enhancer_bin'):
                self.stereo_enhancer_wrapper.append(self.stereo_enhancer.bin,
                                                    self.on_filter_added,
                                                    self.log_tag)
        else:
            self.stereo_enhancer_wrapper.remove(self.stereo_enhancer.bin,
                                                self.on_filter_removed,
                                                self.log_tag)

    def on_stereo_spread_enable(self, obj, state):
        if state:
            if not self.stereo_spread_wrapper.get_by_name('stereo_spread_bin'):
                self.stereo_spread_wrapper.append(self.stereo_spread.bin,
                                                  self.on_filter_added,
                                                  self.log_tag)
        else:
            self.stereo_spread_wrapper.remove(self.stereo_spread.bin,
                                              self.on_filter_removed,
                                              self.log_tag)

    def on_maximizer_enable(self, obj, state):
        if state:
            if not self.maximizer_wrapper.get_by_name('maximizer_bin'):
                self.maximizer_wrapper.append(self.maximizer.bin,
                                              self.on_filter_added,
                                              self.log_tag)
        else:
            self.maximizer_wrapper.remove(self.maximizer.bin,
                                          self.on_filter_removed,
                                          self.log_tag)

    def on_output_limiter_enable(self, obj, state):
        if state:
            if not self.output_limiter_wrapper.get_by_name(
                    'output_limiter_bin'):
                self.output_limiter_wrapper.append(self.output_limiter.bin,
                                                   self.on_filter_added,
                                                   self.log_tag)
        else:
            self.output_limiter_wrapper.remove(self.output_limiter.bin,
                                               self.on_filter_removed,
                                               self.log_tag)

    def reset(self):
        EffectsBase.reset(self)

        self.exciter.reset()
        self.bass_enhancer.reset()
        self.stereo_enhancer.reset()
        self.stereo_spread.reset()
        self.panorama.reset()
        self.maximizer.reset()
        self.output_limiter.reset()
