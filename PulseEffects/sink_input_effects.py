# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GstInsertBin, Gtk
from PulseEffects.bass_enhancer import BassEnhancer
from PulseEffects.effects_base import EffectsBase
from PulseEffects.exciter import Exciter
from PulseEffects.output_limiter import OutputLimiter
from PulseEffects.panorama import Panorama


class SinkInputEffects(EffectsBase):

    def __init__(self, pulse_manager):
        self.pm = pulse_manager
        self.settings = Gio.Settings('com.github.wwmm.pulseeffects.sinkinputs')

        EffectsBase.__init__(self, self.pm.default_sink_rate, self.settings)

        self.log_tag = 'apps: '
        self.panorama_ready = False
        self.exciter_ready = False
        self.bass_enhancer_ready = False

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

        self.panorama = Panorama(self.settings)
        self.exciter = Exciter(self.settings)
        self.bass_enhancer = BassEnhancer(self.settings)
        self.output_limiter = OutputLimiter(self.settings)

        # effects wrappers
        self.panorama_wrapper = GstInsertBin.InsertBin.new('panorama_wrapper')
        self.exciter_wrapper = GstInsertBin.InsertBin.new('exciter_wrapper')
        self.bass_enhancer_wrapper = GstInsertBin.InsertBin.new(
            'bass_enhancer_wrapper')
        self.output_limiter_wrapper = GstInsertBin.InsertBin.new(
            'output_limiter_wrapper')

        # appending effects wrappers to effects bin
        self.effects_bin.insert_after(self.panorama_wrapper,
                                      self.limiter_wrapper,
                                      self.on_filter_added,
                                      self.log_tag)

        self.effects_bin.insert_after(self.exciter_wrapper,
                                      self.equalizer_wrapper,
                                      self.on_filter_added,
                                      self.log_tag)

        self.effects_bin.insert_after(self.bass_enhancer_wrapper,
                                      self.exciter_wrapper,
                                      self.on_filter_added,
                                      self.log_tag)

        self.effects_bin.insert_before(self.output_limiter_wrapper,
                                       self.spectrum_wrapper,
                                       self.on_filter_added,
                                       self.log_tag)

    def init_ui(self):
        EffectsBase.init_ui(self)

        self.panorama.init_ui()
        self.exciter.init_ui()
        self.bass_enhancer.init_ui()
        self.output_limiter.init_ui()

        self.insert_in_listbox('panorama', 2)
        self.add_to_listbox('exciter')
        self.add_to_listbox('bass_enhancer')
        self.add_to_listbox('output_limiter')

        self.listbox.show_all()

        # adding effects widgets to the stack

        self.stack.add_named(self.limiter.ui_window, 'limiter')
        self.stack.add_named(self.panorama.ui_window, 'panorama')
        self.stack.add_named(self.compressor.ui_window, 'compressor')
        self.stack.add_named(self.reverb.ui_window, 'reverb')
        self.stack.add_named(self.highpass.ui_window, 'highpass')
        self.stack.add_named(self.lowpass.ui_window, 'lowpass')
        self.stack.add_named(self.equalizer.ui_window, 'equalizer')
        self.stack.add_named(self.exciter.ui_window, 'exciter')
        self.stack.add_named(self.bass_enhancer.ui_window, 'bass_enhancer')
        self.stack.add_named(self.output_limiter.ui_window, 'output_limiter')

        # on/off switches connections
        self.panorama.ui_enable.connect('state-set', self.on_panorama_enable)
        self.exciter.ui_enable.connect('state-set', self.on_exciter_enable)
        self.bass_enhancer.ui_enable.connect('state-set',
                                             self.on_bass_enhancer_enable)
        self.output_limiter.ui_limiter_enable\
            .connect('state-set', self.on_output_limiter_enable)

        if self.limiter.is_installed:
            self.limiter.bind()
        else:
            self.limiter.ui_window.set_sensitive(False)
            self.limiter.ui_limiter_enable.set_sensitive(False)

        if self.panorama.is_installed:
            self.panorama.bind()
        else:
            self.panorama.ui_window.set_sensitive(False)
            self.panorama.ui_enable.set_sensitive(False)

        if self.compressor.is_installed:
            self.compressor.bind()
        else:
            self.compressor.ui_window.set_sensitive(False)
            self.compressor.ui_enable.set_sensitive(False)

        if self.exciter.is_installed:
            self.exciter.bind()
        else:
            self.exciter.ui_window.set_sensitive(False)
            self.exciter.ui_enable.set_sensitive(False)

        if self.bass_enhancer.is_installed:
            self.bass_enhancer.bind()
        else:
            self.bass_enhancer.ui_window.set_sensitive(False)
            self.bass_enhancer.ui_enable.set_sensitive(False)

        if self.output_limiter.is_installed:
            self.output_limiter.bind()
        else:
            self.output_limiter.ui_window.set_sensitive(False)
            self.output_limiter.ui_limiter_enable.set_sensitive(False)

        self.reverb.bind()
        self.highpass.bind()
        self.lowpass.bind()
        self.equalizer.bind()

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

        self.panorama.post_messages(state)
        self.exciter.post_messages(state)
        self.bass_enhancer.post_messages(state)
        self.output_limiter.post_messages(state)

    def on_message_element(self, bus, msg):
        EffectsBase.on_message_element(self, bus, msg)

        plugin = msg.src.get_name()

        if plugin == 'panorama_input_level':
            peak = msg.get_structure().get_value('peak')

            self.panorama.ui_update_panorama_input_level(peak)
        elif plugin == 'panorama_output_level':
            peak = msg.get_structure().get_value('peak')

            self.panorama.ui_update_panorama_output_level(peak)
        elif plugin == 'exciter_input_level':
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

        self.panorama.reset()
        self.exciter.reset()
        self.bass_enhancer.reset()
        self.output_limiter.reset()
