# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, Gtk
from PulseEffects.effects_base import EffectsBase


class SourceOutputEffects(EffectsBase):

    def __init__(self, pulse_manager):
        self.pm = pulse_manager
        self.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs')

        EffectsBase.__init__(self, self.pm.default_source_rate, self.settings)

        self.log_tag = 'mic: '
        self.disable_app_level_meter = True

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

    def init_ui(self):
        EffectsBase.init_ui(self)

        self.limiter.init_ui()
        self.compressor.init_ui()
        self.highpass.init_ui()
        self.lowpass.init_ui()
        self.equalizer.init_ui()
        self.reverb.init_ui()

        self.add_to_listbox('limiter')
        self.add_to_listbox('compressor')
        self.add_to_listbox('highpass')
        self.add_to_listbox('lowpass')
        self.add_to_listbox('equalizer')
        self.add_to_listbox('reverb')

        self.listbox.show_all()

        # it makes no sense to show the calibration button here
        self.equalizer.ui_eq_calibrate_button.destroy()

        # adding effects widgets to the stack
        self.stack.add_named(self.limiter.ui_window, 'limiter')
        self.stack.add_named(self.compressor.ui_window, 'compressor')
        self.stack.add_named(self.highpass.ui_window, 'highpass')
        self.stack.add_named(self.lowpass.ui_window, 'lowpass')
        self.stack.add_named(self.equalizer.ui_window, 'equalizer')
        self.stack.add_named(self.reverb.ui_window, 'reverb')

        # on/off switches connections
        self.limiter.ui_limiter_enable.connect('state-set',
                                               self.on_limiter_enable)
        self.compressor.ui_enable.connect('state-set',
                                          self.on_compressor_enable)
        self.highpass.ui_enable.connect('state-set', self.on_highpass_enable)
        self.lowpass.ui_enable.connect('state-set', self.on_lowpass_enable)
        self.equalizer.ui_enable.connect('state-set', self.on_equalizer_enable)
        self.reverb.ui_enable.connect('state-set', self.on_reverb_enable)

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
