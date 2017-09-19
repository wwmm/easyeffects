# -*- coding: utf-8 -*-

import gettext

from gi.repository import Gio
from PulseEffects.effects_base import EffectsBase
from PulseEffects.panorama import Panorama


gettext.textdomain('PulseEffects')
_ = gettext.gettext


class SinkInputEffects(EffectsBase):

    def __init__(self, sampling_rate):
        self.settings = Gio.Settings('com.github.wwmm.pulseeffects.sinkinputs')

        EffectsBase.__init__(self, sampling_rate, self.settings)

        self.log_tag = 'apps: '
        self.panorama_ready = False

        self.panorama = Panorama(self.settings)

        self.insert_in_listbox('panorama', 1)

        self.listbox.show_all()

        # adding effects widgets to the stack

        self.stack.add_titled(self.limiter.ui_window, 'Limiter',
                              _('Input Limiter'))
        self.stack.add_titled(self.panorama.ui_window, 'Panorama',
                              _('Panorama'))
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
        self.panorama.ui_enable.connect('state-set', self.on_panorama_enable)
        self.compressor.ui_enable.connect('state-set',
                                          self.on_compressor_enable)
        self.reverb.ui_enable.connect('state-set', self.on_reverb_enable)
        self.highpass.ui_enable.connect('state-set', self.on_highpass_enable)
        self.lowpass.ui_enable.connect('state-set', self.on_lowpass_enable)
        self.equalizer.ui_enable.connect('state-set', self.on_equalizer_enable)

        # order is important
        self.limiter.bind()
        self.panorama.bind()
        self.compressor.bind()
        self.reverb.bind()
        self.highpass.bind()
        self.lowpass.bind()
        self.equalizer.bind()

    def on_listbox_row_activated(self, obj, row):
        EffectsBase.on_listbox_row_activated(self, obj, row)

        name = row.get_name()

        if name == 'panorama':
            self.stack.set_visible_child(self.panorama.ui_window)

    def on_message_element(self, bus, msg):
        EffectsBase.on_message_element(self, bus, msg)

        plugin = msg.src.get_name()

        if plugin == 'panorama_input_level':
            peak = msg.get_structure().get_value('peak')

            self.panorama.ui_update_panorama_input_level(peak)
        elif plugin == 'panorama_output_level':
            peak = msg.get_structure().get_value('peak')

            self.panorama.ui_update_panorama_output_level(peak)

        return True

    def on_panorama_enable(self, obj, state):
        self.panorama_ready = False

        if state:
            limiter_enabled = self.settings.get_value('limiter-state').unpack()

            if limiter_enabled:
                while not self.limiter_ready:
                    pass

                self.effects_bin.insert_after(self.panorama.bin,
                                              self.limiter.bin,
                                              self.on_panorama_added,
                                              self.log_tag)
            else:
                self.effects_bin.prepend(self.panorama.bin,
                                         self.on_panorama_added, self.log_tag)
        else:
            self.effects_bin.remove(self.panorama.bin, self.on_filter_removed,
                                    self.log_tag)

    def on_compressor_enable(self, obj, state):
        self.compressor_ready = False

        if state:
            limiter_enabled = self.settings.get_value('limiter-state').unpack()
            panorama_enabled = self.settings.get_value(
                'panorama-state').unpack()

            if panorama_enabled:
                while not self.panorama_ready:
                    pass

                self.effects_bin.insert_after(self.compressor.bin,
                                              self.panorama.bin,
                                              self.on_compressor_added,
                                              self.log_tag)
            elif limiter_enabled:
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
            panorama_enabled = self.settings.get_value(
                'panorama-state').unpack()
            compressor_enabled = self.settings.get_value(
                'compressor-state').unpack()

            if compressor_enabled:
                while not self.compressor_ready:
                    pass

                self.effects_bin.insert_after(self.reverb.bin,
                                              self.compressor.bin,
                                              self.on_reverb_added,
                                              self.log_tag)
            elif panorama_enabled:
                while not self.panorama_ready:
                    pass

                self.effects_bin.insert_after(self.reverb.bin,
                                              self.panorama.bin,
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
            panorama_enabled = self.settings.get_value(
                'panorama-state').unpack()
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
            elif panorama_enabled:
                while not self.panorama_ready:
                    pass

                self.effects_bin.insert_after(self.highpass.bin,
                                              self.panorama.bin,
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
            panorama_enabled = self.settings.get_value(
                'panorama-state').unpack()
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
            elif panorama_enabled:
                while not self.panorama_ready:
                    pass

                self.effects_bin.insert_after(self.lowpass.bin,
                                              self.panorama.bin,
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
            panorama_enabled = self.settings.get_value(
                'panorama-state').unpack()
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
            elif panorama_enabled:
                while not self.panorama_ready:
                    pass

                self.effects_bin.insert_after(self.equalizer.bin,
                                              self.panorama.bin,
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

    def on_panorama_added(self, bin, element, success, user_data):
        bin_name = element.get_name()
        plugin_name = bin_name.split('_')[0]

        if success:
            self.panorama_ready = True
            self.log.info(user_data + plugin_name + ' plugin was enabled')
        else:
            self.log.critical(user_data + 'failed to enable ' + plugin_name)

    def reset(self):
        self.limiter.reset()
        self.panorama.reset()
        self.compressor.reset()
        self.reverb.reset()
        self.highpass.reset()
        self.lowpass.reset()
        self.equalizer.reset()
