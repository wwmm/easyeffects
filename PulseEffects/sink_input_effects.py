# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GstInsertBin, Gtk
from PulseEffects.effects_base import EffectsBase
from PulseEffects.output_limiter import OutputLimiter
from PulseEffects.panorama import Panorama


class SinkInputEffects(EffectsBase):

    def __init__(self, pulse_manager):
        self.pm = pulse_manager
        self.settings = Gio.Settings('com.github.wwmm.pulseeffects.sinkinputs')

        EffectsBase.__init__(self, self.pm.default_sink_rate, self.settings)

        self.log_tag = 'apps: '
        self.switch_on_all_apps = False
        self.panorama_ready = False

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
        self.output_limiter = OutputLimiter(self.settings)

        # effects wrappers
        self.panorama_wrapper = GstInsertBin.InsertBin.new('panorama_wrapper')
        self.output_limiter_wrapper = GstInsertBin.InsertBin.new(
            'output_limiter_wrapper')

        # appending effects wrappers to effects bin
        self.effects_bin.insert_after(self.panorama_wrapper,
                                      self.limiter_wrapper,
                                      self.on_filter_added,
                                      self.log_tag)

        self.effects_bin.insert_before(self.output_limiter_wrapper,
                                       self.spectrum_wrapper,
                                       self.on_filter_added,
                                       self.log_tag)

    def init_ui(self):
        EffectsBase.init_ui(self)

        self.panorama.init_ui()

        self.insert_in_listbox('panorama', 2)
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
        self.stack.add_named(self.output_limiter.ui_window, 'output_limiter')

        # on/off switches connections
        self.panorama.ui_enable.connect('state-set', self.on_panorama_enable)
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

        if self.output_limiter.is_installed:
            self.output_limiter.bind()

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

    def on_listbox_row_activated(self, obj, row):
        EffectsBase.on_listbox_row_activated(self, obj, row)

        name = row.get_name()

        if name == 'panorama':
            self.stack.set_visible_child(self.panorama.ui_window)
        elif name == 'output_limiter':
            self.stack.set_visible_child(self.output_limiter.ui_window)

    def on_message_element(self, bus, msg):
        EffectsBase.on_message_element(self, bus, msg)

        plugin = msg.src.get_name()

        if plugin == 'panorama_input_level':
            peak = msg.get_structure().get_value('peak')

            self.panorama.ui_update_panorama_input_level(peak)
        elif plugin == 'panorama_output_level':
            peak = msg.get_structure().get_value('peak')

            self.panorama.ui_update_panorama_output_level(peak)
        elif plugin == 'output_limiter_input_level':
            peak = msg.get_structure().get_value('peak')

            self.output_limiter.ui_update_limiter_input_level(peak)
        elif plugin == 'output_limiter_output_level':
            peak = msg.get_structure().get_value('peak')

            self.output_limiter.ui_update_limiter_output_level(peak)

        return True

    def on_panorama_enable(self, obj, state):
        if state:
            self.panorama_wrapper.append(self.panorama.bin,
                                         self.on_filter_added,
                                         self.log_tag)
        else:
            self.panorama_wrapper.remove(self.panorama.bin,
                                         self.on_filter_removed,
                                         self.log_tag)

    def on_output_limiter_enable(self, obj, state):
        if state:
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
        self.output_limiter.reset()
