# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GstInsertBin, Gtk
from PulseEffects.bass_enhancer import BassEnhancer
from PulseEffects.crossfeed import Crossfeed
from PulseEffects.delay import Delay
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

        EffectsBase.__init__(self, self.pm.default_sink_rate)

        self.log_tag = 'SIE - '

        pa_props = 'application.id=com.github.wwmm.pulseeffects.sinkinputs'

        self.set_pa_props(pa_props)

        self.set_source_monitor_name(self.pm.apps_sink_monitor_name)

        pulse_sink = os.environ.get('PULSE_SINK')

        if pulse_sink:
            self.set_output_sink_name(pulse_sink)

            self.log.debug('$PULSE_SINK = ' + pulse_sink)

            msg = 'user has $PULSE_SINK set. Using it as output device'

            self.log.debug(msg)
        else:
            self.set_output_sink_name(self.pm.default_sink_name)

        self.pm.connect('sink_input_added', self.on_app_added)
        self.pm.connect('sink_input_changed', self.on_app_changed)
        self.pm.connect('sink_input_removed', self.on_app_removed)
        self.pm.connect('sink_input_level_changed', self.on_app_level_changed)

        self.exciter = Exciter()
        self.bass_enhancer = BassEnhancer()
        self.delay = Delay()
        self.stereo_enhancer = StereoEnhancer()
        self.stereo_spread = StereoSpread()
        self.crossfeed = Crossfeed()
        self.panorama = Panorama()
        self.maximizer = Maximizer()
        self.output_limiter = OutputLimiter()

        self.limiter.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.limiter')
        self.panorama.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.panorama')
        self.compressor.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.compressor')
        self.reverb.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.reverb')
        self.highpass.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.highpass')
        self.lowpass.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.lowpass')
        self.equalizer.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.equalizer')
        self.exciter.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.exciter')
        self.bass_enhancer.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.bassenhancer')
        self.delay.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.delay')
        self.stereo_enhancer.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.stereoenhancer')
        self.stereo_spread.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.stereospread')
        self.crossfeed.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.crossfeed')
        self.maximizer.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.maximizer')
        self.output_limiter.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.outputlimiter')

        # effects wrappers
        self.exciter_wrapper = GstInsertBin.InsertBin.new('exciter_wrapper')
        self.bass_enhancer_wrapper = GstInsertBin.InsertBin.new(
            'bass_enhancer_wrapper')
        self.delay_wrapper = GstInsertBin.InsertBin.new('delay_wrapper')
        self.stereo_enhancer_wrapper = GstInsertBin.InsertBin.new(
            'stereo_enhancer_wrapper')
        self.stereo_spread_wrapper = GstInsertBin.InsertBin.new(
            'stereo_spread_wrapper')
        self.crossfeed_wrapper = GstInsertBin.InsertBin.new(
            'crossfeed_wrapper')
        self.panorama_wrapper = GstInsertBin.InsertBin.new('panorama_wrapper')
        self.maximizer_wrapper = GstInsertBin.InsertBin.new(
            'maximizer_wrapper')
        self.output_limiter_wrapper = GstInsertBin.InsertBin.new(
            'output_limiter_wrapper')

        # appending effects wrappers to effects bin
        # the effects order is defined here

        self.effects_bin.append(self.limiter_wrapper, self.on_filter_added,
                                None)
        self.effects_bin.append(self.compressor_wrapper, self.on_filter_added,
                                None)
        self.effects_bin.append(self.highpass_wrapper, self.on_filter_added,
                                None)
        self.effects_bin.append(self.lowpass_wrapper, self.on_filter_added,
                                None)
        self.effects_bin.append(self.equalizer_wrapper, self.on_filter_added,
                                None)
        self.effects_bin.append(self.exciter_wrapper, self.on_filter_added,
                                None)
        self.effects_bin.append(self.bass_enhancer_wrapper,
                                self.on_filter_added, None)
        self.effects_bin.append(self.stereo_enhancer_wrapper,
                                self.on_filter_added, None)
        self.effects_bin.append(self.panorama_wrapper, self.on_filter_added,
                                None)
        self.effects_bin.append(self.stereo_spread_wrapper,
                                self.on_filter_added, None)
        self.effects_bin.append(self.reverb_wrapper, self.on_filter_added,
                                None)
        self.effects_bin.append(self.crossfeed_wrapper, self.on_filter_added,
                                None)
        self.effects_bin.append(self.delay_wrapper, self.on_filter_added, None)
        self.effects_bin.append(self.maximizer_wrapper, self.on_filter_added,
                                None)
        self.effects_bin.append(self.output_limiter_wrapper,
                                self.on_filter_added, None)
        self.effects_bin.append(self.spectrum_wrapper, self.on_filter_added,
                                None)

    def init_ui(self):
        EffectsBase.init_ui(self)

        self.limiter.init_ui()
        self.compressor.init_ui()
        self.highpass.init_ui()
        self.lowpass.init_ui()
        self.equalizer.init_ui()
        self.exciter.init_ui()
        self.bass_enhancer.init_ui()
        self.delay.init_ui()
        self.stereo_enhancer.init_ui()
        self.stereo_spread.init_ui()
        self.crossfeed.init_ui()
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
        self.add_to_listbox('panorama')
        self.add_to_listbox('stereo_spread')
        self.add_to_listbox('reverb')
        self.add_to_listbox('crossfeed')
        self.add_to_listbox('delay')
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
        self.stack.add_named(self.panorama.ui_window, 'panorama')
        self.stack.add_named(self.stereo_spread.ui_window, 'stereo_spread')
        self.stack.add_named(self.reverb.ui_window, 'reverb')
        self.stack.add_named(self.crossfeed.ui_window, 'crossfeed')
        self.stack.add_named(self.delay.ui_window, 'delay')
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
        self.delay.ui_enable.connect('state-set', self.on_delay_enable)
        self.stereo_enhancer.ui_enable.connect('state-set',
                                               self.on_stereo_enhancer_enable)
        self.stereo_spread.ui_enable.connect('state-set',
                                             self.on_stereo_spread_enable)
        self.crossfeed.ui_enable.connect('state-set', self.on_crossfeed_enable)
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

        if self.delay.is_installed:
            self.delay.bind()
        else:
            self.delay.ui_window.set_sensitive(False)
            self.delay.ui_enable.set_sensitive(False)
            self.delay.ui_img_state.hide()

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

        if self.crossfeed.is_installed:
            self.crossfeed.bind()
        else:
            self.crossfeed.ui_window.set_sensitive(False)
            self.crossfeed.ui_enable.set_sensitive(False)
            self.crossfeed.ui_img_state.hide()

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

    def on_app_added(self, obj, parameters):
        EffectsBase.on_app_added(self, obj, parameters)

        if self.there_is_window:
            source_name = 'PulseEffects_apps.monitor'.encode('utf-8')
            app_idx = parameters['index']
            app_name = parameters['name']
            connected = parameters['connected']
            corked = parameters['corked']

            if connected and not corked:
                self.streams[str(app_idx)] = self.pm.create_stream(source_name,
                                                                   app_idx,
                                                                   app_name)

    def on_app_changed(self, obj, parameters):
        EffectsBase.on_app_changed(self, obj, parameters)

        if self.there_is_window:
            source_name = 'PulseEffects_apps.monitor'.encode('utf-8')
            app_idx = parameters['index']
            app_name = parameters['name']
            connected = parameters['connected']
            corked = parameters['corked']

            key = str(app_idx)

            if connected:
                if not corked and key not in self.streams:
                    print('added: ' + key)
                    self.streams[key] = self.pm.create_stream(source_name,
                                                              app_idx,
                                                              app_name)
                elif corked and key in self.streams:
                    print('connected and corked removed: ' + key)
                    self.pm.remove_stream(self.streams[key])
            else:
                if key in self.streams:

                    del self.streams[key]

    def on_app_removed(self, obj, idx):
        EffectsBase.on_app_removed(self, obj, idx)

        key = str(idx)

        if key in self.streams:
            del self.streams[key]

    def post_messages(self, state):
        EffectsBase.post_messages(self, state)

        self.exciter.post_messages(state)
        self.bass_enhancer.post_messages(state)
        self.delay.post_messages(state)
        self.stereo_enhancer.post_messages(state)
        self.stereo_spread.post_messages(state)
        self.crossfeed.post_messages(state)
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
        elif plugin == 'delay_input_level':
            peak = msg.get_structure().get_value('peak')

            self.delay.ui_update_input_level(peak)
        elif plugin == 'delay_output_level':
            peak = msg.get_structure().get_value('peak')

            self.delay.ui_update_output_level(peak)
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
        elif plugin == 'crossfeed_input_level':
            peak = msg.get_structure().get_value('peak')

            self.crossfeed.ui_update_input_level(peak)
        elif plugin == 'crossfeed_output_level':
            peak = msg.get_structure().get_value('peak')

            self.crossfeed.ui_update_output_level(peak)
        elif plugin == 'panorama_input_level':
            peak = msg.get_structure().get_value('peak')

            self.panorama.ui_update_input_level(peak)
        elif plugin == 'panorama_output_level':
            peak = msg.get_structure().get_value('peak')

            self.panorama.ui_update_output_level(peak)
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

    def on_exciter_enable(self, obj, state):
        if state:
            if not self.exciter_wrapper.get_by_name('exciter_bin'):
                self.exciter_wrapper.append(self.exciter.bin,
                                            self.on_filter_added, None)
        else:
            self.exciter_wrapper.remove(self.exciter.bin,
                                        self.on_filter_removed, None)

    def on_bass_enhancer_enable(self, obj, state):
        if state:
            if not self.bass_enhancer_wrapper.get_by_name('bass_enhancer_bin'):
                self.bass_enhancer_wrapper.append(self.bass_enhancer.bin,
                                                  self.on_filter_added, None)
        else:
            self.bass_enhancer_wrapper.remove(self.bass_enhancer.bin,
                                              self.on_filter_removed, None)

    def on_delay_enable(self, obj, state):
        if state:
            if not self.delay_wrapper.get_by_name('delay_bin'):
                self.delay_wrapper.append(self.delay.bin,
                                          self.on_filter_added, None)
        else:
            self.delay_wrapper.remove(self.delay.bin,
                                      self.on_filter_removed, None)

    def on_stereo_enhancer_enable(self, obj, state):
        if state:
            if not self.stereo_enhancer_wrapper.get_by_name(
                    'stereo_enhancer_bin'):
                self.stereo_enhancer_wrapper.append(self.stereo_enhancer.bin,
                                                    self.on_filter_added, None)
        else:
            self.stereo_enhancer_wrapper.remove(self.stereo_enhancer.bin,
                                                self.on_filter_removed, None)

    def on_stereo_spread_enable(self, obj, state):
        if state:
            if not self.stereo_spread_wrapper.get_by_name('stereo_spread_bin'):
                self.stereo_spread_wrapper.append(self.stereo_spread.bin,
                                                  self.on_filter_added, None)
        else:
            self.stereo_spread_wrapper.remove(self.stereo_spread.bin,
                                              self.on_filter_removed, None)

    def on_crossfeed_enable(self, obj, state):
        if state:
            if not self.crossfeed_wrapper.get_by_name('crossfeed_bin'):
                self.crossfeed_wrapper.append(self.crossfeed.bin,
                                              self.on_filter_added, None)
        else:
            self.crossfeed_wrapper.remove(self.crossfeed.bin,
                                          self.on_filter_removed, None)

    def on_panorama_enable(self, obj, state):
        if state:
            if not self.panorama_wrapper.get_by_name('panorama_bin'):
                self.panorama_wrapper.append(self.panorama.bin,
                                             self.on_filter_added, None)
        else:
            self.panorama_wrapper.remove(self.panorama.bin,
                                         self.on_filter_removed, None)

    def on_maximizer_enable(self, obj, state):
        if state:
            if not self.maximizer_wrapper.get_by_name('maximizer_bin'):
                self.maximizer_wrapper.append(self.maximizer.bin,
                                              self.on_filter_added, None)
        else:
            self.maximizer_wrapper.remove(self.maximizer.bin,
                                          self.on_filter_removed, None)

    def on_output_limiter_enable(self, obj, state):
        if state:
            if not self.output_limiter_wrapper.get_by_name(
                    'output_limiter_bin'):
                self.output_limiter_wrapper.append(self.output_limiter.bin,
                                                   self.on_filter_added, None)
        else:
            self.output_limiter_wrapper.remove(self.output_limiter.bin,
                                               self.on_filter_removed, None)

    def reset(self):
        EffectsBase.reset(self)

        self.exciter.reset()
        self.bass_enhancer.reset()
        self.delay.reset()
        self.stereo_enhancer.reset()
        self.stereo_spread.reset()
        self.crossfeed.reset()
        self.panorama.reset()
        self.maximizer.reset()
        self.output_limiter.reset()
