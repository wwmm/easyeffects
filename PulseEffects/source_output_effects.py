# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, Gst, GstInsertBin, Gtk
from PulseEffects.deesser import Deesser
from PulseEffects.effects_base import EffectsBase
from PulseEffects.gate import Gate
from PulseEffects.pitch import Pitch
from PulseEffects.webrtc import Webrtc


class SourceOutputEffects(EffectsBase):

    def __init__(self, pulse_manager):
        self.pm = pulse_manager
        self.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs')

        EffectsBase.__init__(self, self.pm.default_source_rate)

        self.log_tag = 'SOE - '
        self.disable_app_level_meter = True

        pa_props = 'application.id=com.github.wwmm.pulseeffects.sourceoutputs'

        self.set_pa_props(pa_props)

        pulse_source = os.environ.get('PULSE_SOURCE')

        if pulse_source:
            self.set_source_monitor_name(pulse_source)

            self.log.debug('$PULSE_SOURCE = ' + pulse_source)

            msg = 'user has $PULSE_SOURCE set. Using it as input device'

            self.log.debug(msg)
        else:
            self.set_source_monitor_name(self.pm.default_source_name)

        self.set_output_sink_name('PulseEffects_mic')

        self.pm.connect('source_output_added', self.on_app_added)
        self.pm.connect('source_output_changed', self.on_app_changed)
        self.pm.connect('source_output_removed', self.on_app_removed)

        self.pitch = Pitch()
        self.gate = Gate(self.settings)
        self.deesser = Deesser(self.settings)
        self.webrtc = Webrtc(self.settings)

        self.limiter.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs.limiter')
        self.compressor.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs.compressor')
        self.reverb.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs.reverb')
        self.highpass.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs.highpass')
        self.lowpass.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs.lowpass')
        self.equalizer.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs.equalizer')
        self.pitch.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs.pitch')

        # effects wrappers

        self.gate_wrapper = GstInsertBin.InsertBin.new('gate_wrapper')
        self.webrtc_wrapper = GstInsertBin.InsertBin.new('webrtc_wrapper')
        self.pitch_wrapper = GstInsertBin.InsertBin.new('pitch_wrapper')
        self.deesser_wrapper = GstInsertBin.InsertBin.new('deesser_wrapper')

        # appending effects wrappers to effects bin
        # the effects order is defined here

        self.effects_bin.append(self.gate_wrapper, self.on_filter_added, None)
        self.effects_bin.append(self.webrtc_wrapper, self.on_filter_added,
                                None)
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
        self.effects_bin.append(self.deesser_wrapper, self.on_filter_added,
                                None)
        self.effects_bin.append(self.reverb_wrapper, self.on_filter_added,
                                None)
        self.effects_bin.append(self.pitch_wrapper, self.on_filter_added, None)
        self.effects_bin.append(self.spectrum_wrapper, self.on_filter_added,
                                None)

    def init_ui(self):
        EffectsBase.init_ui(self)

        self.gate.init_ui()
        self.webrtc.init_ui()
        self.limiter.init_ui()
        self.compressor.init_ui()
        self.highpass.init_ui()
        self.lowpass.init_ui()
        self.equalizer.init_ui()
        self.deesser.init_ui()
        self.reverb.init_ui()
        self.pitch.init_ui()

        self.add_to_listbox('gate')
        self.add_to_listbox('webrtc')
        self.add_to_listbox('limiter')
        self.add_to_listbox('compressor')
        self.add_to_listbox('highpass')
        self.add_to_listbox('lowpass')
        self.add_to_listbox('equalizer')
        self.add_to_listbox('deesser')
        self.add_to_listbox('reverb')
        self.add_to_listbox('pitch')

        self.listbox.show_all()

        # adding effects widgets to the stack
        self.stack.add_named(self.gate.ui_window, 'gate')
        self.stack.add_named(self.webrtc.ui_window, 'webrtc')
        self.stack.add_named(self.limiter.ui_window, 'limiter')
        self.stack.add_named(self.compressor.ui_window, 'compressor')
        self.stack.add_named(self.highpass.ui_window, 'highpass')
        self.stack.add_named(self.lowpass.ui_window, 'lowpass')
        self.stack.add_named(self.equalizer.ui_window, 'equalizer')
        self.stack.add_named(self.deesser.ui_window, 'deesser')
        self.stack.add_named(self.reverb.ui_window, 'reverb')
        self.stack.add_named(self.pitch.ui_window, 'pitch')

        # on/off switches connections
        self.gate.ui_enable.connect('state-set', self.on_gate_enable)
        self.webrtc.ui_enable.connect('state-set', self.on_webrtc_enable)
        self.limiter.ui_limiter_enable.connect('state-set',
                                               self.on_limiter_enable)
        self.compressor.ui_enable.connect('state-set',
                                          self.on_compressor_enable)
        self.highpass.ui_enable.connect('state-set', self.on_highpass_enable)
        self.lowpass.ui_enable.connect('state-set', self.on_lowpass_enable)
        self.equalizer.ui_enable.connect('state-set', self.on_equalizer_enable)
        self.deesser.ui_enable.connect('state-set', self.on_deesser_enable)
        self.reverb.ui_enable.connect('state-set', self.on_reverb_enable)
        self.pitch.ui_enable.connect('state-set', self.on_pitch_enable)

        if self.gate.is_installed:
            self.gate.bind()
        else:
            self.gate.ui_window.set_sensitive(False)
            self.gate.ui_enable.set_sensitive(False)
            self.gate.ui_img_state.hide()

        if self.webrtc.is_installed:
            self.webrtc.set_probe_src_device(self.pm.default_sink_name +
                                             '.monitor')
            self.webrtc.bind()
        else:
            self.webrtc.ui_window.set_sensitive(False)
            self.webrtc.ui_enable.set_sensitive(False)
            self.webrtc.ui_img_state.hide()

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

        if self.deesser.is_installed:
            self.deesser.bind()
        else:
            self.deesser.ui_window.set_sensitive(False)
            self.deesser.ui_enable.set_sensitive(False)
            self.deesser.ui_img_state.hide()

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

        self.gate.post_messages(state)
        self.webrtc.post_messages(state)
        self.pitch.post_messages(state)
        self.deesser.post_messages(state)

    def on_message_element(self, bus, msg):
        EffectsBase.on_message_element(self, bus, msg)

        plugin = msg.src.get_name()

        if plugin == 'gate_input_level':
            peak = msg.get_structure().get_value('peak')

            self.gate.ui_update_input_level(peak)
        elif plugin == 'gate_output_level':
            peak = msg.get_structure().get_value('peak')

            self.gate.ui_update_output_level(peak)
        elif plugin == 'webrtc_input_level':
            peak = msg.get_structure().get_value('peak')

            self.webrtc.ui_update_input_level(peak)
        elif plugin == 'webrtc_output_level':
            peak = msg.get_structure().get_value('peak')

            self.webrtc.ui_update_output_level(peak)
        elif plugin == 'deesser_input_level':
            peak = msg.get_structure().get_value('peak')

            self.deesser.ui_update_input_level(peak)
        elif plugin == 'deesser_output_level':
            peak = msg.get_structure().get_value('peak')

            self.deesser.ui_update_output_level(peak)
        elif plugin == 'pitch_input_level':
            peak = msg.get_structure().get_value('peak')

            self.pitch.ui_update_input_level(peak)
        elif plugin == 'pitch_output_level':
            peak = msg.get_structure().get_value('peak')

            self.pitch.ui_update_output_level(peak)

        return True

    def on_gate_enable(self, obj, state):
        if state:
            if not self.gate_wrapper.get_by_name('gate_bin'):
                self.gate_wrapper.append(self.gate.bin,
                                         self.on_filter_added, None)
        else:
            self.gate_wrapper.remove(self.gate.bin,
                                     self.on_filter_removed, None)

    def on_webrtc_enable(self, obj, state):
        if state:
            self.pipeline.add(self.webrtc.probe_bin)
            self.webrtc.probe_bin.set_state(Gst.State.PLAYING)

            if not self.webrtc_wrapper.get_by_name('webrtc_bin'):
                self.webrtc_wrapper.append(self.webrtc.bin,
                                           self.on_filter_added, None)
        else:
            self.webrtc_wrapper.remove(self.webrtc.bin,
                                       self.on_filter_removed, None)

            self.webrtc.probe_bin.set_state(Gst.State.NULL)
            self.pipeline.remove(self.webrtc.probe_bin)

    def on_deesser_enable(self, obj, state):
        if state:
            if not self.deesser_wrapper.get_by_name('deesser_bin'):
                self.deesser_wrapper.append(self.deesser.bin,
                                            self.on_filter_added, None)
        else:
            self.deesser_wrapper.remove(self.deesser.bin,
                                        self.on_filter_removed, None)

    def on_pitch_enable(self, obj, state):
        if state:
            if not self.pitch_wrapper.get_by_name('pitch_bin'):
                self.pitch_wrapper.append(self.pitch.bin,
                                          self.on_filter_added, None)
        else:
            self.pitch_wrapper.remove(self.pitch.bin,
                                      self.on_filter_removed, None)

    def reset(self):
        EffectsBase.reset(self)

        self.gate.reset()
        self.webrtc.reset()
        self.deesser.reset()
        self.pitch.reset()
