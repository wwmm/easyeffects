# -*- coding: utf-8 -*-
import gettext

from gi.repository import Gio
from PulseEffects.effects_base import EffectsBase

gettext.textdomain('PulseEffects')
_ = gettext.gettext


class SourceOutputEffects(EffectsBase):

    def __init__(self, sampling_rate):
        self.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs')

        EffectsBase.__init__(self, sampling_rate, self.settings)

        self.log_tag = 'mic: '

        self.listbox.show_all()

        # it makes no sense to show the calibration button here
        self.equalizer.ui_eq_calibrate_button.destroy()

        # adding effects widgets to the stack
        self.stack.add_titled(self.limiter.ui_window, 'Limiter',
                              _('Input Limiter'))
        self.stack.add_titled(self.compressor.ui_window, 'Compressor',
                              _('Compressor'))
        self.stack.add_titled(self.reverb.ui_window, 'Reverb',
                              _('Reverberation'))
        self.stack.add_titled(self.highpass.ui_window, 'Highpass',
                              _('High pass'))
        self.stack.add_titled(self.lowpass.ui_window, 'Lowpass',
                              _('Low pass'))
        self.stack.add_titled(self.equalizer.ui_window, 'Equalizer',
                              _('Equalizer'))

        # on/off switches connections
        self.compressor.ui_enable.connect('state-set',
                                          self.on_compressor_enable)
        self.reverb.ui_enable.connect('state-set', self.on_reverb_enable)
        self.highpass.ui_enable.connect('state-set', self.on_highpass_enable)
        self.lowpass.ui_enable.connect('state-set', self.on_lowpass_enable)
        self.equalizer.ui_enable.connect('state-set', self.on_equalizer_enable)

        # order is important
        self.limiter.bind()
        self.compressor.bind()
        self.reverb.bind()
        self.highpass.bind()
        self.lowpass.bind()
        self.equalizer.bind()

    def on_compressor_enable(self, obj, state):
        self.compressor_ready = False

        if state:
            limiter_enabled = self.settings.get_value('limiter-state').unpack()

            if limiter_enabled:
                while not self.limiter_ready:
                    pass

                self.effects_bin.insert_after(self.compressor.bin,
                                              self.limiter.bin,
                                              self.on_compressor_added,
                                              self.log_tag)
            else:
                self.effects_bin.prepend(self.compressor.bin,
                                         self.on_compressor_added,
                                         self.log_tag)
        else:
            self.effects_bin.remove(self.compressor.bin,
                                    self.on_filter_removed,
                                    self.log_tag)

    def on_reverb_enable(self, obj, state):
        self.reverb_ready = False

        if state:
            limiter_enabled = self.settings.get_value('limiter-state').unpack()
            compressor_enabled = self.settings.get_value(
                'compressor-state').unpack()

            if compressor_enabled:
                while not self.compressor_ready:
                    pass

                self.effects_bin.insert_after(self.reverb.bin,
                                              self.compressor.bin,
                                              self.on_reverb_added,
                                              self.log_tag)
            elif limiter_enabled:
                while not self.limiter_ready:
                    pass

                self.effects_bin.insert_after(self.reverb.bin,
                                              self.limiter.bin,
                                              self.on_reverb_added,
                                              self.log_tag)
            else:
                self.effects_bin.prepend(self.reverb.bin,
                                         self.on_reverb_added, self.log_tag)
        else:
            self.effects_bin.remove(self.reverb.bin, self.on_filter_removed,
                                    self.log_tag)

    def on_highpass_enable(self, obj, state):
        self.highpass_ready = False

        if state:
            limiter_enabled = self.settings.get_value('limiter-state').unpack()
            compressor_enabled = self.settings.get_value(
                'compressor-state').unpack()
            reverb_enabled = self.settings.get_value(
                'reverb-state').unpack()

            if reverb_enabled:
                while not self.reverb_ready:
                    pass

                self.effects_bin.insert_after(self.highpass.bin,
                                              self.reverb.bin,
                                              self.on_highpass_added,
                                              self.log_tag)
            elif compressor_enabled:
                while not self.compressor_ready:
                    pass

                self.effects_bin.insert_after(self.highpass.bin,
                                              self.compressor.bin,
                                              self.on_highpass_added,
                                              self.log_tag)
            elif limiter_enabled:
                while not self.limiter_ready:
                    pass

                self.effects_bin.insert_after(self.highpass.bin,
                                              self.limiter.bin,
                                              self.on_highpass_added,
                                              self.log_tag)
            else:
                self.effects_bin.prepend(self.highpass.bin,
                                         self.on_highpass_added, self.log_tag)
        else:
            self.effects_bin.remove(self.highpass.bin, self.on_filter_removed,
                                    self.log_tag)

    def on_lowpass_enable(self, obj, state):
        self.lowpass_ready = False

        if state:
            limiter_enabled = self.settings.get_value('limiter-state').unpack()
            compressor_enabled = self.settings.get_value(
                'compressor-state').unpack()
            reverb_enabled = self.settings.get_value(
                'reverb-state').unpack()
            highpass_enabled = self.settings.get_value(
                'highpass-state').unpack()

            if highpass_enabled:
                while not self.highpass_ready:
                    pass

                self.effects_bin.insert_after(self.lowpass.bin,
                                              self.highpass.bin,
                                              self.on_lowpass_added,
                                              self.log_tag)
            elif reverb_enabled:
                while not self.reverb_ready:
                    pass

                self.effects_bin.insert_after(self.lowpass.bin,
                                              self.reverb.bin,
                                              self.on_lowpass_added,
                                              self.log_tag)
            elif compressor_enabled:
                while not self.compressor_ready:
                    pass

                self.effects_bin.insert_after(self.lowpass.bin,
                                              self.compressor.bin,
                                              self.on_lowpass_added,
                                              self.log_tag)
            elif limiter_enabled:
                while not self.limiter_ready:
                    pass

                self.effects_bin.insert_after(self.lowpass.bin,
                                              self.limiter.bin,
                                              self.on_lowpass_added,
                                              self.log_tag)
            else:
                self.effects_bin.prepend(self.lowpass.bin,
                                         self.on_lowpass_added, self.log_tag)
        else:
            self.effects_bin.remove(self.lowpass.bin, self.on_filter_removed,
                                    self.log_tag)

    def on_equalizer_enable(self, obj, state):
        self.equalizer_ready = False

        if state:
            limiter_enabled = self.settings.get_value('limiter-state').unpack()
            compressor_enabled = self.settings.get_value(
                'compressor-state').unpack()
            reverb_enabled = self.settings.get_value(
                'reverb-state').unpack()
            highpass_enabled = self.settings.get_value(
                'highpass-state').unpack()
            lowpass_enabled = self.settings.get_value(
                'lowpass-state').unpack()

            if lowpass_enabled:
                while not self.lowpass_ready:
                    pass

                self.effects_bin.insert_after(self.equalizer.bin,
                                              self.lowpass.bin,
                                              self.on_equalizer_added,
                                              self.log_tag)
            elif highpass_enabled:
                while not self.highpass_ready:
                    pass

                self.effects_bin.insert_after(self.equalizer.bin,
                                              self.highpass.bin,
                                              self.on_equalizer_added,
                                              self.log_tag)
            elif reverb_enabled:
                while not self.reverb_ready:
                    pass

                self.effects_bin.insert_after(self.equalizer.bin,
                                              self.reverb.bin,
                                              self.on_equalizer_added,
                                              self.log_tag)
            elif compressor_enabled:
                while not self.compressor_ready:
                    pass

                self.effects_bin.insert_after(self.equalizer.bin,
                                              self.compressor.bin,
                                              self.on_equalizer_added,
                                              self.log_tag)
            elif limiter_enabled:
                while not self.limiter_ready:
                    pass

                self.effects_bin.insert_after(self.equalizer.bin,
                                              self.limiter.bin,
                                              self.on_equalizer_added,
                                              self.log_tag)
            else:
                self.effects_bin.prepend(self.equalizer.bin,
                                         self.on_equalizer_added, self.log_tag)
        else:
            self.effects_bin.remove(self.equalizer.bin, self.on_filter_removed,
                                    self.log_tag)

    def reset(self):
        self.limiter.reset()
        self.compressor.reset()
        self.reverb.reset()
        self.highpass.reset()
        self.lowpass.reset()
        self.equalizer.reset()
