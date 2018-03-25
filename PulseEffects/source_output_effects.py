# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GstInsertBin, Gtk
from PulseEffects.effects_base import EffectsBase
from PulseEffects.pitch import Pitch


class SourceOutputEffects(EffectsBase):

    def __init__(self, pulse_manager):
        self.pm = pulse_manager
        self.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs')

        EffectsBase.__init__(self, self.pm.default_source_rate, self.settings)

        self.log_tag = 'mic:'
        self.disable_app_level_meter = True

        pa_props = 'application.id=com.github.wwmm.pulseeffects.sourceoutputs'

        self.set_pa_props(pa_props)

        pulse_source = os.environ.get('PULSE_SOURCE')

        if pulse_source:
            self.set_source_monitor_name(pulse_source)

            self.log.info('$PULSE_SOURCE = ' + pulse_source)

            msg = 'user has $PULSE_SOURCE set. Using it as input device'

            self.log.info(msg)
        else:
            self.set_source_monitor_name(self.pm.default_source_name)

        self.set_output_sink_name('PulseEffects_mic')

        self.pm.connect('source_output_added', self.on_app_added)
        self.pm.connect('source_output_changed', self.on_app_changed)
        self.pm.connect('source_output_removed', self.on_app_removed)
        self.pm.connect('new_default_source', self.update_source_monitor_name)

        self.pitch = Pitch(self.settings)

        # effects wrappers

        self.pitch_wrapper = GstInsertBin.InsertBin.new('pitch_wrapper')

        # appending effects wrappers to effects bin
        # the effects order is defined here

        self.effects_bin.append(self.limiter_wrapper, self.on_filter_added,
                                self.log_tag)
        self.effects_bin.append(self.compressor_wrapper, self.on_filter_added,
                                self.log_tag)
        self.effects_bin.append(self.highpass_wrapper, self.on_filter_added,
                                self.log_tag)
        self.effects_bin.append(self.lowpass_wrapper, self.on_filter_added,
                                self.log_tag)
        self.effects_bin.append(self.equalizer_wrapper, self.on_filter_added,
                                self.log_tag)
        self.effects_bin.append(self.reverb_wrapper, self.on_filter_added,
                                self.log_tag)
        self.effects_bin.append(self.pitch_wrapper, self.on_filter_added,
                                self.log_tag)
        self.effects_bin.append(self.spectrum_wrapper, self.on_filter_added,
                                self.log_tag)

        # webrtcprobe_src = Gst.ElementFactory.make('pulsesrc',
        #                                           'webrtcprobe_src')
        # webrtcprobe_sink = Gst.ElementFactory.make('fakesink',
        #                                            'webrtcprobe_sink')
        #
        # webrtcprobe_src.set_property('provide-clock', False)
        #
        # self.pipeline.add(webrtcprobe_src)
        # self.pipeline.add(webrtcprobe_sink)
        #
        # webrtcprobe_src.link(webrtcprobe_sink)
        #
        # webrtcprobe_src.connect('notify::source-output-index',
        #                         lambda x, y: print(x, y))

    def init_ui(self):
        EffectsBase.init_ui(self)

        self.limiter.init_ui()
        self.compressor.init_ui()
        self.highpass.init_ui()
        self.lowpass.init_ui()
        self.equalizer.init_ui()
        self.reverb.init_ui()
        self.pitch.init_ui()

        self.add_to_listbox('limiter')
        self.add_to_listbox('compressor')
        self.add_to_listbox('highpass')
        self.add_to_listbox('lowpass')
        self.add_to_listbox('equalizer')
        self.add_to_listbox('reverb')
        self.add_to_listbox('pitch')

        self.listbox.show_all()

        # adding effects widgets to the stack
        self.stack.add_named(self.limiter.ui_window, 'limiter')
        self.stack.add_named(self.compressor.ui_window, 'compressor')
        self.stack.add_named(self.highpass.ui_window, 'highpass')
        self.stack.add_named(self.lowpass.ui_window, 'lowpass')
        self.stack.add_named(self.equalizer.ui_window, 'equalizer')
        self.stack.add_named(self.reverb.ui_window, 'reverb')
        self.stack.add_named(self.pitch.ui_window, 'pitch')

        # on/off switches connections
        self.limiter.ui_limiter_enable.connect('state-set',
                                               self.on_limiter_enable)
        self.compressor.ui_enable.connect('state-set',
                                          self.on_compressor_enable)
        self.highpass.ui_enable.connect('state-set', self.on_highpass_enable)
        self.lowpass.ui_enable.connect('state-set', self.on_lowpass_enable)
        self.equalizer.ui_enable.connect('state-set', self.on_equalizer_enable)
        self.reverb.ui_enable.connect('state-set', self.on_reverb_enable)
        self.pitch.ui_enable.connect('state-set', self.on_pitch_enable)

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

        if self.pitch.is_installed:
            self.pitch.bind()
        else:
            self.pitch.ui_window.set_sensitive(False)
            self.pitch.ui_enable.set_sensitive(False)
            self.pitch.ui_img_state.hide()

        self.highpass.bind()
        self.lowpass.bind()
        self.equalizer.bind()
        self.reverb.bind()

    def on_enable_app(self, obj, state, idx):
        if state:
            self.pm.move_source_output_to_pulseeffects_source(idx)
        else:
            self.pm.move_source_output_to_default_source(idx)

    def on_volume_changed(self, obj, idx, audio_channels):
        self.pm.set_source_output_volume(idx, audio_channels, obj.get_value())

    def on_mute(self, button, idx, icon):
        state = button.get_active()

        if state:
            icon_name = 'audio-volume-muted-symbolic'
            icon.set_from_icon_name(icon_name, Gtk.IconSize.BUTTON)
        else:
            icon_name = 'audio-volume-high-symbolic'
            icon.set_from_icon_name(icon_name, Gtk.IconSize.BUTTON)

        self.pm.set_source_output_mute(idx, state)

    def post_messages(self, state):
        EffectsBase.post_messages(self, state)

        self.pitch.post_messages(state)

    def on_message_element(self, bus, msg):
        EffectsBase.on_message_element(self, bus, msg)

        plugin = msg.src.get_name()

        if plugin == 'pitch_input_level':
            peak = msg.get_structure().get_value('peak')

            self.pitch.ui_update_input_level(peak)
        elif plugin == 'pitch_output_level':
            peak = msg.get_structure().get_value('peak')

            self.pitch.ui_update_output_level(peak)

        return True

    def on_pitch_enable(self, obj, state):
        if state:
            if not self.pitch_wrapper.get_by_name('pitch_bin'):
                self.pitch_wrapper.append(self.pitch.bin,
                                          self.on_filter_added,
                                          self.log_tag)
        else:
            self.pitch_wrapper.remove(self.pitch.bin,
                                      self.on_filter_removed,
                                      self.log_tag)

    def reset(self):
        EffectsBase.reset(self)

        self.pitch.reset()
